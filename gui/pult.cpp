#include "pult.h"
#include "ui_pult.h"
#include "../core/logger.h"

pult::pult(uint16_t address, PPBController* controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pult)
    , m_controller(controller)
    , m_address(address)
{
    ui->setupUi(this);
    setWindowTitle(QString("Пульт управления ППБ (адрес: %1)").arg(address));

    if (!m_controller) {
        Logger::error("Пульт: передан нулевой контроллер");
    }
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
