#ifndef MAINCLIENT_H
#define MAINCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QPushButton>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QVariant>

QT_BEGIN_NAMESPACE
namespace Ui { class MainClient; }
QT_END_NAMESPACE

class MainClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainClient(QWidget *parent = nullptr);
    ~MainClient() override;

private slots:
    void on_btn_seat_1_clicked();
    void on_btn_seat_2_clicked();
    void on_btn_seat_3_clicked();
    void on_enterButton_clicked();
    void on_exitButton_clicked();

    void on_btnLedOff_clicked();
    void on_btnLedYellow_clicked();
    void on_btnLedWhite_clicked();
    void on_btnLedWarm_clicked();

    void on_fanSpeed_valueChanged(int value);
    void on_doorOpenButton_clicked();
    void on_doorCloseButton_clicked();

    void readServerData();
    void onSocketError(QAbstractSocket::SocketError error);
    void updateElapsedTime();

private:
    void updateSeatSelection(int seatNum);
    void sendCommand(int id, int type, QVariant value);
    void processMessage(const QString &msg);
    void goToMainPage();
    void goToLoginPage();
    void selectLedButton(QPushButton *selected);

    Ui::MainClient   *ui;
    QTcpSocket       *tcpSocket;
    QList<QPushButton*> seatButtons;
    QList<QPushButton*> ledButtons;
    int               selectedSeatNumber = 0;
    int               selectedLedIndex   = 0;
    QString           recvBuffer;
    QTimer           *elapsedTimer;
    QTime             entryTime;
};

#endif
