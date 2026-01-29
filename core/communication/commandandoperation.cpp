#include "commandandoperation.h"
#include "../../analyzer/testsequencecomparator.h"
#include "../logger.h"
#include <QDataStream>
#include <QtEndian>

// Вспомогательная функция для преобразования QByteArray в uint32_t
static uint32_t bytesToUInt32LE(const QByteArray& data) {
    if (data.size() < 4) return 0;
    uint32_t result;
    memcpy(&result, data.constData(), 4);
    return qFromLittleEndian(result); // или qFromBigEndian - зависит от протокола
}

// Функция для парсинга 2 пакетов в uint32_t (для команд, где 2 пакета = 4 байта данных)
static uint32_t parseTwoPackets(const QVector<QByteArray>& data) {
    if (data.size() < 2) return 0;

    // Извлекаем данные из DataPacket (первые 2 байта - данные, остальное - служебные)
    QByteArray combined;
    for (const QByteArray& packet : data) {
        if (packet.size() >= 2) {
            combined.append(packet.left(2)); // Берем только data[0] и data[1]
        }
    }

    return bytesToUInt32LE(combined);
}

// Реализация метода create
std::unique_ptr<PPBCommand> CommandFactory::create(TechCommand cmd) {
    switch (cmd) {
    case TechCommand::TS: return std::make_unique<StatusCommand>();
    case TechCommand::TC: return std::make_unique<ResetCommand>();
    case TechCommand::VERS: return std::make_unique<VersCommand>();
    case TechCommand::VOLUME: return std::make_unique<VolumeCommand>();
    case TechCommand::CHECKSUM: return std::make_unique<CheckSumCommand>();
    case TechCommand::PROGRAMM: return std::make_unique<ProgrammCommand>();
    case TechCommand::CLEAN: return std::make_unique<CleanCommand>();
    case TechCommand::DROP: return std::make_unique<DROPCommand>();
    case TechCommand::PRBS_M2S: return std::make_unique<PRBS_M2SCommand>();
    case TechCommand::PRBS_S2M: return std::make_unique<PRBS_S2MCommand>();
    case TechCommand::BER_T: return std::make_unique<BER_TCommand>();
    case TechCommand::BER_F: return std::make_unique<BER_FCommand>();
    default: return nullptr;
    }
}

QString CommandFactory::commandName(TechCommand cmd) {
    static const QMap<TechCommand, QString> names = {
                                                     {TechCommand::TS, "Опрос состояния"},
                                                     {TechCommand::TC, "Сброс"},
                                                     {TechCommand::VERS, "Запрос версии"},
                                                     {TechCommand::VOLUME, "Принять том исполняемого ПО"},
                                                     {TechCommand::CHECKSUM, "Выдать контрольную сумму"},
                                                     {TechCommand::PROGRAMM, "Обновить исполняемый файл ПО"},
                                                     {TechCommand::CLEAN, "Очистить временный файл ПО"},
                                                     {TechCommand::DROP, "Отброшенные пакеты ФУ"},
                                                     {TechCommand::PRBS_M2S, "Принять тестовую последовательность данных"},
                                                     {TechCommand::PRBS_S2M, "Выдать тестовую последовательность"},
                                                     {TechCommand::BER_T, "Коэффициент ошибок линии ТУ"},
                                                     {TechCommand::BER_F, "Коэффициент ошибок линии ФУ"},
                                                     };
    return names.value(cmd, "Неизвестная команда");
}

// TS
void StatusCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("StatusCommand::onDataReceived: comm is nullptr!");
        return;
    }
    emit comm->statusDataReady(data);
    comm->completeCurrentOperation(true, "Статус получен");
}

// VERS
void VersCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("VersCommand::onDataReceived: comm is nullptr!");
        return;
    }

    // Парсим 2 пакета = 4 байта CRC32
    uint32_t crc = parseTwoPackets(data);

    // Формируем сообщение для UI
    QString message = QString("Версия ПО. CRC32: 0x%1").arg(crc, 8, 16, QChar('0'));

    // Отправляем сигнал с сырыми данными для дальнейшего парсинга
    emit comm->statusDataReady(data);

    // Завершаем операцию
    comm->completeCurrentOperation(true, message);
}

// CHECKSUM
void CheckSumCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("CheckSumCommand::onDataReceived: comm is nullptr!");
        return;
    }

    uint32_t checksum = parseTwoPackets(data);
    QString message = QString("Контрольная сумма VOLUME: 0x%1").arg(checksum, 8, 16, QChar('0'));
    comm->completeCurrentOperation(true, message);
}

// DROP
void DROPCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("DROPCommand::onDataReceived: comm is nullptr!");
        return;
    }

    uint32_t dropped = parseTwoPackets(data);
    QString message = QString("Отброшенных пакетов ФУ: %1").arg(dropped);
    comm->completeCurrentOperation(true, message);
}

// BER_T
void BER_TCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("BER_TCommand::onDataReceived: comm is nullptr!");
        return;
    }

    uint32_t errors = parseTwoPackets(data);
    float ber = errors / 1000000.0f; // Пример расчета BER
    QString message = QString("Коэффициент ошибок ТУ: %1 (ошибок: %2)").arg(ber, 0, 'f', 6).arg(errors);
    comm->completeCurrentOperation(true, message);
}

// BER_F
void BER_FCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    if (!comm) {
        LOG_WARNING("BER_FCommand::onDataReceived: comm is nullptr!");
        return;
    }

    uint32_t errors = parseTwoPackets(data);
    float ber = errors / 1000000.0f;
    QString message = QString("Коэффициент ошибок ФУ: %1 (ошибок: %2)").arg(ber, 0, 'f', 6).arg(errors);
    comm->completeCurrentOperation(true, message);
}

// PRBS_M2S
void PRBS_M2SCommand::onOkReceived(CommandInterface* comm, uint16_t address) const {
    if (!comm) {
        LOG_WARNING("StatusCommand::onDataReceived: comm is nullptr!");
        return;
    }
    // Генерируем тестовые пакеты прямо здесь
    QVector<DataPacket> testPackets;

    // Генерация 256 пакетов по протоколу
    uint8_t lfsr = 0x01; // Начальное значение

    for (int i = 0; i < PPBConstants::TEST_PACKET_COUNT; ++i) {
        DataPacket packet;
        packet.data[0] = lfsr;
        packet.data[1] = lfsr ^ 0x55; // XOR для разнообразия
        packet.counter = i; // Номер пакета

        // CRC от 3 байт
        uint8_t crcData[3] = {packet.data[0], packet.data[1], packet.counter};
        packet.crc = calculateCRC8(crcData, 3);

        testPackets.append(packet);

        // LFSR сдвиг
        lfsr = (lfsr >> 1) | ((lfsr ^ (lfsr >> 1)) << 7);
    }

    // Отправляем пакеты
    comm->sendDataPackets(testPackets);
}

// PRBS_S2M
void PRBS_S2MCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const
{
    if (!comm) {
        LOG_WARNING("StatusCommand::onDataReceived: comm is nullptr!");
        return;
    }
    // Парсим полученные пакеты
    QVector<DataPacket> receivedPackets;
    for (const auto& packetData : data) {
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(packetData, packet)) {
            receivedPackets.append(packet);
        }
    }

    // Получаем отправленные пакеты (из PRBS_M2S)
    QVector<DataPacket> sentPackets = comm->getGeneratedPackets();

    if (sentPackets.isEmpty()) {
        comm->completeCurrentOperation(false,
                                       "Нет данных для сравнения. Сначала выполните PRBS_M2S.");
        return;
    }

    // Сравниваем
    TestSequenceComparator::Result result =
        TestSequenceComparator::compare(sentPackets, receivedPackets);

    // Формируем отчёт
    QString report = TestSequenceComparator::generateReport(result);

    // Завершаем операцию
    comm->completeCurrentOperation(result.isPerfectMatch, report);
}

// VOLUME
void VolumeCommand::onOkReceived(CommandInterface* comm, uint16_t address) const {
    if (!comm) {
        LOG_WARNING("VolumeCommand::onOkReceived: comm is nullptr!");
        return;
    }

    // После получения OK нужно начать отправку ПО
    // В реальности здесь нужно загрузить файл ПО и разбить на 256 пакетов
    QVector<DataPacket> programPackets;

    // TODO: Загрузить реальный файл ПО
    // Пока заглушка - генерируем тестовые данные
    for (int i = 0; i < 256; ++i) {
        DataPacket packet;
        packet.data[0] = i & 0xFF;
        packet.data[1] = (i >> 8) & 0xFF;
        packet.counter = i;

        // CRC
        uint8_t crcData[3] = {packet.data[0], packet.data[1], packet.counter};
        packet.crc = calculateCRC8(crcData, 3);

        programPackets.append(packet);
    }

    // Отправляем пакеты ПО
    comm->sendDataPackets(programPackets);
    comm->completeCurrentOperation(true, "Начата отправка ПО (256 пакетов)");
}
