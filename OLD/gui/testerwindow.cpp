#include "testerwindow.h"
#include "ui_testerwindow.h"
#include "../core/logger.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QCloseEvent>
#include <QHostAddress>

TesterWindow::TesterWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TesterWindow)
    , m_communication(nullptr)
    , m_currentPPBIndex(0)
    , m_displayAsCodes(false)
    , m_autoPollEnabled(false)
    , m_isExiting(false)
    , m_autoPollTimer(nullptr)
    , m_uiUpdateTimer(nullptr)
{
    ui->setupUi(this);

    // === ОТЛАДОЧНЫЙ ВЫВОД ===
#ifdef QT_DEBUG
    qDebug() << "=== Адресация ППБ (16 бит) ===";
    for (int i = 1; i <= 16; i++) {
        uint16_t addr = (1 << (i - 1));
        qDebug() << QString("ППБ%1: 0x%2 (бит %3)")
                        .arg(i, 2, 10, QChar('0'))
                        .arg(addr, 4, 16, QChar('0'))
                        .arg(i - 1);
    }
    qDebug() << "==============================\n";
#endif

    // Инициализация логгера
    Logger::init([this](const QString& logMessage) {
        appendToLog(logMessage);
    });

    // Инициализация компонентов
    initializeUI();
    initializeCommunication();
    initializeTimers();
    connectSignals();

    // Начальное обновление UI
    updateConnectionUI();
    updateControlsState();
    updateWindowTitle();

    Logger::info("Программа управления ППБ запущена");
}

TesterWindow::~TesterWindow()
{
    m_isExiting = true;

    // Останавливаем таймеры
    if (m_autoPollTimer) m_autoPollTimer->stop();
    if (m_uiUpdateTimer) m_uiUpdateTimer->stop();

    // Отключаемся от ППБ
    if (m_communication) {
        m_communication->disconnect();
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
    ui->lineEditIPAddress->setText("255.255.255.255");
    ui->lineEditPort->setText("12345");

    ui->lineEditPulseDuration->setText("27000");
    ui->lineEditDutyCycle->setText("3");
    ui->lineEditPulseDelay->setText("0");

    // Настройка индикаторов (круги)
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

    // Настройка радио-кнопок
    ui->radioButtonShowPhysical->setChecked(true);
    ui->radioButtonFUReceive->setChecked(true);

    // Инициализируем состояние каналов
    for (int i = 0; i < 8; ++i) {
        UIChannelState defaultState;
        defaultState.power = 1250.0f;
        defaultState.temperature = 45.0f;
        defaultState.vswr = 1.2f;
        defaultState.isOk = true;

        m_channel1States[i] = defaultState;
        m_channel2States[i] = defaultState;
    }
}

void TesterWindow::initializeCommunication()
{
    m_communication = new PPBCommunication(this);
}

void TesterWindow::initializeTimers()
{
    m_autoPollTimer = new QTimer(this);
    m_autoPollTimer->setSingleShot(false);
    connect(m_autoPollTimer, &QTimer::timeout, this, &TesterWindow::onAutoPollTimeout);

    m_uiUpdateTimer = new QTimer(this);
    m_uiUpdateTimer->setSingleShot(false);
    m_uiUpdateTimer->start(100); // Обновление UI каждые 100 мс
    connect(m_uiUpdateTimer, &QTimer::timeout, this, &TesterWindow::onUIUpdateTimeout);
}

void TesterWindow::connectSignals()
{
    // === КНОПКИ УПРАВЛЕНИЯ ===

    // Сеть
    connect(ui->pushButtonConnect, &QPushButton::clicked,
            this, &TesterWindow::onConnectClicked);

    // Управление ППБ
    connect(ui->pushButtonPollStatus, &QPushButton::clicked,
            this, &TesterWindow::onPollStatusClicked);
    connect(ui->pushButtonReset, &QPushButton::clicked,
            this, &TesterWindow::onResetClicked);

    // Параметры генератора
    connect(ui->pushButtonApplyToPPB1, &QPushButton::clicked,
            this, &TesterWindow::onApplyParametersClicked);

    // Режим ФУ
    connect(ui->radioButtonFUTransmit, &QRadioButton::clicked,
            this, &TesterWindow::onFUTransmitClicked);
    connect(ui->radioButtonFUReceive, &QRadioButton::clicked,
            this, &TesterWindow::onFUReceiveClicked);

    // Тестовая последовательность
    connect(ui->pushButtonTestSequence, &QPushButton::clicked,
            this, &TesterWindow::onTestSequenceClicked);

    // Автоопрос
    connect(ui->checkBoxAutoPoll, &QCheckBox::toggled,
            this, &TesterWindow::onAutoPollToggled);

    // Сценарии
    connect(ui->pushButtonLoadScenario, &QPushButton::clicked,
            this, &TesterWindow::onLoadScenarioClicked);
    connect(ui->pushButtonRunScenario, &QPushButton::clicked,
            this, &TesterWindow::onRunScenarioClicked);
    connect(ui->pushButtonStopScenario, &QPushButton::clicked,
            this, &TesterWindow::onStopScenarioClicked);

    // Выбор ППБ
    connect(ui->comboBoxPPBSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TesterWindow::onPPBSelected);

    // Отображение
    connect(ui->radioButtonShowCodes, &QRadioButton::toggled,
            this, &TesterWindow::onDisplayModeChanged);

    // Логи
    connect(ui->pushButtonClearLog, &QPushButton::clicked,
            this, &TesterWindow::onClearLogClicked);
    connect(ui->pushButtonExportLog, &QPushButton::clicked,
            this, &TesterWindow::onExportLogClicked);

    // Выход
    connect(ui->pushButtonExit, &QPushButton::clicked,
            this, &TesterWindow::onExitClicked);

    // === СИГНАЛЫ ОТ PPBCOMMUNICATION ===
    connect(m_communication, &PPBCommunication::stateChanged,
            this, &TesterWindow::onConnectionStateChanged);
    connect(m_communication, &PPBCommunication::busyChanged,
            this, &TesterWindow::onBusyChanged);
    connect(m_communication, &PPBCommunication::connected,
            this, &TesterWindow::onConnected);
    connect(m_communication, &PPBCommunication::disconnected,
            this, &TesterWindow::onDisconnected);
    connect(m_communication, &PPBCommunication::statusReceived,
            this, &TesterWindow::onStatusReceived);
    connect(m_communication, &PPBCommunication::testProgress,
            this, &TesterWindow::onTestProgress);
    connect(m_communication, &PPBCommunication::testCompleted,
            this, &TesterWindow::onTestCompleted);
    connect(m_communication, &PPBCommunication::testPacketReceived,
            this, &TesterWindow::onTestPacketReceived);
    connect(m_communication, &PPBCommunication::errorOccurred,
            this, &TesterWindow::onErrorOccurred);
    connect(m_communication, &PPBCommunication::timeoutOccurred,
            this, &TesterWindow::onTimeoutOccurred);
    Logger::init([this](const QString& logMessage) {
        QMetaObject::invokeMethod(this, [this, logMessage]() {
            appendToLog(logMessage);
        });
    });
}

// === СЛОТЫ ДЛЯ КНОПОК ===

void TesterWindow::onConnectClicked()
{
    PPBState currentState = m_communication->state();



    if (currentState == PPBState::Disconnected) {

        /* Получаем адрес из comboBox (используем 0 для TS команды)
        uint16_t index = static_cast<uint16_t>(ui->comboBoxPPBSelect->currentIndex());
         uint16_t address = index;
        */
        uint8_t address = getSelectedAddress();
        QString ip = getSelectedIP();
        quint16 port = getSelectedPort();

        // Блокируем кнопку сразу
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Подключение...");

        if (m_communication->connectToPPB(address, ip, port)) {
            // UI обновится по сигналам
        } else {
            // Восстанавливаем кнопку если ошибка сразу
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonConnect->setText("Подключиться");
        }
    } else if (currentState == PPBState::Connected) {
        // Отключаемся
        m_communication->disconnect();
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Отключение...");
    }
    // В состоянии Connecting ничего не делаем - кнопка уже заблокирована

}

void TesterWindow::onPollStatusClicked()
{
    uint8_t address = getSelectedAddress();
    m_communication->sendStatusRequest(address);
}

void TesterWindow::onResetClicked()
{
    uint8_t address = getSelectedAddress();
    m_communication->sendReset(address);
}

void TesterWindow::onApplyParametersClicked()
{
    uint8_t address = getSelectedAddress();

    // Получаем параметры из UI
    uint32_t duration = ui->lineEditPulseDuration->text().toUInt();
    uint8_t duty = static_cast<uint8_t>(ui->lineEditDutyCycle->text().toUInt());
    uint32_t delay = ui->lineEditPulseDelay->text().toUInt();

    // TODO: Реализовать отправку параметров через ФУ
    Logger::info(QString("Применение параметров к ППБ%1: Длительность=%2 мкс, Скважность=%3, Задержка=%4 мкс")
                     .arg(address + 1).arg(duration).arg(duty).arg(delay));
}

void TesterWindow::onAutoPollToggled(bool checked)
{
    m_autoPollEnabled = checked;

    if (checked) {
        m_autoPollTimer->start(5000); // Каждые 5 секунд
        Logger::info("Автоопрос включен (интервал 5 сек)");
    } else {
        m_autoPollTimer->stop();
        Logger::info("Автоопрос выключен");
    }
}

void TesterWindow::onFUTransmitClicked()
{
    uint8_t address = getSelectedAddress();
    m_communication->sendFUTransmit(address);
}

void TesterWindow::onFUReceiveClicked()
{
    uint8_t address = getSelectedAddress();
    uint8_t period = 0; // TODO: Получить период из UI, если нужно

    m_communication->sendFUReceive(address, period);
}

void TesterWindow::onTestSequenceClicked()
{
    uint8_t address = getSelectedAddress();

    if (m_communication->isBusy()) {
        QMessageBox::warning(this, "Внимание", "Уже выполняется другая операция");
        return;
    }

    // Блокируем левую панель
    setLeftPanelEnabled(false);

    // Запускаем полный тест
    m_communication->sendFullTest(address);

    Logger::info(QString("Запущен полный тестовый цикл для ППБ%1").arg(address + 1));
}

void TesterWindow::onLoadScenarioClicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Выберите файл сценария",
                                                    "",
                                                    "Сценарии (*.scn);;Все файлы (*.*)");

    if (!filename.isEmpty()) {
        // TODO: Реализовать загрузку сценария
        ui->labelScenarioFile->setText(QFileInfo(filename).fileName());
        Logger::info(QString("Загружен сценарий: %1").arg(filename));
    }
}

void TesterWindow::onRunScenarioClicked()
{
    // TODO: Реализовать запуск сценария
    Logger::info("Запуск сценария (заглушка)");
}

void TesterWindow::onStopScenarioClicked()
{
    // TODO: Реализовать остановку сценария
    Logger::info("Остановка сценария (заглушка)");
}

void TesterWindow::onDisplayModeChanged(bool showCodes)
{
    m_displayAsCodes = showCodes;

    // Обновляем отображение текущего ППБ
    uint8_t address = m_currentPPBIndex;

    // Обновляем канал 1
    if (m_channel1States.contains(address)) {
        updateChannelDisplay(ui->groupBox_PPB1_chanel_1,
                             m_channel1States[address],
                             m_displayAsCodes);
    }

    // Обновляем канал 2
    if (m_channel2States.contains(address)) {
        updateChannelDisplay(ui->groupBox_PPB1_chanel_2,
                             m_channel2States[address],
                             m_displayAsCodes);
    }

    Logger::info(QString("Режим отображения: %1")
                     .arg(showCodes ? "коды" : "физические величины"));
}

void TesterWindow::onPPBSelected(int index)
{
    if (index >= 0 && index < 8) {
        m_currentPPBIndex = static_cast<uint8_t>(index);
        ui->tabWidget->setCurrentIndex(index);

        // Обновляем отображение каналов для выбранного ППБ
        if (m_channel1States.contains(m_currentPPBIndex)) {
            updateChannelDisplay(ui->groupBox_PPB1_chanel_1,
                                 m_channel1States[m_currentPPBIndex],
                                 m_displayAsCodes);
        }

        if (m_channel2States.contains(m_currentPPBIndex)) {
            updateChannelDisplay(ui->groupBox_PPB1_chanel_2,
                                 m_channel2States[m_currentPPBIndex],
                                 m_displayAsCodes);
        }

        Logger::info(QString("Выбран ППБ%1").arg(index + 1));
    }
}

void TesterWindow::onClearLogClicked()
{
    clearLog();
}

void TesterWindow::onExportLogClicked()
{
    exportLog();
}

void TesterWindow::onExitClicked()
{
    close();
}

// === СЛОТЫ ОТ PPBCOMMUNICATION ===

void TesterWindow::onConnectionStateChanged(PPBState state)
{
    updateConnectionUI();
    updateControlsState();
    if (state == PPBState::Disconnected) {
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setText("Подключиться");
        setLeftPanelEnabled(true);
    }
}

void TesterWindow::onBusyChanged(bool busy)
{
    if (!busy) {
        // Разблокируем левую панель, когда операция завершена
        setLeftPanelEnabled(true);
    }

    // Обновляем состояние кнопок
    updateControlsState();
}

void TesterWindow::onConnected()
{
    showStatusMessage("Подключено к ППБ");
    Logger::info("Подключение к ППБ установлено");

    // Автоматически запрашиваем статус после подключения
  //  onPollStatusClicked();
}

void TesterWindow::onDisconnected()
{
    showStatusMessage("Отключено от ППБ");
    Logger::info("Отключено от ППБ");

    // Разблокируем левую панель
    setLeftPanelEnabled(true);
}

void TesterWindow::onStatusReceived(uint16_t address, const QVector<QByteArray>& data)
{
    // Определяем, какой ППБ ответил (может быть несколько, если широковещательный запрос)
    QStringList respondingPPBs;

    if (address & 0x0001) respondingPPBs << "ППБ1";
    if (address & 0x0002) respondingPPBs << "ППБ2";
    if (address & 0x0004) respondingPPBs << "ППБ3";
    if (address & 0x0008) respondingPPBs << "ППБ4";
    if (address & 0x0010) respondingPPBs << "ППБ5";
    if (address & 0x0020) respondingPPBs << "ППБ6";
    if (address & 0x0040) respondingPPBs << "ППБ7";
    if (address & 0x0080) respondingPPBs << "ППБ8";

    if (respondingPPBs.isEmpty()) {
        Logger::error(QString("Получен ответ с некорректным адресом: 0x%1")
                          .arg(address, 4, 16, QChar('0')));
        return;
    }

    // Если ответил один ППБ (обычный случай)
    if (respondingPPBs.size() == 1) {
        // Определяем индекс (0-7) из адреса
        int index = -1;
        uint16_t mask = address & 0x00FF; // Берем только младший байт

        switch (mask) {
        case 0x0001: index = 0; break; // ППБ1
        case 0x0002: index = 1; break; // ППБ2
        case 0x0004: index = 2; break; // ППБ3
        case 0x0008: index = 3; break; // ППБ4
        case 0x0010: index = 4; break; // ППБ5
        case 0x0020: index = 5; break; // ППБ6
        case 0x0040: index = 6; break; // ППБ7
        case 0x0080: index = 7; break; // ППБ8
        default: break;
        }

        if (index != -1) {
            processStatusData(index, data);
            Logger::info(QString("Статус %1 получен").arg(respondingPPBs.first()));
        }
    } else {
        // Ответили несколько ППБ (широковещательный запрос)
        Logger::info(QString("Ответ от нескольких ППБ: %1")
                         .arg(respondingPPBs.join(", ")));

    }
}

void TesterWindow::onTestProgress(int current, int total, OperationType operation)
{
    // Обновляем прогресс в UI
    QString operationName;
    switch (operation) {
    case OperationType::TransmitTest: operationName = "отправка"; break;
    case OperationType::ReceiveTest: operationName = "прием"; break;
    case OperationType::FullTest: operationName = "полный тест"; break;
    default: operationName = "тест"; break;
    }

    statusBar()->showMessage(QString("%1: %2/%3").arg(operationName).arg(current).arg(total), 100);

    // Логируем каждые 10 пакетов
    if (current % 10 == 0) {
        Logger::debug(QString("Прогресс %1: %2/%3").arg(operationName).arg(current).arg(total));
    }
}

void TesterWindow::onTestCompleted(bool success, const QString& report, OperationType operation)
{
    Q_UNUSED(success); // Нам теперь не важно success, мы всегда показываем анализ

    // Создаем свое окно для отображения анализа
    QDialog* analysisDialog = new QDialog(this);
    analysisDialog->setWindowTitle("Результат анализа тестовой последовательности");
    analysisDialog->setMinimumSize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(analysisDialog);

    QTextEdit* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlainText(report);
    layout->addWidget(textEdit);

    QPushButton* closeButton = new QPushButton("Закрыть");
    connect(closeButton, &QPushButton::clicked, analysisDialog, &QDialog::accept);
    layout->addWidget(closeButton);

    analysisDialog->exec();

    statusBar()->clearMessage();
}

void TesterWindow::onTestPacketReceived(const DataPacket& packet)
{
    // Просто логируем получение тестовых пакетов
    // Можно добавить визуализацию, если нужно
    Q_UNUSED(packet);
}

void TesterWindow::onErrorOccurred(const QString& error)
{
    Logger::error(error);
    showStatusMessage(error, 5000);

    // При любой ошибке разблокируем кнопку подключения
    ui->pushButtonConnect->setEnabled(true);
    ui->pushButtonConnect->setText("Подключиться");
}

void TesterWindow::onTimeoutOccurred(const QString& operation)
{
    Logger::warning(QString("Таймаут операции: %1").arg(operation));
    showStatusMessage(QString("Таймаут: %1").arg(operation), 5000);
}

void TesterWindow::onLogMessage(const QString& message)
{
    appendToLog(message);
}

// === ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===

uint16_t TesterWindow::getSelectedAddress() const
{
    // Получаем номер ППБ из comboBox (1-8, но может быть 1-16 если расширить)
    int ppbNumber = ui->comboBoxPPBSelect->currentIndex() + 1; // 1-8

    // Преобразуем номер ППБ в битовую маску
    // ППБ1 -> бит 0 -> 0x0001
    // ППБ2 -> бит 1 -> 0x0002
    // ...
    // ППБ8 -> бит 7 -> 0x0080
    // ППБ9 -> бит 8 -> 0x0100 (если будет в будущем)
    // ППБ16 -> бит 15 -> 0x8000 (если будет в будущем)

    if (ppbNumber >= 1 && ppbNumber <= 16) {
        return (1 << (ppbNumber - 1));
    }

    // Широковещание на все ППБ
    return 0xFFFF;
}

uint16_t getPPBAddressBitmask(int ppbNumber) // ppbNumber = 1-8
{
    static const uint16_t ppbAddresses[8] = {
        0x0001, // ППБ1
        0x0002, // ППБ2
        0x0004, // ППБ3
        0x0008, // ППБ4
        0x0010, // ППБ5
        0x0020, // ППБ6
        0x0040, // ППБ7
        0x0080  // ППБ8
    };

    if (ppbNumber >= 1 && ppbNumber <= 8) {
        return ppbAddresses[ppbNumber - 1];
    }
    return 0x00FF; // Широковещание
}

QString TesterWindow::getSelectedIP() const
{
    return ui->lineEditIPAddress->text().trimmed();
}

quint16 TesterWindow::getSelectedPort() const
{
    bool ok;
    quint16 port = ui->lineEditPort->text().toUShort(&ok);
    return ok ? port : 12345;
}

void TesterWindow::updateConnectionUI()
{
    PPBState state = m_communication->state();
    bool busy = m_communication->isBusy();

    QString style;
    QString text;
    QString tooltip;

    switch (state) {
    case PPBState::Disconnected:
        style = "border-radius: 10px; border: 2px solid #666; background-color: #ff0000;";
        text = "Подключиться";
        tooltip = "Отключено";
        ui->pushButtonConnect->setEnabled(true);
        break;
    case PPBState::Connected:
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

void TesterWindow::updateControlsState()
{
    PPBState state = m_communication->state();
    bool busy = m_communication->isBusy();

    // Поля IP/port ВСЕГДА доступны
    ui->lineEditIPAddress->setEnabled(true);
    ui->lineEditPort->setEnabled(true);

    // Кнопка подключения
    if (state == PPBState::Disconnected) {
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setText("Подключиться");
    } else if (state == PPBState::Connecting) {
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Подключение...");
    } else if (state == PPBState::Connected) {
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setText("Отключиться");
    }

    // Остальные элементы только при подключении
    bool connected = (state == PPBState::Connected && !busy);
    ui->groupBoxControl->setEnabled(connected);
    ui->groupBoxParametrs->setEnabled(connected);
    ui->groupBoxFU->setEnabled(connected);
    ui->groupBoxScenarios->setEnabled(connected);
    ui->pushButtonTestSequence->setEnabled(connected);
}
void TesterWindow::setLeftPanelEnabled(bool enabled)
{
    // Блокируем/разблокируем все groupBox в левой панели
    ui->groupBoxControl->setEnabled(enabled);
    ui->groupBoxParametrs->setEnabled(enabled);
    ui->groupBoxFU->setEnabled(enabled);
    ui->groupBoxScenarios->setEnabled(enabled);
    ui->groupBoxDataView->setEnabled(enabled);

    // Но кнопку Exit всегда оставляем активной
    ui->pushButtonExit->setEnabled(true);

    // Также оставляем активными сетевые настройки
    ui->lineEditIPAddress->setEnabled(enabled);
    ui->lineEditPort->setEnabled(enabled);
    ui->pushButtonConnect->setEnabled(enabled);
}

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
    QString powerValue;
    if (showCodes) {
        uint16_t powerCode = DataConverter::powerToCode(channel.power);
        powerValue = QString("0x%1").arg(powerCode, 4, 16, QChar('0')).toUpper();
    } else {
        powerValue = QString("%1 Вт").arg(channel.power, 0, 'f', 1);
    }

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
    QString vswrValue;
    if (showCodes) {
        uint16_t vswrCode = DataConverter::vswrToCode(channel.vswr);
        vswrValue = QString("0x%1").arg(vswrCode, 4, 16, QChar('0')).toUpper();
    } else {
        vswrValue = QString("%1").arg(channel.vswr, 0, 'f', 2);
    }

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
    QString tempValue;
    if (showCodes) {
        int16_t tempCode = DataConverter::temperatureToCode(channel.temperature);
        tempValue = QString("0x%1").arg(static_cast<uint16_t>(tempCode), 4, 16, QChar('0')).toUpper();
    } else {
        tempValue = QString("%1°C").arg(channel.temperature, 0, 'f', 1);
    }

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

void TesterWindow::onAutoPollTimeout()
{
    if (m_autoPollEnabled && m_communication->state() == PPBState::Connected && !m_communication->isBusy()) {
        onPollStatusClicked();
    }
}

void TesterWindow::onUIUpdateTimeout()
{
    // Периодическое обновление UI
    updateWindowTitle();
    updateConnectionUI();
}

void TesterWindow::processStatusData(uint16_t address, const QVector<QByteArray>& data)
{
    // TODO: Реализовать парсинг реальных данных техсостояния
    // Сейчас используем заглушку

    // Для теста создаем фиктивные данные
    UIChannelState channel1;
    channel1.power = 1250.0f + (rand() % 100 - 50);
    channel1.temperature = 45.0f + (rand() % 10 - 5);
    channel1.vswr = 1.2f + (rand() % 10) / 50.0f;
    channel1.isOk = true;

    UIChannelState channel2;
    channel2.power = 1250.0f + (rand() % 100 - 50);
    channel2.temperature = 45.0f + (rand() % 10 - 5);
    channel2.vswr = 1.2f + (rand() % 10) / 50.0f;
    channel2.isOk = true;

    m_channel1States[address] = channel1;
    m_channel2States[address] = channel2;

    // Обновляем UI если это текущий ППБ
    if (address == m_currentPPBIndex) {
        updateChannelDisplay(ui->groupBox_PPB1_chanel_1, channel1, m_displayAsCodes);
        updateChannelDisplay(ui->groupBox_PPB1_chanel_2, channel2, m_displayAsCodes);
    }

    Logger::info(QString("Данные техсостояния ППБ%1 обработаны").arg(address + 1));
}

void TesterWindow::appendToLog(const QString& message)
{
    ui->textBrowser->append(message);

    // Прокручиваем вниз
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
    ui->textBrowser->ensureCursorVisible();
}

void TesterWindow::clearLog()
{
    ui->textBrowser->clear();
    Logger::info("Окно логов очищено");
}

void TesterWindow::exportLog()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Экспорт логов",
                                                    QString("logs/ppb_log_%1.txt")
                                                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
                                                    "Текстовые файлы (*.txt);;Все файлы (*.*)");

    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->textBrowser->toPlainText();
            file.close();
            Logger::info(QString("Логи экспортированы в: %1").arg(filename));
        } else {
            Logger::error(QString("Не удалось экспортировать логи в: %1").arg(filename));
        }
    }
}

void TesterWindow::showStatusMessage(const QString& message, int timeout)
{
    statusBar()->showMessage(message, timeout);
}

void TesterWindow::updateWindowTitle()
{
    QString title = "Программа управления ППБ";

    if (m_communication->isBusy()) {
        title += " [Выполняется операция]";
    }

    setWindowTitle(title);
}

void TesterWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isExiting) {
        if (m_communication->isBusy()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение",
                "Выполняется операция. Вы уверены, что хотите выйти?",
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No) {
                event->ignore();
                return;
            }
        }

        if (m_communication->state() != PPBState::Disconnected) {
            m_communication->disconnect();
        }

        Logger::info("Программа завершена");
    }

    event->accept();
}


void testPPBAddressing()
{
    qDebug() << "=== Адресация ППБ (16 бит) ===";

    for (int ppbNumber = 1; ppbNumber <= 16; ppbNumber++) {
        uint16_t address = (1 << (ppbNumber - 1));

        QString binary = QString::number(address, 2).rightJustified(16, '0');
        QString hex = QString("0x%1").arg(address, 4, 16, QChar('0')).toUpper();

        // Разделяем на два байта для наглядности
        QString binaryFormatted = binary.left(8) + " " + binary.right(8);

        qDebug() << QString("ППБ%1").arg(ppbNumber, 2, 10, QChar('0'))
                 << "Адрес:" << hex
                 << "Бинарно: [" << binaryFormatted << "]"
                 << QString("(бит %1)").arg(ppbNumber - 1);

}
    qDebug() << "\n=== Примеры комбинаций ===";

    // ППБ1 + ППБ3
    uint16_t ppb1_3 = (1 << 0) | (1 << 2); // 0x0001 | 0x0004 = 0x0005
    qDebug() << "ППБ1 + ППБ3: 0x" << QString::number(ppb1_3, 16).rightJustified(4, '0');

    // ППБ9 (бит 8)
    uint16_t ppb9 = (1 << 8); // 0x0100
    qDebug() << "ППБ9: 0x" << QString::number(ppb9, 16).rightJustified(4, '0')
             << "(бит 8)";

    // Широковещание на все 16 ППБ
    uint16_t broadcast = 0xFFFF;
    qDebug() << "Широковещание (все ППБ): 0x" << QString::number(broadcast, 16);
}
