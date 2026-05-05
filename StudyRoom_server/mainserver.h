#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainServer;
}
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
    void sendUsedSeatsList(QTcpSocket* socket);

private:
    Ui::MainServer *ui;
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    QTcpSocket *m_boardSocket = nullptr;
    QMap<QTcpSocket*, int> m_seatMap;
    bool seatStatus[3] = {false, false, false};
};
#endif // MAINSERVER_H
