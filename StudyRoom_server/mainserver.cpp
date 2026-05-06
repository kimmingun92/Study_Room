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

            sendUsedSeatsList(socket);
        }

        connect(socket, &QTcpSocket::readyRead, this, &MainServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &MainServer::onDisconnected);
    }
}

void MainServer::onReadyRead() {
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    // 보드 데이터는 줄바꿈(\n)이 없을 수 있으므로 readAll() 고려
    QByteArray data = senderSocket->readAll();
    QString msg = QString::fromUtf8(data).trimmed();
    QString senderIp = senderSocket->peerAddress().toString();

    ui->logView->append(QString("[%1] %2").arg(senderIp).arg(msg));

    // 보드에서 온 데이터 중계
    if (senderSocket == m_boardSocket) {
        for (QTcpSocket *client : m_clients) {
            // 입실 완료(좌석 매핑됨)된 클라이언트에게만 보드 데이터 전송
            if (m_seatMap.contains(client)) {
                client->write(data);
                client->flush();
            }
        }
    }
    // 클라이언트에서 온 데이터 처리
    else {
        // 입실 요청 처리: "ENTER:번호"
        if (msg.startsWith("ENTER:")) {
            int seatNum = msg.mid(6).toInt();
            m_seatMap.insert(senderSocket, seatNum); // IP(소켓)와 좌석 번호 매핑

            ui->logView->append(QString("입실 완료: %1번 자리 (IP: %2)").arg(seatNum).arg(senderIp));
            senderSocket->write("ENTRY_SUCCESS"); // 클라이언트에게 성공 알림
            senderSocket->flush();

            broadcastUsedSeats(senderSocket);
        }else if (msg.startsWith("EXIT:")) {
            int seatNum = msg.mid(5).toInt();

            // 매핑 테이블에서 제거하여 자리 비움 처리[cite: 1]
            m_seatMap.remove(senderSocket);

            ui->logView->append(QString("퇴실 완료: %1번 자리 (IP: %2)").arg(seatNum).arg(senderIp));

            broadcastUsedSeats(senderSocket);
        }
        // 일반 제어 명령(@...) -> 보드로 전송
        else if (msg.startsWith("@")) {
            if(m_boardSocket && m_boardSocket->state() == QAbstractSocket::ConnectedState){
                m_boardSocket->write(data);
                m_boardSocket->flush();
            } else {
                ui->logView->append("Board not connected");
            }
        }
    }
}

void MainServer::sendUsedSeatsList(QTcpSocket* socket) {
    QStringList usedSeats;
    // m_seatMap에 저장된 좌석 번호들을 수집
    for (int seatNum : m_seatMap.values()) {
        usedSeats << QString::number(seatNum);
    }

    QString msg = "USED_SEATS:" + usedSeats.join(",");
    socket->write(msg.toUtf8());
    socket->flush();
}

// 모든 클라이언트에게 업데이트된 좌석 목록 브로드캐스트
void MainServer::broadcastUsedSeats(QTcpSocket* socket) {
    for (QTcpSocket *client : m_clients) {
        if(socket != client){
            sendUsedSeatsList(client);
        }
    }
}

void MainServer::onDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        if (socket == m_boardSocket) {
            m_boardSocket = nullptr;
            ui->logView->append("보드 연결 끊김");
        } else {
            if (m_seatMap.contains(socket)) {
                int seatNum = m_seatMap.value(socket);
                m_seatMap.remove(socket);
                ui->logView->append(QString("비정상 종료로 인한 자동 퇴실: %1번 좌석").arg(seatNum));

                // 자리가 비었으므로 다른 클라이언트들에게 실시간 업데이트 전송
                broadcastUsedSeats(socket);
            }
            m_clients.removeAll(socket);
        }
        ui->logView->append("접속 종료: " + socket->peerAddress().toString());
        socket->deleteLater();
    }
}