#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainServer; }
QT_END_NAMESPACE

class MainServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainServer(QWidget *parent = nullptr);
    ~MainServer() override;

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void sendUsedSeatsList(QTcpSocket *socket);
    void broadcastUsedSeats();                    // 전체 브로드캐스트
    void processClientMessage(QTcpSocket *socket, const QString &msg);
    void processBoardMessage(QTcpSocket *socket, const QByteArray &raw, const QString &msg);

    Ui::MainServer *ui;
    QTcpServer     *m_server;
    QList<QTcpSocket*>       m_clients;
    QTcpSocket              *m_boardSocket = nullptr;
    QMap<QTcpSocket*, int>   m_seatMap;
    QMap<QTcpSocket*, QString> m_recvBuffers; // TCP 버퍼
};

#endif
