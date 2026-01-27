#include "shiftdetector.h"
#include <cmath>
#include <QHash>
#include "../core/logger.h"

ShiftDetector::DetectionResult ShiftDetector::analyze(const QVector<DataPacket>& sent,
                                                      const QVector<DataPacket>& received)
{
    DetectionResult result;
    result.shift = 0;
    result.confidence = 0.0;

    if (sent.isEmpty() || received.isEmpty()) {
        result.analysis = "Нет данных для анализа";
        return result;
    }

    // Создаем хеш-таблицу отправленных пакетов
    QHash<QString, int> sentHash;
    for (int i = 0; i < sent.size(); i++) {
        QString key = hashPacket(sent[i]);
        sentHash[key] = i;
    }

    // Сопоставляем полученные пакеты с отправленными
    int matchCount = 0;
    int totalPossible = qMin(sent.size(), received.size());

    // Пробуем разные сдвиги от -20 до +20
    QMap<int, int> shiftMatches;

    for (int shift = -20; shift <= 20; shift++) {
        int matches = 0;

        for (int i = 0; i < received.size(); i++) {
            int sentIndex = i + shift;
            if (sentIndex >= 0 && sentIndex < sent.size()) {
                QString receivedKey = hashPacket(received[i]);
                QString sentKey = hashPacket(sent[sentIndex]);

                if (receivedKey == sentKey) {
                    matches++;
                }
            }
        }

        shiftMatches[shift] = matches;

        Logger::debug(QString("Shift %1: %2 matches").arg(shift).arg(matches));
    }

    // Находим лучший сдвиг
    int bestShift = 0;
    int maxMatches = 0;

    for (auto it = shiftMatches.begin(); it != shiftMatches.end(); ++it) {
        if (it.value() > maxMatches) {
            maxMatches = it.value();
            bestShift = it.key();
        }
    }

    result.shift = bestShift;
    result.confidence = (double)maxMatches / qMin(sent.size(), received.size());

    // Анализируем качество сдвига
    if (maxMatches == 0) {
        result.analysis = "Сдвиг не обнаружен - пакеты не совпадают";
    } else if (result.confidence > 0.9) {
        result.analysis = QString("Обнаружен четкий сдвиг: %1 пакетов (совпало %2 из %3)")
                              .arg(bestShift)
                              .arg(maxMatches)
                              .arg(qMin(sent.size(), received.size()));
    } else if (result.confidence > 0.5) {
        result.analysis = QString("Возможный сдвиг: %1 пакетов (низкое качество, совпало %2 из %3)")
                              .arg(bestShift)
                              .arg(maxMatches)
                              .arg(qMin(sent.size(), received.size()));
    } else {
        result.analysis = QString("Слабое соответствие (совпало %1 из %2), возможно несколько сдвигов")
                              .arg(maxMatches)
                              .arg(qMin(sent.size(), received.size()));
    }

    return result;
}

QString ShiftDetector::hashPacket(const DataPacket& packet)
{
    // Создаем уникальный ключ из данных и CRC
    return QString("%1-%2-%3-%4")
        .arg(packet.data[0], 2, 16, QChar('0'))
        .arg(packet.data[1], 2, 16, QChar('0'))
        .arg(packet.data[2], 2, 16, QChar('0'))
        .arg(packet.crc, 2, 16, QChar('0'))
        .toUpper();
}
