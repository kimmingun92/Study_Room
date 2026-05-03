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
}

MainServer::~MainServer()
{
    delete ui;
}

void MainServer::onNewConnection(){
    while(m_server->hasPendingConnections()){
        QTcpSocket *socket = m_server->nextPendingConnection();
        QString clientIp = socket->peerAddress().toString();

        // 보드 IP인 경우 별도 저장 (client와 분리)
        if(clientIp.contains("10.10.16.200")){
            m_boardSocket = socket;
            ui->logView->append("Board connected from " + clientIp);
        } else {
            m_clients.append(socket);
            ui->logView->append("Client connected from " + clientIp);
        }

        connect(socket, &QTcpSocket::readyRead, this, &MainServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &MainServer::onDisconnected);
    }
}

void MainServer::onReadyRead() {
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    while (senderSocket->canReadLine()) {
        QByteArray data = senderSocket->readLine();
        QString senderIp = senderSocket->peerAddress().toString();

        qDebug() << "Data received from" << senderSocket->peerAddress().toString() << ":" << data;

        ui->logView->append(QString("[%1] %2").arg(senderIp).arg(QString(data)));

        // 데이터 중계 로직
        if (senderSocket == m_boardSocket) {
            // 1. 보드에서 온 데이터($DATA...) -> 모든 클라이언트(사용자 앱)에게 전송
            for (QTcpSocket *client : m_clients) {
                client->write(data);
                client->flush();
            }
        } else {
            // 2. 앱에서 온 데이터($CMD...) -> 보드에게 전송
            if(m_boardSocket && m_boardSocket->state() == QAbstractSocket::ConnectedState){
                m_boardSocket->write(data);
                m_boardSocket->flush();
            } else {
                ui->logView->append("Board not connected");
            }
        }
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