#include "pult.h"
#include "ui_pult.h"
#include "../core/logger.h"
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
        Logger::error("Пульт: передан нулевой контроллер");
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

    Logger::info("Пульт инициализирован для адреса " + QString::number(address));
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
    Logger::debug("Pult log: " + message);

    // Можно добавить вывод в отдельный виджет, если нужно
    // Например, в QTextEdit или QListWidget
}

void pult::onControllerErrorOccurred(const QString& error)
{
    // Показываем ошибку пользователю
    QMessageBox::warning(this, "Ошибка", error);
    Logger::error("Pult error: " + error);

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
        Logger::info("Pult operation: " + message);
    } else {
        ui->statusbar->setText("✗ " + message);
        ui->statusbar->setStyleSheet("color: orange; font-weight: bold;");
        Logger::warning("Pult operation failed: " + message);
    }

    // Очищаем статус через 3 секунды
    if (m_statusTimer) {
        m_statusTimer->start(3000);
    }
}
