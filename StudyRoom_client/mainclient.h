#ifndef MAINCLIENT_H
#define MAINCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainClient;
}
QT_END_NAMESPACE

class MainClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainClient(QWidget *parent = nullptr);
    ~MainClient() override;

private slots:
    void on_connectBtn_clicked();
    void on_sendBtn_clicked();
    void onReadyRead();

private:
    Ui::MainClient *ui;
    QTcpSocket *tcpSocket;
};
#endif // MAINCLIENT_H
