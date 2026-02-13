#include "pult.h"
#include "ui_pult.h"
#include "../core/logging/logging_unified.h"
#include <QTimer>
pult::pult(uint16_t address, PPBController* controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pult)
    , m_controller(controller)
    , m_address(address)
    , m_statusTimer(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(QString("Пульт управления ППБ (адрес: %1)").arg(address));

    if (!m_controller) {
        LOG_UI_ALERT("Пульт: передан нулевой контроллер");
                return;
    }

    // Инициализируем таймер для очистки статуса
    m_statusTimer = new QTimer(this);
    m_statusTimer->setSingleShot(true);
    connect(m_statusTimer, &QTimer::timeout, this, [this]() {
        ui->statusbar->clear();
    });

    // Подключаем сигналы для обратной связи
   /* connect(m_controller, &PPBController::logMessage,
            this, &pult::onControllerLogMessage); */
    connect(m_controller, &PPBController::errorOccurred,
            this, &pult::onControllerErrorOccurred);
    connect(m_controller, &PPBController::operationCompleted,
            this, &pult::onControllerOperationCompleted);


    // Подключаем сигналы анализа
    connect(m_controller, &PPBController::analysisStarted,
            this, &pult::onAnalysisStarted);
    connect(m_controller, &PPBController::analysisProgress,
            this, &pult::onAnalysisProgress);
    connect(m_controller, &PPBController::analysisComplete,
            this, &pult::onAnalysisComplete);
    LOG_UI("Пульт инициализирован для адреса " + QString::number(address));
}

pult::~pult()
{
    delete ui;
}

// ==================== КОМАНДЫ ====================

void pult::on_TSComand_clicked()
{
    if (m_controller) {
        m_controller->requestStatus(m_address);
    }
}

void pult::on_TCCommand_clicked()
{
    if (m_controller) {
        m_controller->resetPPB(m_address);
    }
}

void pult::on_PRBS_S2MCommand_clicked()
{
    if (m_controller) {
        m_controller->startPRBS_S2M(m_address);
    }
}

void pult::on_PRBS_M2SCommand_clicked()
{
    if (m_controller) {
        m_controller->startPRBS_M2S(m_address);
    }
}

void pult::on_VERSComand_clicked()
{
    if (m_controller) {
        m_controller->requestVersion(m_address);
    }
}

void pult::on_VolumeComand_clicked()
{
    if (m_controller) {
        m_controller->requestVolume(m_address);
    }
}

void pult::on_ChecksumCommand_clicked()
{
    if (m_controller) {
        m_controller->requestChecksum(m_address);
    }
}

void pult::on_ProgramCommand_clicked()
{
    if (m_controller) {
        m_controller->sendProgram(m_address);
    }
}

void pult::on_CleanCommand_clicked()
{
    if (m_controller) {
        m_controller->sendClean(m_address);
    }
}

void pult::on_DropCommand_clicked()
{
    if (m_controller) {
        m_controller->requestDroppedPackets(m_address);
    }
}

void pult::on_BER_TCommand_clicked()
{
    if (m_controller) {
        m_controller->requestBER_T(m_address);
    }
}

void pult::on_BER_FCommand_clicked()
{
    if (m_controller) {
        m_controller->requestBER_F(m_address);
    }
}

void pult::onControllerLogMessage(const QString& message)
{
    // Логируем сообщение
    LOG_UI_ALERT("Pult log: " + message);

    // Можно добавить вывод в отдельный виджет, если нужно
    // Например, в QTextEdit или QListWidget
}

void pult::onControllerErrorOccurred(const QString& error)
{
    // Показываем ошибку пользователю
    QMessageBox::warning(this, "Ошибка", error);
    LOG_UI("Pult error: " + error);

    // Обновляем статусную метку
    ui->statusbar->setText("Ошибка: " + error);
    ui->statusbar->setStyleSheet("color: red; font-weight: bold;");

    // Очищаем статус через 5 секунд
    if (m_statusTimer) {
        m_statusTimer->start(5000);
    }
}

void pult::onControllerOperationCompleted(bool success, const QString& message)
{
    // Обновляем статус в UI
    if (success) {
        ui->statusbar->setText("✓ " + message);
        ui->statusbar->setStyleSheet("color: green; font-weight: bold;");
        LOG_UI("Pult operation: " + message);
    } else {
        ui->statusbar->setText("✗ " + message);
        ui->statusbar->setStyleSheet("color: orange; font-weight: bold;");
       LOG_UI("Pult operation failed: " + message);
    }

    // Очищаем статус через 3 секунды
    if (m_statusTimer) {
        m_statusTimer->start(3000);
    }
}



void pult::on_AnalizeBttn_clicked()
{
    if (m_controller) {
        m_controller->analize();
    }
}

void pult::onAnalysisStarted() {
    ui->statusbar->setText("📊 Анализ начат...");
    ui->statusbar->setStyleSheet("color: blue; font-weight: bold;");
}

void pult::onAnalysisProgress(int percent) {
    ui->statusbar->setText(QString("📊 Анализ: %1%").arg(percent));
}

void pult::onAnalysisComplete(const QString& summary, const QVariantMap& details) {
    ui->statusbar->setText("✅ Анализ завершен");
    ui->statusbar->setStyleSheet("color: green; font-weight: bold;");

    // Можно показать диалог с результатами
    QMessageBox::information(this, "Результаты анализа", summary);

    // Очищаем статус через 5 секунд
    if (m_statusTimer) {
        m_statusTimer->start(5000);
    }
}
