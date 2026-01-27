#include "testsequenceanalyzer.h"
#include "guerrillaanalyzer.h"
#include "shiftdetector.h"
#include "intelligentanalyzer.h"
#include "../core/logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QHash>
#include <QSet>

TestSequenceAnalyzer::TestResult
TestSequenceAnalyzer::analyzeTest(const QVector<DataPacket>& sentPackets,
                                  const QVector<DataPacket>& receivedPackets)
{
    TestResult result;
    result.packetsSent = sentPackets.size();
    result.packetsReceived = receivedPackets.size();

    if (sentPackets.isEmpty() || receivedPackets.isEmpty()) {
        result.report = "Нет данных для анализа (один из массивов пуст)";
        result.success = false;
        return result;
    }

    // 1. Используем GuerrillaAnalyzer для детального анализа соответствия
    GuerrillaAnalyzer guerrillaAnalyzer;
    auto guerrillaResult = guerrillaAnalyzer.analyze(sentPackets, receivedPackets);

    // 2. Используем ShiftDetector для анализа сдвига
    ShiftDetector shiftDetector;
    auto shiftResult = shiftDetector.analyze(sentPackets, receivedPackets);

    // 3. Используем IntelligentAnalyzer для комплексного анализа
    IntelligentAnalyzer intelligentAnalyzer;
    auto intelligentResult = intelligentAnalyzer.analyze(sentPackets, receivedPackets);

    // 4. Формируем итоговый отчет
    QString report;
    report += "=== ОТЧЕТ О ТЕСТОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ ===\n\n";

    // 4.1. Основная статистика
    report += "1. ОСНОВНАЯ СТАТИСТИКА:\n";
    report += QString("   Отправлено: %1 пакетов\n").arg(result.packetsSent);
    report += QString("   Получено: %1 пакетов\n").arg(result.packetsReceived);
    report += QString("   Совпало: %1 пакетов (%2%)\n")
                  .arg(guerrillaResult.matched)
                  .arg(result.packetsSent > 0 ?
                           (guerrillaResult.matched * 100.0 / result.packetsSent) : 0, 0, 'f', 1);

    // 4.2. Анализ сдвига
    report += "\n2. АНАЛИЗ СДВИГА:\n";
    report += "   " + shiftResult.analysis + "\n";
    result.detectedShift = shiftResult.shift;

    // 4.3. Детальный анализ соответствия
    report += "\n3. ДЕТАЛЬНЫЙ АНАЛИЗ СООТВЕТСТВИЯ:\n";
    report += guerrillaAnalyzer.generateReport(guerrillaResult, sentPackets, receivedPackets);

    // 4.4. Интеллектуальный анализ
    report += "\n4. ИНТЕЛЛЕКТУАЛЬНЫЙ АНАЛИЗ:\n";
    report += intelligentResult.diagnosis;

    // 4.5. Таблица соответствия
    if (!guerrillaResult.receivedToSent.isEmpty()) {
        report += "\n5. ТАБЛИЦА СООТВЕТСТВИЯ (первые 50):\n";
        report += "   Полученный → Отправленный\n";

        QList<int> recvIndices = guerrillaResult.receivedToSent.keys();
        std::sort(recvIndices.begin(), recvIndices.end());

        int count = 0;
        for (int recvIndex : recvIndices) {
            int sentIndex = guerrillaResult.receivedToSent[recvIndex];
            QString marker = (sentIndex == recvIndex) ? "✓" : "✗";
            report += QString("   %1 → %2 %3\n")
                          .arg(recvIndex, 4)
                          .arg(sentIndex, 4)
                          .arg(marker);

            if (++count >= 50) {
                report += QString("   ... и еще %1 пар\n").arg(guerrillaResult.receivedToSent.size() - 50);
                break;
            }
        }
    }

    // 4.6. Потерянные пакеты (детально)
    if (!guerrillaResult.lostIndices.isEmpty()) {
        report += "\n6. ПОТЕРЯННЫЕ ПАКЕТЫ (первые 20):\n";
        report += "   Индекс | Данные [3 байта]\n";
        report += "   -------|-----------------\n";

        for (int i = 0; i < qMin(guerrillaResult.lostIndices.size(), 20); i++) {
            int idx = guerrillaResult.lostIndices[i];
            if (idx < sentPackets.size()) {
                const DataPacket& packet = sentPackets[idx];
                report += QString("   %1 | [%2 %3 %4]\n")
                              .arg(idx, 4)
                              .arg(packet.data[0], 2, 16, QChar('0'))
                              .arg(packet.data[1], 2, 16, QChar('0'))
                              .arg(packet.data[2], 2, 16, QChar('0'));
            }
        }

        if (guerrillaResult.lostIndices.size() > 20) {
            report += QString("   ... и еще %1 пакетов\n").arg(guerrillaResult.lostIndices.size() - 20);
        }
    }

    // 4.7. Неизвестные пакеты (детально)
    if (!guerrillaResult.unknownIndices.isEmpty()) {
        report += "\n7. НЕИЗВЕСТНЫЕ ПАКЕТЫ (первые 20):\n";
        report += "   Индекс | Данные [3 байта]\n";
        report += "   -------|-----------------\n";

        for (int i = 0; i < qMin(guerrillaResult.unknownIndices.size(), 20); i++) {
            int idx = guerrillaResult.unknownIndices[i];
            if (idx < receivedPackets.size()) {
                const DataPacket& packet = receivedPackets[idx];
                report += QString("   %1 | [%2 %3 %4]\n")
                              .arg(idx, 4)
                              .arg(packet.data[0], 2, 16, QChar('0'))
                              .arg(packet.data[1], 2, 16, QChar('0'))
                              .arg(packet.data[2], 2, 16, QChar('0'));
            }
        }

        if (guerrillaResult.unknownIndices.size() > 20) {
            report += QString("   ... и еще %1 пакетов\n").arg(guerrillaResult.unknownIndices.size() - 20);
        }
    }

    // 5. Сохраняем результат
    result.report = report;
    result.packetsLost = guerrillaResult.lost;
    result.packetsCorrupted = 0; // Можно добавить подсчет пакетов с ошибкой CRC

    // Рассчитываем процент потерь
    if (result.packetsSent > 0) {
        result.lossPercentage = (result.packetsLost * 100.0) / result.packetsSent;
    }

    // Определяем успешность теста
    // Допускаем до 5% потерь и до 10% пакетов в неправильном порядке
    bool acceptableLosses = result.lossPercentage <= 5.0;
    bool acceptableCorruption = (guerrillaResult.unknown <= result.packetsSent * 0.1) &&
                                (guerrillaResult.duplicates <= result.packetsSent * 0.1);

    result.success = acceptableLosses && acceptableCorruption;

    // 6. Сохраняем детальные таблицы в файл
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("test_analysis_%1.txt").arg(timestamp);
    saveTablesToFile(sentPackets, receivedPackets, filename);

    // 7. Логируем результат
    Logger::info(QString("Анализ тестовой последовательности завершен. "
                         "Отправлено: %1, Получено: %2, Совпало: %3")
                     .arg(result.packetsSent)
                     .arg(result.packetsReceived)
                     .arg(guerrillaResult.matched));

    if (result.success) {
        Logger::info("Тест пройден успешно");
    } else {
        Logger::warning("Тест пройден с ошибками");
    }

    return result;
}

void TestSequenceAnalyzer::saveTablesToFile(const QVector<DataPacket>& sent,
                                            const QVector<DataPacket>& received,
                                            const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error(QString("Не удалось открыть файл для записи таблиц: %1").arg(filename));
        return;
    }

    QTextStream stream(&file);

    // Заголовок
    stream << "СВОДНЫЕ ТАБЛИЦЫ ТЕСТОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ\n";
    stream << "============================================\n\n";

    stream << "Дата и время: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";

    // Таблица отправленных пакетов
    stream << "ОТПРАВЛЕННЫЕ ПАКЕТЫ (всего " << sent.size() << "):\n";
    stream << "Индекс | Данные [3 байта] | CRC\n";
    stream << "-------|-------------------|-----\n";

    for (int i = 0; i < sent.size(); i++) {
        const DataPacket& packet = sent[i];
        stream << QString("%1 | [%2 %3 %4] | %5\n")
                      .arg(i, 6)
                      .arg(packet.data[0], 2, 16, QChar('0'))
                      .arg(packet.data[1], 2, 16, QChar('0'))
                      .arg(packet.data[2], 2, 16, QChar('0'))
                      .arg(packet.crc, 2, 16, QChar('0'));
    }

    stream << "\n\n";

    // Таблица полученных пакетов
    stream << "ПОЛУЧЕННЫЕ ПАКЕТЫ (всего " << received.size() << "):\n";
    stream << "Индекс | Данные [3 байта] | CRC | Статус CRC\n";
    stream << "-------|-------------------|-----|-----------\n";

    for (int i = 0; i < received.size(); i++) {
        const DataPacket& packet = received[i];
        bool crcOk = PacketBuilder::checkDataPacketCRC(packet);

        stream << QString("%1 | [%2 %3 %4] | %5 | %6\n")
                      .arg(i, 6)
                      .arg(packet.data[0], 2, 16, QChar('0'))
                      .arg(packet.data[1], 2, 16, QChar('0'))
                      .arg(packet.data[2], 2, 16, QChar('0'))
                      .arg(packet.crc, 2, 16, QChar('0'))
                      .arg(crcOk ? "OK" : "ERROR");
    }

    file.close();
    Logger::info(QString("Таблицы сохранены в файл: %1").arg(filename));
}

QString TestSequenceAnalyzer::hashPacket(const DataPacket& packet)
{
    // Хеш по 3 байтам данных
    return QString("%1-%2-%3")
        .arg(packet.data[0], 2, 16, QChar('0'))
        .arg(packet.data[1], 2, 16, QChar('0'))
        .arg(packet.data[2], 2, 16, QChar('0'))
        .toUpper();
}
