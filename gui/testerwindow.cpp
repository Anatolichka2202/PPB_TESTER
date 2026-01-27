#include "testerwindow.h"
#include "ui_testerwindow.h"
#include "ppbcontroller.h"
#include "../core/logger.h"
#include "pult.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QCloseEvent>
#include <QThread>
TesterWindow::TesterWindow(PPBController* controller, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TesterWindow)
    , m_controller(controller) // Используем переданный контроллер
    , m_pultWindow(nullptr)
    , m_displayAsCodes(false)
    , m_currentPPBIndex(0)
    , m_isExiting(false)
{
    qDebug() << "TesterWindow constructor start, thread:" << QThread::currentThread();
    qDebug() << "Controller address:" << m_controller;
    qDebug() << "Controller communication:" << (m_controller ? m_controller->m_communication : nullptr);

    ui->setupUi(this);

 QTimer::singleShot(0, this, [this]() {
        if (!m_controller) {
            LOG_ERROR("TesterWindow: контроллер не передан!");
            QMessageBox::critical(this, "Ошибка",
                                  "Контроллер не инициализирован. Приложение будет закрыто.");
            return;
        }

    // Инициализация логгера
    Logger::init([this](const QString& logMessage) {
        appendToLog(logMessage);
    });

    if (!m_controller) {
        LOG_ERROR("TesterWindow: контроллер не передан!");
        QMessageBox::critical(this, "Ошибка",
                              "Контроллер не инициализирован. Приложение будет закрыто.");
        return;
    }

    initializeUI();
    connectSignals();
    updateConnectionUI();
    updateControlsState();

    Logger::info("Программа управления ППБ запущена");
    });
}
TesterWindow::~TesterWindow()
{
    m_isExiting = true;
    if (m_controller && m_controller->parent() != this) {
        // Контроллер управляется ApplicationManager
        m_controller = nullptr;
    }
    delete ui;
}

void TesterWindow::initializeUI()
{
    // Заполняем комбобокс выбора ППБ
    ui->comboBoxPPBSelect->clear();
    for (int i = 0; i < 8; ++i) {
        ui->comboBoxPPBSelect->addItem(QString("ППБ%1").arg(i + 1));
    }

    // Устанавливаем значения по умолчанию
    ui->lineEditIPAddress->setText("198.168.0.230");
    ui->lineEditPort->setText("1080");
    ui->lineEditPulseDuration->setText("27000");
    ui->lineEditDutyCycle->setText("3");
    ui->lineEditPulseDelay->setText("0");

    // Настройка индикаторов
    QString circleStyle = "border-radius: 10px; border: 2px solid #666; background-color: #cccccc;";

    // Канал 1
    ui->Power_circle_label_ppb_chanel_1->setStyleSheet(circleStyle);
    ui->Capacity_circle_label_ppb_chanel_1->setStyleSheet(circleStyle);
    ui->KSWN_circle_label_ppb_chanel_1->setStyleSheet(circleStyle);
    ui->tem_circle_label_ppb_chanel_1->setStyleSheet(circleStyle);

    // Канал 2
    ui->Power_circle_label_ppb_chanel_2->setStyleSheet(circleStyle);
    ui->Capacity_circle_label_ppb_chanel_2->setStyleSheet(circleStyle);
    ui->KSWN_circle_label_ppb_chanel_2->setStyleSheet(circleStyle);
    ui->tem_circle_label_ppb_chanel_2->setStyleSheet(circleStyle);

    ui->radioButtonShowPhysical->setChecked(true);
    ui->radioButtonFUReceive->setChecked(true);
}

void TesterWindow::connectSignals()
{
    // UI -> Контроллер
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &TesterWindow::onConnectClicked);
    connect(ui->pushButtonPollStatus, &QPushButton::clicked, this, &TesterWindow::onPollStatusClicked);
    connect(ui->pushButtonReset, &QPushButton::clicked, this, &TesterWindow::onResetClicked);
    connect(ui->pushButtonApplyToPPB1, &QPushButton::clicked, this, &TesterWindow::onApplyParametersClicked);
    connect(ui->checkBoxAutoPoll, &QCheckBox::toggled, this, &TesterWindow::onAutoPollToggled);
    connect(ui->pushButtonTestSequence, &QPushButton::clicked, this, &TesterWindow::onTestSequenceClicked);
    connect(ui->radioButtonFUTransmit, &QRadioButton::clicked, this, &TesterWindow::onFUTransmitClicked);
    connect(ui->radioButtonFUReceive, &QRadioButton::clicked, this, &TesterWindow::onFUReceiveClicked);
    connect(ui->radioButtonShowCodes, &QRadioButton::toggled, this, &TesterWindow::onDisplayModeChanged);
    connect(ui->comboBoxPPBSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TesterWindow::onPPBSelected);
    connect(ui->pushButtonClearLog, &QPushButton::clicked, this, &TesterWindow::onClearLogClicked);
    connect(ui->pushButtonExportLog, &QPushButton::clicked, this, &TesterWindow::onExportLogClicked);
    connect(ui->pushButtonExit, &QPushButton::clicked, this, &TesterWindow::onExitClicked);
    connect(ui->pult_active, &QPushButton::clicked, this, &TesterWindow::onPultActiveClicked);

    // Добавим кнопки PRBS (если их нет в UI, нужно добавить)
    // connect(ui->pushButtonPRBS_M2S, &QPushButton::clicked, this, &TesterWindow::onPRBSM2SClicked);
    // connect(ui->pushButtonPRBS_S2M, &QPushButton::clicked, this, &TesterWindow::onPRBSS2MClicked);

    // Контроллер -> UI
    connect(m_controller, &PPBController::connectionStateChanged,
            this, &TesterWindow::onControllerConnectionStateChanged);
    connect(m_controller, &PPBController::logMessage,
            this, &TesterWindow::onControllerLogMessage);
    connect(m_controller, &PPBController::errorOccurred,
            this, &TesterWindow::onControllerErrorOccurred);
    connect(m_controller, &PPBController::channelStateUpdated,
            this, &TesterWindow::onControllerChannelStateUpdated);
    connect(m_controller, &PPBController::autoPollToggled,
            this, &TesterWindow::onAutoPollToggledFromController);
    connect(m_controller, &PPBController::operationProgress,
            this, &TesterWindow::onOperationProgress);
    connect(m_controller, &PPBController::operationCompleted,
            this, &TesterWindow::onOperationCompleted);
}

// ==================== СЛОТЫ ДЛЯ UI ====================

void TesterWindow::onConnectClicked()
{
    QString ip_t = ui->lineEditIPAddress->text();
    QString portStr = ui->lineEditPort->text();

    // Валидация IP
    QHostAddress addr;
    if (!addr.setAddress(ip_t)) {
        QMessageBox::warning(this, "Ошибка", "Неверный IP-адрес!");
        return;
    }

    // Валидация порта
    bool ok;
    quint16 port_t = portStr.toUShort(&ok);
    if (!ok || port_t == 0 || port_t > 65535) {
        QMessageBox::warning(this, "Ошибка", "Неверный порт!");
        return;
    }

    uint16_t address = getSelectedAddress();
    QString ip = getSelectedIP();
    quint16 port = getSelectedPort();


    PPBState state = m_controller->connectionState();

    if (state == PPBState::Idle) {
        m_controller->connectToPPB(address, ip, port);
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Подключение...");
    } else if (state == PPBState::Ready) {
        m_controller->disconnect();
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Отключение...");
    }
}

void TesterWindow::onPollStatusClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->requestStatus(address);
}

void TesterWindow::onResetClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->resetPPB(address);
}

void TesterWindow::onApplyParametersClicked()
{
    uint16_t address = getSelectedAddress();
    uint32_t duration = ui->lineEditPulseDuration->text().toUInt();
    uint8_t duty = static_cast<uint8_t>(ui->lineEditDutyCycle->text().toUInt());
    uint32_t delay = ui->lineEditPulseDelay->text().toUInt();

    m_controller->setGeneratorParameters(address, duration, duty, delay);
}

void TesterWindow::onAutoPollToggled(bool checked)
{
    if (checked) {
        m_controller->startAutoPoll(5000);
    } else {
        m_controller->stopAutoPoll();
    }
}

void TesterWindow::onTestSequenceClicked()
{
    if (m_controller->isBusy()) {
        QMessageBox::warning(this, "Внимание", "Уже выполняется другая операция");
        return;
    }

    setLeftPanelEnabled(false);
    uint16_t address = getSelectedAddress();
    m_controller->runFullTest(address);
}

void TesterWindow::onFUTransmitClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->setFUTransmit(address);
}

void TesterWindow::onFUReceiveClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->setFUReceive(address);
}

void TesterWindow::onPRBSM2SClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->startPRBS_M2S(address);
}

void TesterWindow::onPRBSS2MClicked()
{
    uint16_t address = getSelectedAddress();
    m_controller->startPRBS_S2M(address);
}

// ==================== СЛОТЫ ОТ КОНТРОЛЛЕРА ====================

void TesterWindow::onControllerConnectionStateChanged(PPBState state)
{
    updateConnectionUI();
    updateControlsState();
}

void TesterWindow::onControllerStatusReceived(uint16_t address, const QVector<QByteArray>& data)
{
    appendToLog(QString("Получен статус ППБ %1").arg(address));
}

void TesterWindow::onControllerErrorOccurred(const QString& error)
{
    appendToLog("[ОШИБКА] " + error);
    showStatusMessage(error, 5000);
    ui->pushButtonConnect->setEnabled(true);
    ui->pushButtonConnect->setText("Подключиться");
}

void TesterWindow::onControllerChannelStateUpdated(uint8_t ppbIndex, int channel, const UIChannelState& state)
{
    if (ppbIndex == m_currentPPBIndex) {
        if (channel == 1) {
            updateChannelDisplay(ui->groupBox_PPB1_chanel_1, state, m_displayAsCodes);
        } else if (channel == 2) {
            updateChannelDisplay(ui->groupBox_PPB1_chanel_2, state, m_displayAsCodes);
        }
    }
}

void TesterWindow::onControllerLogMessage(const QString& message)
{
    appendToLog(message);
}

void TesterWindow::onAutoPollToggledFromController(bool enabled)
{
    ui->checkBoxAutoPoll->setChecked(enabled);
}

void TesterWindow::onOperationProgress(int current, int total, const QString& operation)
{
    ui->statusbar->showMessage(QString("%1: %2/%3").arg(operation).arg(current).arg(total), 3000);

    // Можно добавить прогресс-бар, если нужно
    // ui->progressBar->setMaximum(total);
    // ui->progressBar->setValue(current);
}

void TesterWindow::onOperationCompleted(bool success, const QString& message)
{
    setLeftPanelEnabled(true);
    ui->statusbar->showMessage(message, 5000);

    if (!success) {
        QMessageBox::warning(this, "Ошибка операции", message);
    } else {
        appendToLog("Операция завершена: " + message);
    }
}

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

void TesterWindow::updateChannelDisplay(QGroupBox* channelBox, const UIChannelState& channel, bool showCodes)
{
    // Находим все элементы внутри groupBox
    QString suffix = channelBox->objectName().right(1); // "1" или "2"

    QLabel* powerCircle = channelBox->findChild<QLabel*>(QString("Power_circle_label_ppb_chanel_%1").arg(suffix));
    QLabel* powerText = channelBox->findChild<QLabel*>(QString("statuslabel_power_ppb_chanel_%1").arg(suffix));
    QLabel* capacityCircle = channelBox->findChild<QLabel*>(QString("Capacity_circle_label_ppb_chanel_%1").arg(suffix));
    QLabel* capacityText = channelBox->findChild<QLabel*>(QString("statuslabel_capacity_ppb_chanel_%1").arg(suffix));
    QLabel* vswrCircle = channelBox->findChild<QLabel*>(QString("KSWN_circle_label_ppb_chanel_%1").arg(suffix));
    QLabel* vswrText = channelBox->findChild<QLabel*>(QString("statuslabel_kswn_ppb_chanel_%1").arg(suffix));
    QLabel* tempCircle = channelBox->findChild<QLabel*>(QString("tem_circle_label_ppb_chanel_%1").arg(suffix));
    QLabel* tempText = channelBox->findChild<QLabel*>(QString("statuslabel_temp_ppb_chanel_%1").arg(suffix));

    if (!powerCircle || !powerText || !capacityCircle || !capacityText ||
        !vswrCircle || !vswrText || !tempCircle || !tempText) {
        Logger::error("Не найдены элементы интерфейса для канала");
        return;
    }

    // Стили для индикаторов
    QString styleNormal = "border-radius: 10px; border: 2px solid #666; background-color: #00ff00;";
    QString styleWarning = "border-radius: 10px; border: 2px solid #666; background-color: #ffff00;";
    QString styleAlarm = "border-radius: 10px; border: 2px solid #666; background-color: #ff0000;";

    // Питание
    if (channel.isOk) {
        powerCircle->setStyleSheet(styleNormal);
        powerText->setText(showCodes ? "0x01" : "В норме");
    } else {
        powerCircle->setStyleSheet(styleAlarm);
        powerText->setText(showCodes ? "0x00" : "АВАРИЯ");
    }

    // Мощность
    QString powerValue = formatPower(channel.power, showCodes);
    if (channel.power >= 1200.0f && channel.power <= 1300.0f) {
        capacityCircle->setStyleSheet(styleNormal);
        capacityText->setText(powerValue);
    } else if (channel.power >= 550.0f) {
        capacityCircle->setStyleSheet(styleWarning);
        capacityText->setText(powerValue);
    } else {
        capacityCircle->setStyleSheet(styleAlarm);
        capacityText->setText(powerValue);
    }

    // КСВН
    QString vswrValue = formatVSWR(channel.vswr, showCodes);
    if (channel.vswr <= 1.3f) {
        vswrCircle->setStyleSheet(styleNormal);
        vswrText->setText(vswrValue);
    } else if (channel.vswr <= 4.0f) {
        vswrCircle->setStyleSheet(styleWarning);
        vswrText->setText(vswrValue);
    } else {
        vswrCircle->setStyleSheet(styleAlarm);
        vswrText->setText(vswrValue);
    }

    // Температура
    QString tempValue = formatTemperature(channel.temperature, showCodes);
    if (channel.temperature <= 70.0f) {
        tempCircle->setStyleSheet(styleNormal);
        tempText->setText(tempValue);
    } else if (channel.temperature <= 85.0f) {
        tempCircle->setStyleSheet(styleWarning);
        tempText->setText(tempValue);
    } else {
        tempCircle->setStyleSheet(styleAlarm);
        tempText->setText(tempValue);
    }
}

QString TesterWindow::formatPower(float watts, bool showCodes) const
{
    if (showCodes) {
        uint16_t powerCode = DataConverter::powerToCode(watts);
        return QString("0x%1").arg(powerCode, 4, 16, QChar('0')).toUpper();
    } else {
        return QString("%1 Вт").arg(watts, 0, 'f', 1);
    }
}

QString TesterWindow::formatTemperature(float celsius, bool showCodes) const
{
    if (showCodes) {
        int16_t tempCode = DataConverter::temperatureToCode(celsius);
        return QString("0x%1").arg(static_cast<uint16_t>(tempCode), 4, 16, QChar('0')).toUpper();
    } else {
        return QString("%1°C").arg(celsius, 0, 'f', 1);
    }
}

QString TesterWindow::formatVSWR(float vswr, bool showCodes) const
{
    if (showCodes) {
        uint16_t vswrCode = DataConverter::vswrToCode(vswr);
        return QString("0x%1").arg(vswrCode, 4, 16, QChar('0')).toUpper();
    } else {
        return QString("%1").arg(vswr, 0, 'f', 2);
    }
}

void TesterWindow::appendToLog(const QString& message)
{
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("[hh:mm:ss] ");

    ui->textBrowser->append(timestamp + message);

    // Прокручиваем вниз
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
    ui->textBrowser->ensureCursorVisible();
}

void TesterWindow::showStatusMessage(const QString& message, int timeout)
{
    statusBar()->showMessage(message, timeout);
}

void TesterWindow::updateWindowTitle()
{
    QString title = "Программа управления ППБ";

    if (m_controller->isBusy()) {
        title += " [Выполняется операция]";
    }

    setWindowTitle(title);
}

void TesterWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isExiting) {
        if (m_controller->isBusy()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение",
                "Выполняется операция. Вы уверены, что хотите выйти?",
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No) {
                event->ignore();
                return;
            }
        }

        Logger::info("Программа завершена");
    }

    event->accept();
}

uint16_t TesterWindow::getSelectedAddress() const
{
    int index = ui->comboBoxPPBSelect->currentIndex();
    if (index >= 0 && index < 8) {
        return (1 << index);
    }
    return 0;
}

QString TesterWindow::getSelectedIP() const
{
    return ui->lineEditIPAddress->text();
}

quint16 TesterWindow::getSelectedPort() const
{
    return ui->lineEditPort->text().toUShort();
}

void TesterWindow::updateControlsState()
{
    bool isConnected = (m_controller->connectionState() == PPBState::Ready);
    bool isBusy = m_controller->isBusy();

    ui->pushButtonPollStatus->setEnabled(isConnected && !isBusy);
    ui->pushButtonReset->setEnabled(isConnected && !isBusy);
    ui->pushButtonApplyToPPB1->setEnabled(isConnected && !isBusy);
    ui->pushButtonTestSequence->setEnabled(isConnected && !isBusy);
    ui->radioButtonFUTransmit->setEnabled(isConnected && !isBusy);
    ui->radioButtonFUReceive->setEnabled(isConnected && !isBusy);
    ui->checkBoxAutoPoll->setEnabled(isConnected && !isBusy);
    ui->pushButtonConnect->setEnabled(!isBusy || isConnected);
}

void TesterWindow::updateConnectionUI()
{
    PPBState state = m_controller->connectionState();
    bool busy = m_controller->isBusy();

    QString style;
    QString text;
    QString tooltip;

    switch (state) {
    case PPBState::Idle:
        style = "border-radius: 10px; border: 2px solid #666; background-color: #ff0000;";
        text = "Подключиться";
        tooltip = "Отключено";
        ui->pushButtonConnect->setEnabled(true);
        break;
    case PPBState::Ready:
        if (busy) {
            style = "border-radius: 10px; border: 2px solid #666; background-color: #ffff00;";
            text = "Выполняется операция...";
            tooltip = "Выполняется операция";
            ui->pushButtonConnect->setEnabled(false);
        } else {
            style = "border-radius: 10px; border: 2px solid #666; background-color: #00ff00;";
            text = "Отключиться";
            tooltip = "Подключено";
            ui->pushButtonConnect->setEnabled(true);
        }
        break;
    default:
        style = "border-radius: 10px; border: 2px solid #666; background-color: #ffff00;";
        text = "Ожидание...";
        tooltip = "Ожидание";
        ui->pushButtonConnect->setEnabled(false);
        break;
    }

    ui->labelConnectionStatus->setStyleSheet(style);
    ui->labelConnectionStatus->setToolTip(tooltip);
    ui->pushButtonConnect->setText(text);
}

void TesterWindow::setLeftPanelEnabled(bool enabled)
{
    ui->groupBoxControl->setEnabled(enabled);
    ui->groupBoxParametrs->setEnabled(enabled);
    ui->groupBoxFU->setEnabled(enabled);
    ui->groupBoxScenarios->setEnabled(enabled);
    ui->groupBoxDataView->setEnabled(enabled);

    ui->pushButtonExit->setEnabled(true);

    ui->lineEditIPAddress->setEnabled(enabled);
    ui->lineEditPort->setEnabled(enabled);
    ui->pushButtonConnect->setEnabled(enabled);
}

// ==================== СЛОТЫ UI ====================

void TesterWindow::onDisplayModeChanged(bool showCodes)
{
    m_displayAsCodes = showCodes;
    // Обновляем отображение текущих каналов
    UIChannelState channel1 = m_controller->getChannelState(m_currentPPBIndex, 1);
    UIChannelState channel2 = m_controller->getChannelState(m_currentPPBIndex, 2);

    updateChannelDisplay(ui->groupBox_PPB1_chanel_1, channel1, showCodes);
    updateChannelDisplay(ui->groupBox_PPB1_chanel_2, channel2, showCodes);
}

void TesterWindow::onPPBSelected(int index)
{
    m_currentPPBIndex = static_cast<uint8_t>(index);

    // Обновляем отображение для выбранного ППБ
    UIChannelState channel1 = m_controller->getChannelState(m_currentPPBIndex, 1);
    UIChannelState channel2 = m_controller->getChannelState(m_currentPPBIndex, 2);

    updateChannelDisplay(ui->groupBox_PPB1_chanel_1, channel1, m_displayAsCodes);
    updateChannelDisplay(ui->groupBox_PPB1_chanel_2, channel2, m_displayAsCodes);
}

void TesterWindow::onClearLogClicked()
{
    ui->textBrowser->clear();
}

void TesterWindow::onExportLogClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт лога", "", "Текстовые файлы (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->textBrowser->toPlainText();
            file.close();
        }
    }
}

void TesterWindow::onExitClicked()
{
    close();
}

void TesterWindow::onPultActiveClicked()
{
    if (m_pultWindow) {
        m_pultWindow->show();
        m_pultWindow->raise();
        m_pultWindow->activateWindow();
        return;
    }

    uint16_t address = getSelectedAddress();

    // Создаем пульт с контроллером
    m_pultWindow = new pult(address, m_controller, nullptr);
    m_pultWindow->setAttribute(Qt::WA_DeleteOnClose);
    m_pultWindow->setWindowTitle(QString("Пульт управления ППБ %1").arg(ui->comboBoxPPBSelect->currentText()));
    m_pultWindow->show();

    // Соединяем сигнал закрытия для очистки указателя
    connect(m_pultWindow, &pult::destroyed, this, [this]() {
        m_pultWindow = nullptr;
    });

    Logger::info("Активирован пульт управления");
}



