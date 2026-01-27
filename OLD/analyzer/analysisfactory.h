#ifndef ANALYSISFACTORY_H
#define ANALYSISFACTORY_H

#include "../core/packetbuilder.h"
#include <QString>
#include <QVector>

class AnalysisFactory
{
public:
    static QString autoAnalyze(const QVector<DataPacket>& sent,
                               const QVector<DataPacket>& received);

private:
    static QString analyzeWithShiftDetector(const QVector<DataPacket>& sent,
                                            const QVector<DataPacket>& received);

    static QString analyzeWithGuerrilla(const QVector<DataPacket>& sent,
                                        const QVector<DataPacket>& received);

    static QString analyzeCombined(const QVector<DataPacket>& sent,
                                   const QVector<DataPacket>& received);
};

#endif // ANALYSISFACTORY_H
