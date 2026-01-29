#ifndef COMMANDANDOPERATION_H
#define COMMANDANDOPERATION_H

#include <QObject>
#include <QMap>
#include <memory>
#include "ppbprotocol.h"
#include "packetbuilder.h"
#include "commandinterface.h"
#include <QTimer>

namespace PPBConstants {
constexpr int OPERATION_TIMEOUT_MS = 5000;    // Таймаут операции 5 сек
constexpr int PACKET_TIMEOUT_MS = 1000;       // Таймаут между пакетами 1 сек
constexpr int TEST_PACKET_COUNT = 256;        // 256 тестовых пакетов
constexpr int PACKET_INTERVAL_MS = 100;       // Интервал 10 Гц = 100 мс
constexpr int BER_RESPONSE = 2;               // 2 пакета ответа на БЕР_Т/Ф
constexpr int STATUS_RESPONSE =9;             // 9 пакетов статуса
constexpr int VERS_RESPONSE =2;               //2 пакеты версии
constexpr int CHECKSUM_RESPONSE=2;            //2 пакета контр суммы
}

// Базовый класс для всех команд
class PPBCommand {

public:
    virtual ~PPBCommand() = default;

    // Уникальный ID команды
    virtual TechCommand commandId() const = 0;

    // Человекочитаемое имя
    virtual QString name() const = 0;

    // Создать пакет запроса
    virtual QByteArray buildRequest(uint16_t address) const = 0;

    // Ожидаемое количество пакетов ответа
    virtual int expectedResponsePackets() const = 0;

    // Таймаут операции (мс)
    virtual int timeoutMs() const = 0;

    virtual void onOkReceived(CommandInterface* comm, uint16_t address) const
    {
        // По умолчанию: ждать данные если expectedResponsePackets > 0
        if (expectedResponsePackets() > 0) {
            comm->setState(PPBState::WaitingData);
            comm->startTimeoutTimer(timeoutMs());
        } else {
            // Иначе завершаем операцию
            comm->completeCurrentOperation(true, "Команда выполнена");
        }
    }

    virtual void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {comm->completeCurrentOperation(true, QString("Получено %1 пакетов").arg(data.size())); }


    virtual void onTimeout(CommandInterface* comm) const  {comm->completeCurrentOperation(false, "Таймаут операции"); }
};

// Базовый шаблонный класс конкретной команды
template<TechCommand CmdId, int ExpectedPackets = 0, int Timeout = 3000>
class ConcretePPBCommand : public PPBCommand {
public:
    TechCommand commandId() const override { return CmdId; }
    int expectedResponsePackets() const override { return ExpectedPackets; }
    int timeoutMs() const override { return Timeout; }

    QByteArray buildRequest(uint16_t address) const override {
        return PacketBuilder::createTURequest(address, CmdId);
    }

    QString name() const override;
};

// TS команда с переопределенным onDataReceived
class StatusCommand : public ConcretePPBCommand<TechCommand::TS, PPBConstants::STATUS_RESPONSE> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// TC команда (использует реализацию по умолчанию)
using ResetCommand = ConcretePPBCommand<TechCommand::TC, 0>;

// VERS команда с переопределенным onDataReceived
class VersCommand : public ConcretePPBCommand<TechCommand::VERS, PPBConstants::VERS_RESPONSE> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// VOLUME команда с переопределенным onOkReceived
class VolumeCommand : public ConcretePPBCommand<TechCommand::VOLUME, 0> {
public:
    void onOkReceived(CommandInterface* comm, uint16_t address) const override;
};

// CHECKSUM команда с переопределенным onDataReceived
class CheckSumCommand : public ConcretePPBCommand<TechCommand::CHECKSUM, PPBConstants::CHECKSUM_RESPONSE> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// Остальные команды (используют реализацию по умолчанию)
using ProgrammCommand = ConcretePPBCommand<TechCommand::PROGRAMM, 0>;
using CleanCommand = ConcretePPBCommand<TechCommand::CLEAN, 0>;

// DROP команда с переопределенным onDataReceived
class DROPCommand : public ConcretePPBCommand<TechCommand::DROP, 0> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// PRBS_M2S команда с переопределенным onOkReceived
class PRBS_M2SCommand : public ConcretePPBCommand<TechCommand::PRBS_M2S, PPBConstants::TEST_PACKET_COUNT> {
public:
    void onOkReceived(CommandInterface* comm, uint16_t address) const override;
};

// PRBS_S2M команда с переопределенным onDataReceived
class PRBS_S2MCommand : public ConcretePPBCommand<TechCommand::PRBS_S2M, PPBConstants::TEST_PACKET_COUNT> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// BER_T команда с переопределенным onDataReceived
class BER_TCommand : public ConcretePPBCommand<TechCommand::BER_T, PPBConstants::BER_RESPONSE> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

// BER_F команда с переопределенным onDataReceived
class BER_FCommand : public ConcretePPBCommand<TechCommand::BER_F, PPBConstants::BER_RESPONSE> {
public:
    void onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const override;
};

class CommandFactory {
public:
    static std::unique_ptr<PPBCommand> create(TechCommand cmd);
    static QString commandName(TechCommand cmd);
};

// Определяем метод name() для ConcretePPBCommand
template<TechCommand CmdId, int ExpectedPackets, int Timeout>
QString ConcretePPBCommand<CmdId, ExpectedPackets, Timeout>::name() const {
    return CommandFactory::commandName(CmdId);
}

#endif // COMMANDANDOPERATION_H
