#ifndef GUERRILLAANALYZER_H
#define GUERRILLAANALYZER_H

#include "../core/packetbuilder.h"
#include <QVector>
#include <QString>
#include <QMap>
#include <QList>

class GuerrillaAnalyzer
{
public:
    struct AnalysisResult {
        int totalSent = 0;
        int totalReceived = 0;
        int matched = 0;           // Совпавшие пакеты (по содержимому)
        int inOrder = 0;           // В правильном порядке
        int outOfOrder = 0;        // В неправильном порядке
        int lost = 0;              // Отправленные, но не найденные в полученных
        int duplicates = 0;        // Дубликаты (несколько полученных на один отправленный)
        int unknown = 0;           // Полученные, которые не удалось сопоставить

        QList<int> lostIndices;                 // Индексы потерянных пакетов
        QList<int> duplicateIndices;            // Индексы дубликатов
        QList<int> unknownIndices;              // Индексы неизвестных пакетов
        QMap<int, int> receivedToSent;          // Полученный индекс -> отправленный индекс
    };

    // Анализ соответствия пакетов по содержимому (беспорядочный)
    AnalysisResult analyze(const QVector<DataPacket>& sent,
                           const QVector<DataPacket>& received,
                           int searchWindow = 5);

    // Генерация отчета на основе результатов анализа
    QString generateReport(const AnalysisResult& result,
                           const QVector<DataPacket>& sent,
                           const QVector<DataPacket>& received);

private:
    // Хеширование пакета по его содержимому
    static QString hashPacket(const DataPacket& packet);

    // Сравнение пакетов
    static bool comparePackets(const DataPacket& a, const DataPacket& b);
};

#endif // GUERRILLAANALYZER_H
