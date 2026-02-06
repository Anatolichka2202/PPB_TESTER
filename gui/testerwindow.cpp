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
#include "../core/logwrapper.h"
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

    setupLogging();

 QTimer::singleShot(0, this, [this]() {
        if (!m_controller) {
            LOG_ERROR("TesterWindow: контроллер не передан!");
            QMessageBox::critical(this, "Ошибка",
                                  "Контроллер не инициализирован. Приложение будет закрыто.");
            return;
        }

    //LogWrapper::instance();

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

    LOG_CAT_INFO("UI", "Программа управления ППБ запущена");
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

void TesterWindow::setupLogging()
{
    // Подключаемся к сигналу LogWrapper
    connect(LogWrapper::instance(), &LogWrapper::logEntryReceived,
            this, &TesterWindow::onLogEntryReceived, Qt::QueuedConnection);

    // Настраиваем QTextBrowser для отображения HTML
    ui->textBrowser->setOpenExternalLinks(false);
    ui->textBrowser->setReadOnly(true);

    // Добавляем CSS стили
    QString css = R"(
        .log-entry {
            padding: 2px 5px;
            margin: 1px 0;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 10pt;
            white-space: pre;
        }

        .log-debug {
            color: #006400; /* темно-зеленый */
            background-color: #f0fff0;
        }

        .log-info {
            color: #000000;
            background-color: #ffffff;
        }

        .log-warning {
            color: #ff8c00; /* темно-оранжевый */
            background-color: #fff8dc;
            font-weight: bold;
        }

        .log-error {
            color: #8b0000; /* темно-красный */
            background-color: #fff0f0;
            font-weight: bold;
        }

        .log-time {
            color: #808080; /* серый */
            font-weight: normal;
        }

        .log-level {
            color: #0000ff; /* синий */
            font-weight: bold;
        }

        .log-category {
            color: #800080; /* фиолетовый */
            font-weight: bold;
        }

        .log-message {
            color: #000000;
            font-weight: normal;
        }

        /* Стили для категорий */
        .log-category-controller {
            border-left: 3px solid #0066cc;
            margin-left: 2px;
        }

        .log-category-engine {
            border-left: 3px solid #cc6600;
            margin-left: 2px;
        }

        .log-category-ppbcom {
            border-left: 3px solid #6600cc;
            margin-left: 2px;
        }

        .log-category-command {
            border-left: 3px solid #00cc66;
            margin-left: 2px;
        }

        .log-category-ui {
            border-left: 3px solid #cc0066;
            margin-left: 2px;
        }

        .log-category-udp {
            border-left: 3px solid #00cccc;
            margin-left: 2px;
        }

        .log-category-general {
            border-left: 3px solid #cccccc;
            margin-left: 2px;
        }

        /* Табличный стиль для структурированных данных */
        .log-table {
            border-collapse: collapse;
            margin: 5px 0;
            width: 100%;
        }

        .log-table td, .log-table th {
            border: 1px solid #ddd;
            padding: 4px 8px;
        }

        .log-table th {
            background-color: #f2f2f2;
            text-align: left;
        }
    )";

    ui->textBrowser->document()->setDefaultStyleSheet(css);

    LOG_CAT_DEBUG("UI", "Настройка логирования завершена");
}

// Слот для получения логов
void TesterWindow::onLogEntryReceived(const LogEntry& entry)
{
    // Сохраняем запись для возможного экспорта
    m_logEntries.append(entry);

    // Ограничиваем размер хранимых логов (например, последние 10000 записей)
    const int MAX_LOG_ENTRIES = 10000;
    if (m_logEntries.size() > MAX_LOG_ENTRIES) {
        m_logEntries.remove(0, m_logEntries.size() - MAX_LOG_ENTRIES);
    }

    // Форматируем и отображаем
    QString html = formatLogEntryToHtml(entry);

    // Используем QMetaObject для гарантии работы в UI потоке
    QMetaObject::invokeMethod(this, [this, html]() {
        ui->textBrowser->append(html);

        // Прокручиваем вниз
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textBrowser->setTextCursor(cursor);
        ui->textBrowser->ensureCursorVisible();
    }, Qt::QueuedConnection);
}

// Метод форматирования LogEntry в HTML
QString TesterWindow::formatLogEntryToHtml(const LogEntry& entry)
{
    QString cssClass = QString("log-%1 log-category-%2")
    .arg(entry.level.toLower())
        .arg(entry.category.toLower());

    // Форматируем время
    QString timeStr = entry.timestamp.toString("hh:mm:ss.zzz");

    // Форматируем сообщение (обрабатываем переносы строк)
    QString message = entry.message.toHtmlEscaped();
    message.replace("\n", "<br/>");

    // Создаем HTML
    QString html = QString(
                       "<div class='log-entry %1'>"
                       "<span class='log-time'>%2</span> "
                       "<span class='log-level'>[%3]</span> "
                       "<span class='log-category'>[%4]</span> "
                       "<span class='log-message'>%5</span>"
                       "</div>")
                       .arg(cssClass)
                       .arg(timeStr)
                       .arg(entry.level)
                       .arg(entry.category)
                       .arg(message);

    return html;
}

// Метод форматирования LogEntry в простой текст
QString TesterWindow::formatLogEntryToPlainText(const LogEntry& entry)
{
    return entry.toString();
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
    // 1. Изменение состояния подключения
    connect(m_controller, &PPBController::connectionStateChanged,
            this, &TesterWindow::onControllerConnectionStateChanged);

    // 2. Получение статуса от ППБ (РАНЬШЕ НЕ БЫЛО ПОДКЛЮЧЕНО!)
    connect(m_controller, &PPBController::statusReceived,
            this, &TesterWindow::onControllerStatusReceived);

    // 3. Ошибки при работе
    connect(m_controller, &PPBController::errorOccurred,
            this, &TesterWindow::onControllerErrorOccurred);

    /*/ 4. Сообщения для лога
    connect(m_controller, &PPBController::logMessage,
            this, &TesterWindow::onControllerLogMessage);
*/
    // 5. Обновление состояния каналов
    connect(m_controller, &PPBController::channelStateUpdated,
            this, &TesterWindow::onControllerChannelStateUpdated);

    // 6. Изменение состояния автоопроса
    connect(m_controller, &PPBController::autoPollToggled,
            this, &TesterWindow::onAutoPollToggledFromController);

    // 7. Прогресс выполнения операций
    connect(m_controller, &PPBController::operationProgress,
            this, &TesterWindow::onOperationProgress);

    // 8. Завершение операций
    connect(m_controller, &PPBController::operationCompleted,
            this, &TesterWindow::onOperationCompleted);

    // 9. ИЗМЕНЕНИЕ СОСТОЯНИЯ ЗАНЯТОСТИ (НОВОЕ!)
    connect(m_controller, &PPBController::busyChanged,
            this, &TesterWindow::onControllerBusyChanged);

    LOG_DEBUG("TesterWindow: все сигналы подключены");
}

// ==================== СЛОТЫ ДЛЯ UI ====================

void TesterWindow::onConnectClicked()
{
    QString ip = ui->lineEditIPAddress->text();
    QString portStr = ui->lineEditPort->text();

    // Валидация IP
    QHostAddress addr;
    if (!addr.setAddress(ip)) {
        QMessageBox::warning(this, "Ошибка", "Неверный IP-адрес!");
        return;
    }

    // Валидация порта
    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok || port == 0 || port > 65535) {
        QMessageBox::warning(this, "Ошибка", "Неверный порт!");
        return;
    }

    uint16_t address = getSelectedAddress();
    PPBState state = m_controller->connectionState();

    // ВАЖНО: НЕ БЛОКИРУЕМ кнопку здесь!
    // Состояние кнопки обновится АВТОМАТИЧЕСКИ через onControllerConnectionStateChanged

    if (state == PPBState::Idle) {
        LOG_DEBUG("TesterWindow: запрос подключения к адресу 0x" +
                  QString::number(address, 16));
        m_controller->connectToPPB(address, ip, port);
    } else if (state == PPBState::Ready) {
        LOG_DEBUG("TesterWindow: запрос отключения");
        m_controller->disconnect();
    }
    // Для других состояний (SendingCommand, WaitingData) не делаем ничего
    // Кнопка уже должна быть заблокирована через updateControlsState()
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
    // Логируем получение статуса
    LOG_DEBUG(QString("TesterWindow: получен статус ППБ 0x%1, %2 пакетов")
                  .arg(address, 4, 16, QChar('0')).arg(data.size()));

    // Можно добавить дополнительную обработку здесь
    // Например, парсить данные и обновлять другие элементы UI

    // Пока просто добавляем в лог
    LOG_CAT_INFO("CONTROLLER", QString("Статус ППБ %1 получен (%2 пакетов)")
                    .arg(address).arg(data.size()));
}

void TesterWindow::onControllerErrorOccurred(const QString& error)
{
    // Вместо appendToLog - логируем через новую систему
    LOG_CAT_ERROR("CONTROLLER", error);

    // Для статусной строки  прямой вызов
    showStatusMessage(error, 5000);

    updateControlsState();
    updateConnectionUI();

    LOG_CAT_DEBUG("UI", "Обработана ошибка: " + error);
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
        LOG_CAT_INFO("CONTROLLER","Операция завершена: " + message);
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
    PPBState state = m_controller->connectionState();
    bool isBusy = m_controller->isBusy();

    LOG_DEBUG(QString("TesterWindow::updateControlsState: state=%1, busy=%2")
                  .arg(static_cast<int>(state)).arg(isBusy));

    // Определяем, подключены ли мы (только состояние Ready означает подключение)
    bool isConnected = (state == PPBState::Ready);

    // === ЭЛЕМЕНТЫ, ДОСТУПНЫЕ ТОЛЬКО ПРИ ПОДКЛЮЧЕНИИ И НЕ ВО ВРЕМЯ ОПЕРАЦИИ ===
    bool connectedAndNotBusy = isConnected && !isBusy;

    ui->pushButtonPollStatus->setEnabled(connectedAndNotBusy);
    ui->pushButtonReset->setEnabled(connectedAndNotBusy);
    ui->pushButtonApplyToPPB1->setEnabled(connectedAndNotBusy);
    ui->pushButtonTestSequence->setEnabled(connectedAndNotBusy);
    ui->radioButtonFUTransmit->setEnabled(connectedAndNotBusy);
    ui->radioButtonFUReceive->setEnabled(connectedAndNotBusy);
    ui->checkBoxAutoPoll->setEnabled(connectedAndNotBusy);

    // === КНОПКА ПОДКЛЮЧЕНИЯ/ОТКЛЮЧЕНИЯ ===
    // Доступна, если:
    // 1. Система не занята (не isBusy)
    // 2. И мы в состоянии Idle (для подключения) или Ready (для отключения)
    bool connectButtonEnabled = !isBusy && (state == PPBState::Idle || state == PPBState::Ready);
    ui->pushButtonConnect->setEnabled(connectButtonEnabled);

    // === ДОПОЛНИТЕЛЬНЫЕ ЭЛЕМЕНТЫ ===
    ui->comboBoxPPBSelect->setEnabled(!isBusy);
    ui->lineEditIPAddress->setEnabled(!isBusy && !isConnected);
    ui->lineEditPort->setEnabled(!isBusy && !isConnected);
    ui->pushButtonClearLog->setEnabled(true);  // Всегда доступна
    ui->pushButtonExportLog->setEnabled(true); // Всегда доступна
    ui->pult_active->setEnabled(!isBusy);      // Доступна, когда система не занята

    LOG_DEBUG("TesterWindow::updateControlsState: завершено");
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
        break;

    case PPBState::Ready:
        if (busy) {
            style = "border-radius: 10px; border: 2px solid #666; background-color: #ffff00;";
            text = "Выполняется операция...";
            tooltip = "Выполняется операция";
        } else {
            style = "border-radius: 10px; border: 2px solid #666; background-color: #00ff00;";
            text = "Отключиться";
            tooltip = "Подключено";
        }
        break;

    case PPBState::SendingCommand:
    case PPBState::WaitingData:
        style = "border-radius: 10px; border: 2px solid #666; background-color: #ffff00;";
        text = "Операция...";
        tooltip = "Выполняется команда";
        break;

    default:
        style = "border-radius: 10px; border: 2px solid #666; background-color: #cccccc;";
        text = "Ожидание...";
        tooltip = "Неизвестное состояние";
        break;
    }

    ui->labelConnectionStatus->setStyleSheet(style);
    ui->labelConnectionStatus->setToolTip(tooltip);
    ui->pushButtonConnect->setText(text);

    LOG_DEBUG(QString("TesterWindow::updateConnectionUI: state=%1, text='%2'")
                  .arg(static_cast<int>(state)).arg(text));
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
    m_logEntries.clear();
    LOG_CAT_INFO("UI", "Лог очищен");
}

void TesterWindow::onExportLogClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт лога", "",
                                                    "Текстовые файлы (*.txt);;HTML файлы (*.html)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_CAT_ERROR("UI", "Не удалось открыть файл для записи: " + fileName);
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream stream(&file);

    if (fileName.endsWith(".html", Qt::CaseInsensitive)) {
        // Экспорт в HTML
        stream << "<!DOCTYPE html>\n";
        stream << "<html><head>\n";
        stream << "<meta charset=\"UTF-8\">\n";
        stream << "<title>Лог ППБ</title>\n";
        stream << "<style>\n";
        stream << ui->textBrowser->document()->defaultStyleSheet();
        stream << "</style>\n";
        stream << "</head><body>\n";

        for (const LogEntry& entry : m_logEntries) {
            stream << formatLogEntryToHtml(entry) << "\n";
        }

        stream << "</body></html>\n";
    } else {
        // Экспорт в простой текст
        stream << "=== Лог ППБ ===\n";
        stream << "Экспортировано: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        stream << "Количество записей: " << m_logEntries.size() << "\n\n";

        for (const LogEntry& entry : m_logEntries) {
            stream << formatLogEntryToPlainText(entry) << "\n";
        }
    }

    file.close();
    LOG_CAT_INFO("UI", QString("Лог экспортирован в файл: %1 (%2 записей)")
                           .arg(fileName).arg(m_logEntries.size()));
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

    // Устанавливаем иконку (если нужно)
     m_pultWindow->setWindowIcon(QIcon("../remote-control.png"));

    // Подключаем сигнал закрытия
    connect(m_pultWindow, &pult::destroyed, this, [this]() {
        LOG_DEBUG("TesterWindow: пульт уничтожен");
        m_pultWindow = nullptr;
    });

    // Показываем пульт
    m_pultWindow->show();

    LOG_CAT_INFO("CONTROLLER","Активирован пульт управления для ППБ " +
                 QString::number(address, 16));
    LOG_CAT_INFO("PULT","Открыт пульт управления");
}

void TesterWindow::onControllerBusyChanged(bool busy)
{
    LOG_CAT_DEBUG("TesterWindow"," состояние занятости изменилось: " +
              QString(busy ? "занят" : "свободен"));

    // Обновляем состояние элементов управления
    updateControlsState();
    updateConnectionUI();

    // Обновляем заголовок окна
    updateWindowTitle();
}

