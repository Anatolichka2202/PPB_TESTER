#include "guerrillaanalyzer.h"
#include <cmath>
#include <climits>
#include <algorithm>
#include <QtGlobal>
#include <QHash>
#include <QSet>
#include "../core/logger.h"

GuerrillaAnalyzer::AnalysisResult GuerrillaAnalyzer::analyze(
    const QVector<DataPacket>& sent,
    const QVector<DataPacket>& received,
    int searchWindow)
{
    AnalysisResult result;
    result.totalSent = sent.size();
    result.totalReceived = received.size();

    if (sent.isEmpty() || received.isEmpty()) {
        return result;
    }

    // Шаг 1: Построение хеш-таблиц
    QHash<QString, QList<int>> sentHash; // хеш -> список индексов (на случай одинаковых пакетов)
    QHash<QString, QList<int>> receivedHash;

    for (int i = 0; i < sent.size(); i++) {
        sentHash[hashPacket(sent[i])].append(i);
    }

    for (int i = 0; i < received.size(); i++) {
        receivedHash[hashPacket(received[i])].append(i);
    }

    // Шаг 2: Сопоставление пакетов
    QVector<bool> sentMatched(sent.size(), false);
    QVector<bool> receivedMatched(received.size(), false);

    // Сначала сопоставляем уникальные пакеты
    for (auto recvIt = receivedHash.begin(); recvIt != receivedHash.end(); ++recvIt) {
        QString hash = recvIt.key();
        QList<int> recvIndices = recvIt.value();

        if (sentHash.contains(hash)) {
            QList<int> sentIndices = sentHash[hash];

            // Сопоставляем пока есть пары
            int pairsToMatch = qMin(recvIndices.size(), sentIndices.size());
            for (int i = 0; i < pairsToMatch; i++) {
                int recvIndex = recvIndices[i];
                int sentIndex = sentIndices[i];

                result.receivedToSent[recvIndex] = sentIndex;
                receivedMatched[recvIndex] = true;
                sentMatched[sentIndex] = true;
                result.matched++;

                // Проверяем порядок
                if (sentIndex == recvIndex) {
                    result.inOrder++;
                } else {
                    result.outOfOrder++;
                }
            }

            // Если остались лишние полученные пакеты с тем же хешем - это дубликаты
            if (recvIndices.size() > sentIndices.size()) {
                result.duplicates += recvIndices.size() - sentIndices.size();
                for (int i = pairsToMatch; i < recvIndices.size(); i++) {
                    result.duplicateIndices.append(recvIndices[i]);
                }
            }
        }
    }

    // Шаг 3: Подсчет потерянных пакетов
    for (int i = 0; i < sentMatched.size(); i++) {
        if (!sentMatched[i]) {
            result.lostIndices.append(i);
            result.lost++;
        }
    }

    // Шаг 4: Подсчет неизвестных пакетов
    for (int i = 0; i < receivedMatched.size(); i++) {
        if (!receivedMatched[i]) {
            result.unknownIndices.append(i);
            result.unknown++;
        }
    }

    return result;
}

QString GuerrillaAnalyzer::generateReport(const AnalysisResult& result,
                                          const QVector<DataPacket>& sent,
                                          const QVector<DataPacket>& received)
{
    QString report;

    report += "=== АНАЛИЗ СООТВЕТСТВИЯ (БЕСПОРЯДОЧНЫЙ) ===\n\n";

    // Общая статистика
    report += QString("Отправлено: %1 пакетов\n").arg(result.totalSent);
    report += QString("Получено: %1 пакетов\n").arg(result.totalReceived);
    report += QString("Совпало: %1 пакетов\n").arg(result.matched);

    if (result.matched > 0) {
        report += QString("  • В правильном порядке: %1\n").arg(result.inOrder);
        report += QString("  • В неправильном порядке: %1\n").arg(result.outOfOrder);
    }

    if (result.lost > 0) {
        report += QString("Потеряно: %1 пакетов\n").arg(result.lost);
    }

    if (result.duplicates > 0) {
        report += QString("Дубликатов: %1 пакетов\n").arg(result.duplicates);
    }

    if (result.unknown > 0) {
        report += QString("Неизвестных: %1 пакетов\n").arg(result.unknown);
    }

    // Таблица соответствия
    if (!result.receivedToSent.isEmpty()) {
        report += "\nТАБЛИЦА СООТВЕТСТВИЯ:\n";
        report += "Полученный → Отправленный\n";

        QList<int> recvIndices = result.receivedToSent.keys();
        std::sort(recvIndices.begin(), recvIndices.end());

        int count = 0;
        for (int recvIndex : recvIndices) {
            int sentIndex = result.receivedToSent[recvIndex];
            report += QString("  %1 → %2 %3\n")
                          .arg(recvIndex, 3)
                          .arg(sentIndex, 3)
                          .arg(sentIndex == recvIndex ? "✓" : "✗");

            if (++count >= 50) { // Ограничим вывод
                report += QString("  ... и еще %1 пар\n").arg(result.receivedToSent.size() - 50);
                break;
            }
        }
    }

    // Потерянные пакеты (детально)
    if (!result.lostIndices.isEmpty()) {
        report += "\nПОТЕРЯННЫЕ ПАКЕТЫ:\n";
        report += "Индекс | Данные [3 байта]\n";

        for (int i = 0; i < qMin(result.lostIndices.size(), 20); i++) {
            int idx = result.lostIndices[i];
            if (idx < sent.size()) {
                const DataPacket& packet = sent[idx];
                report += QString("  %1 | [%2 %3 %4]\n")
                              .arg(idx, 3)
                              .arg(packet.data[0], 2, 16, QChar('0'))
                              .arg(packet.data[1], 2, 16, QChar('0'))
                              .arg(packet.data[2], 2, 16, QChar('0'));
            }
        }

        if (result.lostIndices.size() > 20) {
            report += QString("  ... и еще %1 пакетов\n").arg(result.lostIndices.size() - 20);
        }
    }

    // Неизвестные пакеты (детально)
    if (!result.unknownIndices.isEmpty()) {
        report += "\nНЕИЗВЕСТНЫЕ ПАКЕТЫ:\n";
        report += "Индекс | Данные [3 байта]\n";

        for (int i = 0; i < qMin(result.unknownIndices.size(), 20); i++) {
            int idx = result.unknownIndices[i];
            if (idx < received.size()) {
                const DataPacket& packet = received[idx];
                report += QString("  %1 | [%2 %3 %4]\n")
                              .arg(idx, 3)
                              .arg(packet.data[0], 2, 16, QChar('0'))
                              .arg(packet.data[1], 2, 16, QChar('0'))
                              .arg(packet.data[2], 2, 16, QChar('0'));
            }
        }

        if (result.unknownIndices.size() > 20) {
            report += QString("  ... и еще %1 пакетов\n").arg(result.unknownIndices.size() - 20);
        }
    }

    // Итог
    report += "\nИТОГ:\n";
    if (result.lost == 0 && result.unknown == 0 && result.duplicates == 0) {
        report += "✓ ВСЕ ПАКЕТЫ ПРИШЛИ\n";
        if (result.outOfOrder > 0) {
            report += QString("⚠  Но %1 пакетов в неправильном порядке\n").arg(result.outOfOrder);
        } else {
            report += "✓ И все в правильном порядке\n";
        }
    } else {
        report += "✗ ЕСТЬ ПРОБЛЕМЫ\n";
    }

    return report;
}

QString GuerrillaAnalyzer::hashPacket(const DataPacket& packet)
{
    // Хеш по 3 байтам данных
    return QString("%1-%2-%3")
        .arg(packet.data[0], 2, 16, QChar('0'))
        .arg(packet.data[1], 2, 16, QChar('0'))
        .arg(packet.data[2], 2, 16, QChar('0'))
        .toUpper();
}

bool GuerrillaAnalyzer::comparePackets(const DataPacket& a, const DataPacket& b)
{
    return (a.data[0] == b.data[0] &&
            a.data[1] == b.data[1] &&
            a.data[2] == b.data[2]);
}
