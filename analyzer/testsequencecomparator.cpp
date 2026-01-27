// testsequencecomparator.cpp
#include "testsequencecomparator.h"
#include <QStringList>
#include "../core/packetbuilder.h"
#include <QSet>
TestSequenceComparator::Result TestSequenceComparator::compare(
    const QVector<DataPacket>& expected,
    const QVector<DataPacket>& actual)
{
    Result result;
    result.totalPackets = expected.size();
    result.isPerfectMatch = true;

    // Создаем карту ожидаемых пакетов по индексу
    QMap<uint8_t, DataPacket> expectedMap;
    for (const auto& packet : expected) {
        expectedMap[packet.counter] = packet;
    }

    // Проверяем полученные пакеты
    QSet<uint8_t> receivedIndices;
    for (const auto& packet : actual) {
        uint8_t index = packet.counter;
        receivedIndices.insert(index);

        // Проверяем, есть ли такой индекс в ожидаемых
        if (!expectedMap.contains(index)) {
            result.errors++;
            result.detailedErrors[index] =
                QString("Неожиданный индекс пакета: %1").arg(index);
            result.isPerfectMatch = false;
            continue;
        }

        // Сравниваем данные
        const DataPacket& expectedPacket = expectedMap[index];
        if (memcmp(packet.data, expectedPacket.data, 2) != 0) {
            result.errors++;
            result.detailedErrors[index] =
                QString("Данные не совпадают. Ожидалось: [0x%1 0x%2], получено: [0x%3 0x%4]")
                    .arg(expectedPacket.data[0], 2, 16, QChar('0'))
                    .arg(expectedPacket.data[1], 2, 16, QChar('0'))
                    .arg(packet.data[0], 2, 16, QChar('0'))
                    .arg(packet.data[1], 2, 16, QChar('0'));
            result.isPerfectMatch = false;
        }

        // Проверяем CRC
        if (!PacketBuilder::checkDataPacketCRC(packet)) {
            result.errors++;
            result.detailedErrors[index] =
                QString("Ошибка CRC в пакете %1").arg(index);
            result.isPerfectMatch = false;
        }
    }

    // Проверяем пропущенные пакеты
    for (uint8_t i = 0; i < expected.size(); ++i) {
        if (!receivedIndices.contains(i)) {
            result.missingPackets++;
            result.detailedErrors[i] =
                QString("Пакет %1 не получен").arg(i);
            result.isPerfectMatch = false;
        }
    }

    // Проверяем порядок
    uint8_t lastIndex = 0;
    for (const auto& packet : actual) {
        if (packet.counter < lastIndex) {
            result.outOfOrderPackets++;
            result.detailedErrors[packet.counter] =
                QString("Нарушен порядок: получен %1 после %2")
                    .arg(packet.counter).arg(lastIndex);
            result.isPerfectMatch = false;
        }
        lastIndex = packet.counter;
    }

    return result;
}

QString TestSequenceComparator::generateReport(const Result& result)
{
    QStringList report;

    report << QString("=== Результат сравнения тестовой последовательности ===");
    report << QString("Всего ожидалось пакетов: %1").arg(result.totalPackets);
    report << QString("Принято пакетов: %1").arg(result.totalPackets - result.missingPackets);
    report << QString("Пропущено пакетов: %1").arg(result.missingPackets);
    report << QString("Ошибок данных/CRC: %1").arg(result.errors);
    report << QString("Нарушений порядка: %1").arg(result.outOfOrderPackets);
    report << QString("Состояние: %1").arg(result.isPerfectMatch ? "ИДЕАЛЬНО" : "ЕСТЬ ОШИБКИ");

    if (!result.detailedErrors.isEmpty()) {
        report << "";
        report << "Детали ошибок:";
        QList<int> errorIndices = result.detailedErrors.keys();
        std::sort(errorIndices.begin(), errorIndices.end());

        for (int index : errorIndices) {
            report << QString("  Пакет %1: %2").arg(index).arg(result.detailedErrors[index]);
        }
    }

    return report.join("\n");
}
