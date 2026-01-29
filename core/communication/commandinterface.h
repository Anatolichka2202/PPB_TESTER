
#ifndef COMMANDINTERFACE_H
#define COMMANDINTERFACE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "ppbprotocol.h"

enum class PPBState;
struct DataPacket;
struct PPBResponse;
struct BridgeResponse;

class CommandInterface : public QObject
{
    Q_OBJECT
public:
    explicit CommandInterface(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~CommandInterface() = default;

    //INTERFACE
    virtual void setState(PPBState) = 0;
    virtual void startTimeoutTimer(int ms) = 0;
    virtual void stopTimeoutTimer() = 0;
    virtual void completeCurrentOperation(bool success, const QString& message="") = 0;
    virtual void sendPacket(const QByteArray& packet, const QString& description) = 0; //одиночная
    virtual void sendDataPackets(const QVector<DataPacket>& packets) = 0;              //вектор
    virtual QVector<DataPacket> getGeneratedPackets() const = 0;                       //генерация

signals:
    void statusDataReady(const QVector <QByteArray>& data);
    void testDataReady(const QVector<DataPacket>& data);
    void commandProgress(int current, int total);
};

#endif // COMMANDINTERFACE_H
