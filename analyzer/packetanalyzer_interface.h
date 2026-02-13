#ifndef PACKETANALYZER_INTERFACE_H
#define PACKETANALYZER_INTERFACE_H

#pragma once

#include <QObject>
#include <QVector>
#include <QVariantMap>

// Включаем протокол
#include "../core/communication/ppbprotocol.h"
#include "../core/utilits/crc.h"

class PacketAnalyzerInterface : public QObject {
    Q_OBJECT
public:
    explicit PacketAnalyzerInterface(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~PacketAnalyzerInterface() = default;

    // Основные методы
    virtual void addSentPackets(const QVector<DataPacket>& packets) = 0;
    virtual void addReceivedPackets(const QVector<DataPacket>& packets) = 0;
    virtual void clear() = 0;

    // Анализ
    virtual void analyze() = 0;

    // Настройки
    virtual void setCheckCRC(bool check) = 0;
    virtual void setMaxReorderingWindow(int window) = 0;

    // Геттеры для статистики
    virtual int sentCount() const = 0;
    virtual int receivedCount() const = 0;

signals:
    void analysisStarted();
    void analysisProgress(int percent);
    void analysisComplete(const QString& resultSummary);
    void detailedResultsReady(const QVariantMap& results);
};

#endif // PACKETANALYZER_INTERFACE_H
