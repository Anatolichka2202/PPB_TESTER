#ifndef SHIFTDETECTOR_H
#define SHIFTDETECTOR_H

#include "../core/packetbuilder.h"
#include <QVector>
#include <QString>
#include <QMap>

class ShiftDetector
{
public:
    struct DetectionResult {
        int shift = 0;
        double confidence = 0.0;
        QString analysis;
    };

    DetectionResult analyze(const QVector<DataPacket>& sent,
                            const QVector<DataPacket>& received);

private:
    QString hashPacket(const DataPacket& packet);
};

#endif // SHIFTDETECTOR_H
