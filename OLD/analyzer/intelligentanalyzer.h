#ifndef INTELLIGENTANALYZER_H
#define INTELLIGENTANALYZER_H

#include "../core/packetbuilder.h"
#include <QVector>
#include <QString>
#include <QMap>

class IntelligentAnalyzer
{
public:
    struct AnalysisResult {
        bool hasShift = false;
        int shift = 0;
        double shiftConfidence = 0.0;

        int totalSent = 0;
        int totalReceived = 0;
        int matched = 0;
        int lost = 0;
        int duplicates = 0;
        int unknown = 0;
        int outOfOrder = 0;  // Пакеты в неправильном порядке

        QMap<int, int> receivedToSent; // Карта соответствия: полученный -> отправленный
        QVector<int> lostIndices;
        QVector<int> duplicateIndices;
        QVector<int> unknownIndices;
        QVector<QPair<int, int>> outOfOrderPairs; // (ожидаемый, фактический)

        QString diagnosis;
        QString recommendations;
    };

    struct BlockInfo {
        int start;      // Начало блока
        int length;     // Длина блока
        int shift;      // Сдвиг блока
        bool isValid;   // Корректный блок
    };

    struct BlockAnalysisResult {
        QList<BlockInfo> blocks;
        int totalBlocks = 0;
        int maxBlockSize = 0;
        int minBlockSize = 0;
        QString analysis;
    };

    // Основной метод анализа
    AnalysisResult analyze(const QVector<DataPacket>& sent,
                           const QVector<DataPacket>& received);

    // Блочный анализ с различными сдвигами
    BlockAnalysisResult analyzeBlocks(const QVector<DataPacket>& sent,
                                      const QVector<DataPacket>& received);

private:
    // Внутренние методы анализа
    AnalysisResult analyzeWithShift(const QVector<DataPacket>& sent,
                                    const QVector<DataPacket>& received);

    AnalysisResult analyzeWithoutShift(const QVector<DataPacket>& sent,
                                       const QVector<DataPacket>& received);

    // Сравнение пакетов
    static bool packetsMatch(const DataPacket& a, const DataPacket& b);

    // Поиск наилучшего сдвига
    static int findBestShift(const QVector<DataPacket>& sent,
                             const QVector<DataPacket>& received,
                             double& confidence);

    // Хеширование пакета
    static QString hashPacket(const DataPacket& packet);

    // Генерация диагноза
    static QString generateDiagnosis(const AnalysisResult& result);

    // Генерация рекомендаций
    static QString generateRecommendations(const AnalysisResult& result);

    // Построение хеш-таблицы
    static QHash<QString, int> buildHashTable(const QVector<DataPacket>& packets);
};

#endif // INTELLIGENTANALYZER_H
