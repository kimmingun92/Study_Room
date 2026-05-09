#include "mainclient.h"
#include "ui_mainclient.h"
#include <QMessageBox>

// ── 스타일 상수 ────────────────────────────────────────────
static const QString SEAT_DEFAULT =
    "QPushButton {"
    "  background-color: white;"
    "  color: #1E293B;"
    "  border: 2px solid #E2E8F0;"
    "  border-radius: 12px;"
    "  font-size: 28px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover {"
    "  border: 2px solid #93C5FD;"
    "  background-color: #EFF6FF;"
    "}";

static const QString SEAT_SELECTED =
    "QPushButton {"
    "  background-color: #2563EB;"
    "  color: white;"
    "  border: 2px solid #1D4ED8;"
    "  border-radius: 12px;"
    "  font-size: 28px;"
    "  font-weight: bold;"
    "}";

static const QString SEAT_USED =
    "QPushButton {"
    "  background-color: #9CA3AF;"
    "  color: #E5E7EB;"
    "  border: 2px solid #9CA3AF;"
    "  border-radius: 12px;"
    "  font-size: 28px;"
    "  font-weight: bold;"
    "}";

// index: 0=Off, 1=Yellow, 2=White, 3=Warm White
struct LedStyle {
    QString normal;
    QString selected;
    QString label;
};

static const QList<LedStyle> LED_STYLES = {
    // Off
    {
        "QPushButton { background-color: #1E293B; color: #94A3B8;"
        "  border: 2px solid #334155; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "QPushButton { background-color: #0F172A; color: white;"
        "  border: 2px solid #94A3B8; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "Off"
    },
    // Yellow
    {
        "QPushButton { background-color: #FEF9C3; color: #854D0E;"
        "  border: 2px solid #FDE047; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "QPushButton { background-color: #EAB308; color: white;"
        "  border: 2px solid #CA8A04; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "Yellow"
    },
    // White
    {
        "QPushButton { background-color: #F8FAFC; color: #475569;"
        "  border: 2px solid #CBD5E1; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "QPushButton { background-color: white; color: #1E293B;"
        "  border: 2px solid #2563EB; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "White"
    },
    // Warm White
    {
        "QPushButton { background-color: #FFF7ED; color: #9A3412;"
        "  border: 2px solid #FDBA74; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "QPushButton { background-color: #F97316; color: white;"
        "  border: 2px solid #EA580C; border-radius: 10px;"
        "  font-size: 12px; font-weight: bold; }",
        "Warm"
    }
};
// ───────────────────────────────────────────────────────────

MainClient::MainClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainClient)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    // ── 좌석 버튼 리스트 ──────────────────────────────────
    seatButtons << ui->btn_seat_1 << ui->btn_seat_2 << ui->btn_seat_3;
    for (QPushButton *btn : seatButtons)
        btn->setStyleSheet(SEAT_DEFAULT);

    // ── LED 버튼 리스트 등록 ──────────────────────────────
    ledButtons << ui->btnLedOff
               << ui->btnLedYellow
               << ui->btnLedWhite
               << ui->btnLedWarm;

    // 초기 스타일 적용
    for (int i = 0; i < ledButtons.size(); ++i)
        ledButtons[i]->setStyleSheet(LED_STYLES[i].normal);

    // Off 기본 선택
    selectLedButton(ui->btnLedOff);

    // ── 경과 시간 타이머 ───────────────────────────────────
    elapsedTimer = new QTimer(this);
    elapsedTimer->setInterval(1000);
    connect(elapsedTimer, &QTimer::timeout,
            this, &MainClient::updateElapsedTime);

    // ── FAN 슬라이더 레이블 연동 ──────────────────────────
    connect(ui->fanSpeed, &QSlider::valueChanged, this, [this](int value) {
        ui->fanSpeedValueLabel->setText(QString("단계: %1").arg(value));
    });

    // ── 소켓 설정 ─────────────────────────────────────────
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::readyRead,
            this, &MainClient::readServerData);
    connect(tcpSocket, &QAbstractSocket::errorOccurred,
            this, &MainClient::onSocketError);
    connect(tcpSocket, &QTcpSocket::connected, this, [this]() {
        ui->statusbar->showMessage("서버 연결됨");
    });
    connect(tcpSocket, &QTcpSocket::disconnected, this, [this]() {
        ui->statusbar->showMessage("서버 연결 끊김");
    });

    tcpSocket->connectToHost("10.10.16.9", 5000);
}

MainClient::~MainClient() { delete ui; }

// ──────────────────────────────────────────────
// LED 버튼 선택 상태 업데이트
// ──────────────────────────────────────────────
void MainClient::selectLedButton(QPushButton *selected)
{
    for (int i = 0; i < ledButtons.size(); ++i) {
        if (ledButtons[i] == selected) {
            ledButtons[i]->setStyleSheet(LED_STYLES[i].selected);
            selectedLedIndex = i;
        } else {
            ledButtons[i]->setStyleSheet(LED_STYLES[i].normal);
        }
    }
}

// ──────────────────────────────────────────────
// LED 버튼 슬롯
// ──────────────────────────────────────────────
void MainClient::on_btnLedOff_clicked()
{
    selectLedButton(ui->btnLedOff);
    if (selectedSeatNumber == 0) return;
    sendCommand(50 + (selectedSeatNumber - 1), 0, 0);
}

void MainClient::on_btnLedYellow_clicked()
{
    selectLedButton(ui->btnLedYellow);
    if (selectedSeatNumber == 0) return;
    sendCommand(50 + (selectedSeatNumber - 1), 0, 1);
}

void MainClient::on_btnLedWhite_clicked()
{
    selectLedButton(ui->btnLedWhite);
    if (selectedSeatNumber == 0) return;
    sendCommand(50 + (selectedSeatNumber - 1), 0, 2);
}

void MainClient::on_btnLedWarm_clicked()
{
    selectLedButton(ui->btnLedWarm);
    if (selectedSeatNumber == 0) return;
    sendCommand(50 + (selectedSeatNumber - 1), 0, 3);
}

// ──────────────────────────────────────────────
// 좌석 버튼
// ──────────────────────────────────────────────
void MainClient::on_btn_seat_1_clicked() { updateSeatSelection(1); }
void MainClient::on_btn_seat_2_clicked() { updateSeatSelection(2); }
void MainClient::on_btn_seat_3_clicked() { updateSeatSelection(3); }

void MainClient::updateSeatSelection(int seatNum)
{
    selectedSeatNumber = seatNum;
    for (int i = 0; i < seatButtons.size(); ++i) {
        if (!seatButtons[i]->isEnabled()) continue;
        seatButtons[i]->setStyleSheet(
            (i == seatNum - 1) ? SEAT_SELECTED : SEAT_DEFAULT
            );
    }
}

// ──────────────────────────────────────────────
// 입실 / 퇴실
// ──────────────────────────────────────────────
void MainClient::on_enterButton_clicked()
{
    if (selectedSeatNumber == 0) {
        QMessageBox::warning(this, "알림", "좌석을 먼저 선택해주세요.");
        return;
    }
    tcpSocket->write(QString("ENTER:%1\n").arg(selectedSeatNumber).toUtf8());
    tcpSocket->flush();
}

void MainClient::on_exitButton_clicked()
{
    if (selectedSeatNumber != 0) {
        tcpSocket->write(QString("EXIT:%1\n").arg(selectedSeatNumber).toUtf8());
        tcpSocket->flush();
    }
    goToLoginPage();
}

// ──────────────────────────────────────────────
// 제어 명령 전송
// ──────────────────────────────────────────────
void MainClient::sendCommand(int id, int type, QVariant value)
{
    tcpSocket->write(
        QString("@%1:%2:%3\n").arg(id).arg(type).arg(value.toString()).toUtf8()
        );
    tcpSocket->flush();
}

void MainClient::on_fanSpeed_valueChanged(int value)
{
    if (selectedSeatNumber == 0) return;
    sendCommand(53, 0, value);
}

void MainClient::on_doorOpenButton_clicked()
{
    sendCommand(53 + selectedSeatNumber, 3, 1);
}

void MainClient::on_doorCloseButton_clicked()
{
    sendCommand(53 + selectedSeatNumber, 3, 0);
}

// ──────────────────────────────────────────────
// 수신 처리
// ──────────────────────────────────────────────
void MainClient::readServerData()
{
    recvBuffer += QString::fromUtf8(tcpSocket->readAll());
    while (recvBuffer.contains('\n')) {
        int idx     = recvBuffer.indexOf('\n');
        QString msg = recvBuffer.left(idx).trimmed();
        recvBuffer  = recvBuffer.mid(idx + 1);
        if (!msg.isEmpty()) processMessage(msg);
    }
}

void MainClient::processMessage(const QString &msg)
{
    if (msg.startsWith("USED_SEATS:")) {
        for (QPushButton *btn : seatButtons) {
            btn->setEnabled(true);
            btn->setStyleSheet(SEAT_DEFAULT);
        }
        QStringList usedList = msg.mid(11).split(",");
        for (const QString &s : usedList) {
            if (s.isEmpty()) continue;
            int idx = s.toInt() - 1;
            if (idx >= 0 && idx < seatButtons.size()) {
                seatButtons[idx]->setEnabled(false);
                seatButtons[idx]->setStyleSheet(SEAT_USED);
            }
        }
        if (selectedSeatNumber > 0
            && seatButtons[selectedSeatNumber - 1]->isEnabled()) {
            seatButtons[selectedSeatNumber - 1]->setStyleSheet(SEAT_SELECTED);
        }

    } else if (msg == "ENTRY_SUCCESS") {
        goToMainPage();

    } else if (msg == "ENTRY_FAIL") {
        QMessageBox::warning(this, "입실 실패",
                             "해당 좌석은 이미 사용 중이거나\n유효하지 않은 좌석 번호입니다.");
        selectedSeatNumber = 0;
        for (QPushButton *btn : seatButtons)
            if (btn->isEnabled()) btn->setStyleSheet(SEAT_DEFAULT);
    }
}

// ──────────────────────────────────────────────
// 페이지 전환
// ──────────────────────────────────────────────
void MainClient::goToMainPage()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->seatInfoLabel->setText(QString("좌석 %1번").arg(selectedSeatNumber));

    entryTime = QTime::currentTime();
    elapsedTimer->start();
    updateElapsedTime();

    // LED Off으로 초기화
    selectLedButton(ui->btnLedOff);

    // FAN 슬라이더 초기화
    ui->fanSpeed->blockSignals(true);
    ui->fanSpeed->setValue(1);
    ui->fanSpeedValueLabel->setText("단계: 1");
    ui->fanSpeed->blockSignals(false);
}

void MainClient::goToLoginPage()
{
    elapsedTimer->stop();
    selectedSeatNumber = 0;
    ui->stackedWidget->setCurrentIndex(0);
}

// ──────────────────────────────────────────────
// 경과 시간
// ──────────────────────────────────────────────
void MainClient::updateElapsedTime()
{
    int secs = entryTime.secsTo(QTime::currentTime());
    ui->elapsedTimeLabel->setText(
        QString("입실 시간: %1:%2:%3")
            .arg(secs / 3600,        2, 10, QChar('0'))
            .arg((secs % 3600) / 60, 2, 10, QChar('0'))
            .arg(secs % 60,          2, 10, QChar('0'))
        );
}

void MainClient::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    ui->statusbar->showMessage("연결 오류: " + tcpSocket->errorString());
}
