#ifndef TESTSEQUENCECOMPARATOR_H
#define TESTSEQUENCECOMPARATOR_H
#include <QString>
#include <QMap>

#include "../core/ppbprotocol.h"
class TestSequenceComparator {
public:
    struct Result {
        bool isPerfectMatch;
        int totalPackets;
        int errors;
        int missingPackets;
        int outOfOrderPackets;
        QMap<int, QString> detailedErrors; // позиция → описание ошибки
    };

    static Result compare(const QVector<DataPacket>& expected,
                          const QVector<DataPacket>& actual);

    static QString generateReport(const Result& result);
};

#endif // TESTSEQUENCECOMPARATOR_H
