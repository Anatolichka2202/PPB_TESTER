#include "intelligentanalyzer.h"
#include <cmath>
#include <algorithm>
#include <QDebug>
#include <QHash>
#include <QSet>
#include "../core/logger.h"

IntelligentAnalyzer::AnalysisResult
IntelligentAnalyzer::analyze(const QVector<DataPacket>& sent,
                             const QVector<DataPacket>& received)
{
    AnalysisResult result;
    result.totalSent = sent.size();
    result.totalReceived = received.size();

    if (sent.isEmpty() || received.isEmpty()) {
        result.diagnosis = "Один из массивов данных пуст";
        return result;
    }

    // Шаг 1: Создание хеш-таблиц для быстрого поиска
    QHash<QString, QList<int>> sentHash; // хеш -> список индексов (для дубликатов)
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

    // Сначала сопоставляем уникальные совпадения
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
                    // В правильном порядке
                } else {
                    result.outOfOrder++;
                    result.outOfOrderPairs.append(qMakePair(sentIndex, recvIndex));
                }
            }

            // Лишние полученные пакеты с тем же хешем - дубликаты
            if (recvIndices.size() > sentIndices.size()) {
                result.duplicates += recvIndices.size() - sentIndices.size();
                for (int i = pairsToMatch; i < recvIndices.size(); i++) {
                    result.duplicateIndices.append(recvIndices[i]);
                }
            }
        }
    }

    // Шаг 3: Поиск потерянных пакетов
    for (int i = 0; i < sentMatched.size(); i++) {
        if (!sentMatched[i]) {
            result.lostIndices.append(i);
            result.lost++;
        }
    }

    // Шаг 4: Поиск неизвестных пакетов
    for (int i = 0; i < receivedMatched.size(); i++) {
        if (!receivedMatched[i]) {
            result.unknownIndices.append(i);
            result.unknown++;
        }
    }

    // Шаг 5: Анализ сдвига
    double confidence = 0.0;
    int shift = findBestShift(sent, received, confidence);

    if (confidence > 0.7) {
        result.hasShift = true;
        result.shift = shift;
        result.shiftConfidence = confidence;
    }

    // Шаг 6: Формирование отчета
    result.diagnosis = generateDiagnosis(result);
    result.recommendations = generateRecommendations(result);

    Logger::info(QString("IntelligentAnalyzer: анализ завершен. Совпало: %1/%2")
                     .arg(result.matched).arg(result.totalSent));

    return result;
}

IntelligentAnalyzer::BlockAnalysisResult
IntelligentAnalyzer::analyzeBlocks(const QVector<DataPacket>& sent,
                                   const QVector<DataPacket>& received)
{
    BlockAnalysisResult result;

    if (sent.isEmpty() || received.isEmpty()) {
        result.analysis = "Нет данных для анализа блоков";
        return result;
    }

    // Сначала получим сопоставление пакетов
    AnalysisResult basicAnalysis = analyze(sent, received);

    if (basicAnalysis.receivedToSent.isEmpty()) {
        result.analysis = "Нет сопоставленных пакетов для анализа блоков";
        return result;
    }

    // Преобразуем в список пар (полученный, отправленный)
    QList<QPair<int, int>> pairs;
    for (auto it = basicAnalysis.receivedToSent.begin(); it != basicAnalysis.receivedToSent.end(); ++it) {
        pairs.append(qMakePair(it.key(), it.value())); // полученный, отправленный
    }

    // Сортируем по индексу полученного пакета
    std::sort(pairs.begin(), pairs.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.first < b.first;
              });

    // Анализ блоков
    bool inBlock = false;
    int currentBlockStart = 0;
    int currentBlockShift = 0;
    int currentBlockLength = 0;

    for (int i = 0; i < pairs.size(); i++) {
        int receivedIndex = pairs[i].first;
        int sentIndex = pairs[i].second;
        int shift = receivedIndex - sentIndex; // сдвиг = полученный - отправленный

        if (!inBlock) {
            // Начало нового блока
            inBlock = true;
            currentBlockStart = receivedIndex;
            currentBlockShift = shift;
            currentBlockLength = 1;
        } else {
            // Проверяем, продолжается ли текущий блок
            if (shift == currentBlockShift) {
                // Тот же сдвиг - продолжаем блок
                currentBlockLength++;
            } else {
                // Сдвиг изменился - сохраняем текущий блок
                BlockInfo block;
                block.start = currentBlockStart;
                block.length = currentBlockLength;
                block.shift = currentBlockShift;
                block.isValid = true;
                result.blocks.append(block);

                // Начинаем новый блок
                currentBlockStart = receivedIndex;
                currentBlockShift = shift;
                currentBlockLength = 1;
            }
        }
    }

    // Сохраняем последний блок
    if (inBlock && currentBlockLength > 0) {
        BlockInfo block;
        block.start = currentBlockStart;
        block.length = currentBlockLength;
        block.shift = currentBlockShift;
        block.isValid = true;
        result.blocks.append(block);
    }

    // Статистика по блокам
    result.totalBlocks = result.blocks.size();
    if (!result.blocks.isEmpty()) {
        result.minBlockSize = result.blocks.first().length;
        for (const BlockInfo& block : result.blocks) {
            result.maxBlockSize = qMax(result.maxBlockSize, block.length);
            result.minBlockSize = qMin(result.minBlockSize, block.length);
        }
    }

    // Формирование отчета
    result.analysis = "=== БЛОЧНЫЙ АНАЛИЗ ===\n\n";
    result.analysis += QString("Обнаружено блоков: %1\n").arg(result.totalBlocks);

    if (result.totalBlocks > 0) {
        result.analysis += QString("Максимальный размер блока: %1 пакетов\n").arg(result.maxBlockSize);
        result.analysis += QString("Минимальный размер блока: %1 пакетов\n\n").arg(result.minBlockSize);

        result.analysis += "Детали блоков:\n";
        result.analysis += "№ | Начало | Длина | Сдвиг\n";
        result.analysis += "--|--------|-------|------\n";

        for (int i = 0; i < result.blocks.size(); i++) {
            const BlockInfo& block = result.blocks[i];
            result.analysis += QString("%1 | %2 | %3 | %4\n")
                                   .arg(i + 1)
                                   .arg(block.start)
                                   .arg(block.length)
                                   .arg(block.shift);
        }

        // Анализ разнообразия сдвигов
        QSet<int> shifts;
        for (const BlockInfo& block : result.blocks) {
            shifts.insert(block.shift);
        }

        result.analysis += QString("\nУникальных сдвигов: %1\n").arg(shifts.size());
        if (shifts.size() > 1) {
            result.analysis += "⚠  Внимание: обнаружены разные сдвиги в разных блоках!\n";
        }
    } else {
        result.analysis += "Блоки не обнаружены\n";
    }

    return result;
}

bool IntelligentAnalyzer::packetsMatch(const DataPacket& a, const DataPacket& b)
{
    // Сравниваем только данные (3 байта)
    return (a.data[0] == b.data[0] &&
            a.data[1] == b.data[1] &&
            a.data[2] == b.data[2]);
}

int IntelligentAnalyzer::findBestShift(const QVector<DataPacket>& sent,
                                       const QVector<DataPacket>& received,
                                       double& confidence)
{
    if (sent.isEmpty() || received.isEmpty()) {
        confidence = 0.0;
        return 0;
    }

    // Подсчитываем сдвиги для всех сопоставленных пар
    QMap<int, int> shiftCount;
    int totalMatched = 0;

    // Быстрое сопоставление через хеши
    QHash<QString, QList<int>> sentHash;
    QHash<QString, QList<int>> receivedHash;

    for (int i = 0; i < sent.size(); i++) {
        sentHash[hashPacket(sent[i])].append(i);
    }

    for (int i = 0; i < received.size(); i++) {
        receivedHash[hashPacket(received[i])].append(i);
    }

    // Сопоставляем первые совпадения
    for (auto recvIt = receivedHash.begin(); recvIt != receivedHash.end(); ++recvIt) {
        QString hash = recvIt.key();
        QList<int> recvIndices = recvIt.value();

        if (sentHash.contains(hash)) {
            QList<int> sentIndices = sentHash[hash];
            int pairsToMatch = qMin(recvIndices.size(), sentIndices.size());

            for (int i = 0; i < pairsToMatch; i++) {
                int shift = recvIndices[i] - sentIndices[i];
                shiftCount[shift]++;
                totalMatched++;
            }
        }
    }

    if (totalMatched == 0) {
        confidence = 0.0;
        return 0;
    }

    // Находим наиболее частый сдвиг
    int bestShift = 0;
    int maxCount = 0;
    for (auto it = shiftCount.begin(); it != shiftCount.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            bestShift = it.key();
        }
    }

    confidence = (double)maxCount / totalMatched;

    return bestShift;
}

QString IntelligentAnalyzer::hashPacket(const DataPacket& packet)
{
    // Хеш по 3 байтам данных (без CRC)
    return QString("%1-%2-%3")
        .arg(packet.data[0], 2, 16, QChar('0'))
        .arg(packet.data[1], 2, 16, QChar('0'))
        .arg(packet.data[2], 2, 16, QChar('0'))
        .toUpper();
}

QString IntelligentAnalyzer::generateDiagnosis(const AnalysisResult& result)
{
    QString diagnosis;

    diagnosis += "=== ДИАГНОСТИКА ТЕСТОВОЙ ПОСЛЕДОВАТЕЛЬНОСТИ ===\n\n";

    // Основная статистика
    diagnosis += QString("Отправлено: %1 пакетов\n").arg(result.totalSent);
    diagnosis += QString("Получено: %1 пакетов\n").arg(result.totalReceived);
    diagnosis += QString("Совпало: %1 пакетов\n").arg(result.matched);

    if (result.hasShift) {
        diagnosis += QString("Обнаружен сдвиг: %1 пакетов (уверенность: %2%)\n")
                         .arg(result.shift)
                         .arg(result.shiftConfidence * 100, 0, 'f', 1);
    }

    if (result.matched > 0) {
        diagnosis += QString("В правильном порядке: %1 пакетов\n").arg(result.matched - result.outOfOrder);
        if (result.outOfOrder > 0) {
            diagnosis += QString("В неправильном порядке: %1 пакетов\n").arg(result.outOfOrder);
        }
    }

    if (result.lost > 0) {
        diagnosis += QString("Потеряно: %1 пакетов\n").arg(result.lost);
    }

    if (result.duplicates > 0) {
        diagnosis += QString("Дубликатов: %1\n").arg(result.duplicates);
    }

    if (result.unknown > 0) {
        diagnosis += QString("Неизвестных пакетов: %1\n").arg(result.unknown);
    }

    // Оценка качества
    double quality = 0.0;
    if (result.totalSent > 0) {
        quality = (double)result.matched / result.totalSent;
    }

    diagnosis += QString("\nКачество передачи: %1%\n").arg(quality * 100, 0, 'f', 1);

    if (quality > 0.95) {
        diagnosis += "Отличное качество передачи ✓\n";
    } else if (quality > 0.8) {
        diagnosis += "Хорошее качество передачи\n";
    } else if (quality > 0.6) {
        diagnosis += "Удовлетворительное качество передачи\n";
    } else {
        diagnosis += "Низкое качество передачи, требуется анализ проблем\n";
    }

    return diagnosis;
}

QString IntelligentAnalyzer::generateRecommendations(const AnalysisResult& result)
{
    QString recommendations = "\n=== РЕКОМЕНДАЦИИ ===\n";

    if (result.lost > result.totalSent * 0.1) { // >10% потерь
        recommendations += "1. Высокий процент потерь:\n";
        recommendations += "   • Увеличить интервал между пакетами\n";
        recommendations += "   • Проверить стабильность сети\n";
        recommendations += "   • Проверить настройки таймаутов\n";
    }

    if (result.hasShift && std::abs(result.shift) > 0) {
        recommendations += QString("2. Обнаружен сдвиг %1 пакетов:\n").arg(result.shift);
        recommendations += "   • Это может быть вызвано задержкой обработки\n";
        recommendations += "   • Рекомендуется добавить паузу перед началом теста\n";
    }

    if (result.outOfOrder > 0) {
        recommendations += "3. Нарушен порядок пакетов:\n";
        recommendations += "   • UDP пакеты могут приходить в произвольном порядке\n";
        recommendations += "   • Это нормально для UDP протокола\n";
    }

    if (result.duplicates > 0) {
        recommendations += "4. Обнаружены дубликаты:\n";
        recommendations += "   • Возможно, ППБ повторно отправляет пакеты\n";
        recommendations += "   • Проверить логику отправки на стороне ППБ\n";
    }

    if (result.unknown > 0) {
        recommendations += "5. Неизвестные пакеты:\n";
        recommendations += "   • Получены пакеты, которые не были отправлены\n";
        recommendations += "   • Проверить, не ведется ли параллельное общение\n";
    }

    // Общие рекомендации
    recommendations += "\n6. Общие рекомендации:\n";
    recommendations += "   • Включить детальное логирование\n";
    recommendations += "   • Провести несколько тестов для статистики\n";
    recommendations += "   • Сохранять полные логи каждого теста\n";

    return recommendations;
}

QHash<QString, int> IntelligentAnalyzer::buildHashTable(const QVector<DataPacket>& packets)
{
    QHash<QString, int> hashTable;
    for (int i = 0; i < packets.size(); i++) {
        hashTable[hashPacket(packets[i])] = i;
    }
    return hashTable;
}

// Эти методы теперь не используются напрямую, но оставим для совместимости
IntelligentAnalyzer::AnalysisResult
IntelligentAnalyzer::analyzeWithShift(const QVector<DataPacket>& sent,
                                      const QVector<DataPacket>& received)
{
    return analyze(sent, received);
}

IntelligentAnalyzer::AnalysisResult
IntelligentAnalyzer::analyzeWithoutShift(const QVector<DataPacket>& sent,
                                         const QVector<DataPacket>& received)
{
    return analyze(sent, received);
}
