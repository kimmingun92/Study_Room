#include "mainclient.h"
#include "ui_mainclient.h"

MainClient::MainClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainClient)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);
    seatButtons << ui->btn_seat_1 << ui->btn_seat_2 << ui->btn_seat_3;

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainClient::readServerData);
    tcpSocket->connectToHost("10.10.16.9", 5000);
}

MainClient::~MainClient()
{
    delete ui;
}

void MainClient::on_btn_seat_1_clicked() {
    updateSeatSelection(1);
}

void MainClient::on_btn_seat_2_clicked() {
    updateSeatSelection(2);
}

void MainClient::on_btn_seat_3_clicked() {
    updateSeatSelection(3);
}

void MainClient::on_enterButton_clicked(){
    if(selectedSeatNumber != 0){
        QString msg = QString("ENTER:%1").arg(selectedSeatNumber);
        tcpSocket->write(msg.toUtf8());
    }
}

void MainClient::updateSeatSelection(int seatNum) {
    selectedSeatNumber = seatNum;

    for (int i = 0; i < seatButtons.size(); ++i) {
        // 이미 비활성화(회색)된 자리는 건드리지 않음
        if (!seatButtons[i]->isEnabled()) continue;

        if (i == (seatNum - 1)) {
            seatButtons[i]->setStyleSheet("background-color: blue; color: white;");
        } else {
            seatButtons[i]->setStyleSheet("background-color: white;");
        }
    }
}

void MainClient::readServerData(){
    QByteArray data = tcpSocket->readAll();
    QString msg = QString::fromUtf8(data);

    if(msg.startsWith("USED_SEATS:")){
        // "USED_SEATS:2,3" -> "2,3" 추출
        QStringList usedSeats = msg.mid(11).split(",");

        for (const QString &seatNum : usedSeats) {
            int index = seatNum.toInt() - 1; // 버튼 인덱스는 0부터 시작
            if (index >= 0 && index < seatButtons.size()) {
                // 이미 입실된 자리는 회색으로 만들고 클릭 비활성화
                seatButtons[index]->setStyleSheet("background-color: gray;");
                seatButtons[index]->setEnabled(false);
            }
        }
    } else if(msg == "ENTRY_SUCCESS"){
        ui->stackedWidget->setCurrentIndex(1);
        ui->seatInfoLabel->setText(QString("현재 좌석: %1번").arg(selectedSeatNumber));
    }
}

void MainClient::sendCommand(int id, int type, QVariant value) {
    // 프로토콜 형식: @ID:TYPE:VALUE
    QString cmd = QString("@%1:%2:%3\n").arg(id).arg(type).arg(value.toString());
    tcpSocket->write(cmd.toUtf8());
}

void MainClient::on_ledColor_currentIndexChanged(int index)
{
    int ledId = 50 + (selectedSeatNumber - 1);
    sendCommand(ledId, 0, index);
}

void MainClient::on_fanSpeed_valueChanged(int arg1)
{
    // 추후 추가
}


void MainClient::on_doorOpenButton_clicked()
{
    int doorId = 54 + selectedSeatNumber;
    sendCommand(doorId, 3, 1);
}


void MainClient::on_doorCloseButton_clicked()
{
    int doorId = 54 + selectedSeatNumber;
    sendCommand(doorId, 3, 0);
}


void MainClient::on_exitButton_clicked()
{
    // 서버에 퇴실 알림 전송 (EXIT:좌석번호)
    if (selectedSeatNumber != 0) {
        QString msg = QString("EXIT:%1").arg(selectedSeatNumber);
        tcpSocket->write(msg.toUtf8());
        tcpSocket->flush();
    }

    // 클라이언트 내부 데이터 초기화
    selectedSeatNumber = 0;

    // 다시 좌석 선택 페이지(Index 0)로 이동
    ui->stackedWidget->setCurrentIndex(0);
}

