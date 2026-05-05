#ifndef MAINCLIENT_H
#define MAINCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QPushButton>

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
    void on_btn_seat_1_clicked();
    void on_btn_seat_2_clicked();
    void on_btn_seat_3_clicked();
    void on_enterButton_clicked();
    void readServerData();
    void updateSeatSelection(int seatNum);
    void sendCommand(int id, int type, QVariant value);
    void on_ledColor_currentIndexChanged(int index);
    void on_fanSpeed_valueChanged(int arg1);
    void on_doorOpenButton_clicked();
    void on_doorCloseButton_clicked();
    void on_exitButton_clicked();

private:
    Ui::MainClient *ui;
    QTcpSocket *tcpSocket;
    QList<QPushButton*> seatButtons;
    int selectedSeatNumber = 0;
};
#endif // MAINCLIENT_H
