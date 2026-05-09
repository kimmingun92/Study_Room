#include "mainserver.h"
#include "ui_mainserver.h"

MainServer::MainServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainServer)
{
    ui->setupUi(this);
    m_server = new QTcpServer(this);

    if (m_server->listen(QHostAddress::Any, 5000)) {
        ui->logView->append("[SERVER] 서버 시작: 포트 5000");
    } else {
        ui->logView->append("[SERVER] 서버 시작 실패: " + m_server->errorString());
    }

    connect(m_server, &QTcpServer::newConnection, this, &MainServer::onNewConnection);
}

MainServer::~MainServer()
{
    delete ui;
}

// ──────────────────────────────────────────────
// 신규 연결
// ──────────────────────────────────────────────
void MainServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        QString clientIp = socket->peerAddress().toString();

        if (clientIp.contains("10.10.16.200")) {
            // 기존 보드 소켓이 살아있으면 먼저 정리
            if (m_boardSocket && m_boardSocket->state() == QAbstractSocket::ConnectedState) {
                ui->logView->append("[BOARD] 기존 보드 소켓 교체");
                m_boardSocket->disconnectFromHost();
            }
            m_boardSocket = socket;
            ui->logView->append("[BOARD] 보드 연결됨: " + clientIp);
        } else {
            m_clients.append(socket);
            m_recvBuffers[socket] = "";
            ui->logView->append("[CLIENT] 클라이언트 연결됨: " + clientIp);

            // 연결 즉시 현재 사용 중인 좌석 목록 전송
            sendUsedSeatsList(socket);
        }

        connect(socket, &QTcpSocket::readyRead,   this, &MainServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &MainServer::onDisconnected);
    }
}

// ──────────────────────────────────────────────
// 데이터 수신 - 버퍼 기반 처리
// ──────────────────────────────────────────────
void MainServer::onReadyRead()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray rawData = senderSocket->readAll();

    // 보드에서 온 데이터: 줄바꿈 없이 오는 경우 있으므로 별도 처리
    if (senderSocket == m_boardSocket) {
        QString msg = QString::fromUtf8(rawData).trimmed();
        ui->logView->append("[BOARD->] " + msg);
        processBoardMessage(senderSocket, rawData, msg);
        return;
    }

    // 클라이언트 데이터: 줄바꿈(\n) 기반 버퍼 파싱
    m_recvBuffers[senderSocket] += QString::fromUtf8(rawData);

    while (m_recvBuffers[senderSocket].contains('\n')) {
        int idx = m_recvBuffers[senderSocket].indexOf('\n');
        QString msg = m_recvBuffers[senderSocket].left(idx).trimmed();
        m_recvBuffers[senderSocket] = m_recvBuffers[senderSocket].mid(idx + 1);

        if (msg.isEmpty()) continue;

        QString senderIp = senderSocket->peerAddress().toString();
        ui->logView->append(QString("[%1] %2").arg(senderIp).arg(msg));
        processClientMessage(senderSocket, msg);
    }
}

// ──────────────────────────────────────────────
// 클라이언트 메시지 처리
// ──────────────────────────────────────────────
void MainServer::processClientMessage(QTcpSocket *socket, const QString &msg)
{
    QString senderIp = socket->peerAddress().toString();

    if (msg.startsWith("ENTER:")) {
        int seatNum = msg.mid(6).toInt();

        // 좌석 번호 유효성 검사
        if (seatNum < 1 || seatNum > 3) {
            socket->write("ENTRY_FAIL\n");
            socket->flush();
            ui->logView->append("[ERROR] 유효하지 않은 좌석 번호: " + QString::number(seatNum));
            return;
        }
        // 중복 입실 방지
        if (m_seatMap.contains(socket)) {
            socket->write("ENTRY_FAIL\n");
            socket->flush();
            ui->logView->append("[ERROR] 이미 입실 중인 소켓의 재입실 요청 무시");
            return;
        }
        // 이미 사용 중인 좌석 확인
        if (m_seatMap.values().contains(seatNum)) {
            socket->write("ENTRY_FAIL\n");
            socket->flush();
            ui->logView->append(QString("[ERROR] 좌석 %1번은 이미 사용 중").arg(seatNum));
            return;
        }

        m_seatMap.insert(socket, seatNum);
        ui->logView->append(QString("[ENTER] %1번 좌석 입실 완료 (%2)").arg(seatNum).arg(senderIp));

        socket->write("ENTRY_SUCCESS\n");
        socket->flush();

        // 본인 포함 전체 브로드캐스트
        broadcastUsedSeats();

    } else if (msg.startsWith("EXIT:")) {
        int seatNum = msg.mid(5).toInt();
        m_seatMap.remove(socket);
        ui->logView->append(QString("[EXIT] %1번 좌석 퇴실 완료 (%2)").arg(seatNum).arg(senderIp));

        broadcastUsedSeats();

    } else if (msg.startsWith("@")) {
        // 제어 명령 → 보드로 포워딩
        if (m_boardSocket && m_boardSocket->state() == QAbstractSocket::ConnectedState) {
            m_boardSocket->write((msg + "\n").toUtf8());
            m_boardSocket->flush();
            ui->logView->append("[->BOARD] " + msg);
        } else {
            ui->logView->append("[ERROR] 보드 미연결 상태에서 제어 명령 수신");
        }
    }
}

// ──────────────────────────────────────────────
// 보드 메시지 처리 (센서 데이터 → 입실 클라이언트에게 포워딩)
// ──────────────────────────────────────────────
void MainServer::processBoardMessage(QTcpSocket *socket, const QByteArray &raw, const QString &msg)
{
    Q_UNUSED(socket)
    // @BOARD:READY 같은 내부 메시지는 포워딩 제외
    if (msg.startsWith("@BOARD:")) return;

    for (QTcpSocket *client : m_clients) {
        if (m_seatMap.contains(client)) {
            client->write(raw);
            client->flush();
        }
    }
}

// ──────────────────────────────────────────────
// 좌석 목록 전송
// ──────────────────────────────────────────────
void MainServer::sendUsedSeatsList(QTcpSocket *socket)
{
    QStringList usedSeats;
    for (int seatNum : m_seatMap.values()) {
        usedSeats << QString::number(seatNum);
    }
    QString msg = "USED_SEATS:" + usedSeats.join(",") + "\n";
    socket->write(msg.toUtf8());
    socket->flush();
}

// 전체 클라이언트에게 브로드캐스트
void MainServer::broadcastUsedSeats()
{
    for (QTcpSocket *client : m_clients) {
        sendUsedSeatsList(client);
    }
}

// ──────────────────────────────────────────────
// 연결 종료
// ──────────────────────────────────────────────
void MainServer::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    if (socket == m_boardSocket) {
        m_boardSocket = nullptr;
        ui->logView->append("[BOARD] 보드 연결 끊김");
    } else {
        m_recvBuffers.remove(socket); // 버퍼 정리

        if (m_seatMap.contains(socket)) {
            int seatNum = m_seatMap.value(socket);
            m_seatMap.remove(socket);
            ui->logView->append(QString("[AUTO-EXIT] 비정상 종료 - %1번 좌석 자동 퇴실").arg(seatNum));
            broadcastUsedSeats();
        }
        m_clients.removeAll(socket);
    }

    ui->logView->append("[DISCONNECTED] " + socket->peerAddress().toString());
    socket->deleteLater();
}
