#include "mainserver.h"
#include "ui_mainserver.h"

MainServer::MainServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainServer)
{
    ui->setupUi(this);
    m_server = new QTcpServer(this);

    if(m_server->listen(QHostAddress::Any, 5000)){
        ui->logView->append("서버 시작: 포트 5000");
    }

    connect(m_server, &QTcpServer::newConnection, this, &MainServer::onNewConnection);
    connect(ui->sendBtn, &QPushButton::clicked, this, &MainServer::onSendButtonClicked);
}

MainServer::~MainServer()
{
    delete ui;
}

void MainServer::onNewConnection(){
    while(m_server->hasPendingConnections()){
        QTcpSocket *socket = m_server->nextPendingConnection();
        m_clients.append(socket);

        QString ip = socket->peerAddress().toString();
        ui->logView->append("접속됨: " + ip);

        // 보드 IP인 경우 별도 저장 (client와 분리)
        if(ip.contains("10.10.16.200")){
            m_boardSocket = socket;
            ui->logView->append("보드가 인식되었습니다.");
        }

        connect(socket, &QTcpSocket::readyRead, this, &MainServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &MainServer::onDisconnected);
    }
}

void MainServer::onReadyRead() {
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();
    QString senderIp = senderSocket->peerAddress().toString();

    ui->logView->append(QString("[%1] %2").arg(senderIp).arg(QString(data)));

    // 데이터 중계 로직
    if (senderSocket == m_boardSocket) {
        // 1. 보드에서 온 데이터($DATA...) -> 모든 클라이언트(사용자 앱)에게 전송
        for (QTcpSocket *client : m_clients) {
            if (client != m_boardSocket) client->write(data);
        }
    } else {
        // 2. 앱에서 온 데이터($CMD...) -> 보드에게 전송
        if (m_boardSocket) m_boardSocket->write(data);
    }
}

void MainServer::onDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        if (socket == m_boardSocket) m_boardSocket = nullptr;
        m_clients.removeAll(socket);
        ui->logView->append("접속 종료: " + socket->peerAddress().toString());
        socket->deleteLater();
    }
}

void MainServer::onSendButtonClicked() {
    // UI에서 직접 보드로 명령 날리기 테스트용
    QString cmd = ui->cmdInput->text();
    if (m_boardSocket && !cmd.isEmpty()) {
        m_boardSocket->write(cmd.toUtf8());
        ui->logView->append("서버 -> 보드 명령 전송: " + cmd);
    }
}