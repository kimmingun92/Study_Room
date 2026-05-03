#include "mainclient.h"
#include "ui_mainclient.h"
#include <QMessageBox>

MainClient::MainClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainClient)
{
    ui->setupUi(this);
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainClient::onReadyRead);
}

MainClient::~MainClient()
{
    delete ui;
}

void MainClient::on_connectBtn_clicked()
{
    if(tcpSocket->state() != QAbstractSocket::ConnectedState){
        // 나중에 10.10.16.9로 변경
        tcpSocket->connectToHost("127.0.0.1", 5000);

        if(tcpSocket->waitForConnected(3000)){
            ui->statusLabel->setText("Status: Connected");
        } else {
            QMessageBox::critical(this, "Error", "Connection Failed!");
        }
    }
}

void MainClient::on_sendBtn_clicked()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QString cmd = ui->cmdInput->text();

        if (!cmd.isEmpty()) {
            tcpSocket->write(cmd.toUtf8());
            ui->logText->append("Sent: " + cmd);
            ui->cmdInput->clear();
        }
    } else {
        ui->statusLabel->setText("Status: Disconnected");
    }
}

void MainClient::onReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    ui->logText->append("Recv: " + QString::fromUtf8(data));
}