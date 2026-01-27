#include "commandandoperation.h"
#include "../analyzer/testsequencecomparator.h"
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
    emit comm->statusDataReady(data);
    comm->completeCurrentOperation(true, "Статус получен");
}

// VERS
void VersCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    // TODO: обработка данных версии
    comm->completeCurrentOperation(true, "Версия получена");
}

// CHECKSUM
void CheckSumCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    // TODO: обработка контрольной суммы
    comm->completeCurrentOperation(true, "Контрольная сумма получена");
}

// DROP
void DROPCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    // TODO: обработка отброшенных пакетов
    comm->completeCurrentOperation(true, "Данные об отброшенных пакетах получены");
}

// BER_T
void BER_TCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    // TODO: обработка коэффициента ошибок
    comm->completeCurrentOperation(true, "Коэффициент ошибок ТУ получен");
}

// BER_F
void BER_FCommand::onDataReceived(CommandInterface* comm, const QVector<QByteArray>& data) const {
    // TODO: обработка коэффициента ошибок
    comm->completeCurrentOperation(true, "Коэффициент ошибок ФУ получен");
}

// PRBS_M2S
void PRBS_M2SCommand::onOkReceived(CommandInterface* comm, uint16_t address) const {
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
    // После OK нужно отправлять данные ПО
    comm->completeCurrentOperation(true, "Начало приема ПО");
}
