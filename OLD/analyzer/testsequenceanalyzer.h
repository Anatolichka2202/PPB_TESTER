#ifndef TESTSEQUENCEANALYZER_H
#define TESTSEQUENCEANALYZER_H

#include "../core/packetbuilder.h"
#include <QVector>
#include <QString>

class TestSequenceAnalyzer
{
public:
    struct TestResult {
        bool success = false;
        QString report;
        int packetsSent = 0;
        int packetsReceived = 0;
        int packetsLost = 0;
        int packetsCorrupted = 0;
        double lossPercentage = 0.0;
        int detectedShift = 0;
    };

    TestResult analyzeTest(const QVector<DataPacket>& sentPackets,
                           const QVector<DataPacket>& receivedPackets);

    // Новый метод для сохранения таблиц в файл
    static void saveTablesToFile(const QVector<DataPacket>& sent,
                                 const QVector<DataPacket>& received,
                                 const QString& filename);

private:
    // Вспомогательный метод для хеширования пакета
    static QString hashPacket(const DataPacket& packet);
};

#endif // TESTSEQUENCEANALYZER_H
