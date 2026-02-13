#ifndef PACKETANALYZER_ADAPTER_H
#define PACKETANALYZER_ADAPTER_H

#pragma once

#include "packetanalyzer_interface.h"
#include "packetanalyzer.h"

class PacketAnalyzerAdapter : public PacketAnalyzerInterface {
    Q_OBJECT
private:
    PacketAnalyzer m_analyzer;

public:
    explicit PacketAnalyzerAdapter(QObject* parent = nullptr)
        : PacketAnalyzerInterface(parent) {
        // Подключаем сигналы внутреннего анализатора к нашим сигналам
        connect(&m_analyzer, &PacketAnalyzer::analysisStarted,
                this, &PacketAnalyzerAdapter::analysisStarted);
        connect(&m_analyzer, &PacketAnalyzer::analysisProgress,
                this, &PacketAnalyzerAdapter::analysisProgress);
        connect(&m_analyzer, &PacketAnalyzer::analysisComplete,
                [this](const PacketAnalyzer::AnalysisResult& result) {
                    emit analysisComplete(result.toString());

                    // Простая передача данных
                    QVariantMap details;
                    details["totalSent"] = result.totalSent;
                    details["totalReceived"] = result.totalReceived;
                    details["lostPackets"] = result.lostPackets;
                    details["ber"] = result.ber;

                    // Добавляем больше данных для совместимости
                    details["outOfOrderPackets"] = result.outOfOrderPackets;
                    details["crcErrors"] = result.crcErrors;
                    details["bitErrors"] = result.bitErrors;
                    details["packetLossRate"] = result.packetLossRate;
                    details["outOfOrderRate"] = result.outOfOrderRate;
                    details["analysisTimeMs"] = result.analysisTimeMs;

                    // Добавляем детали ошибок
                    QVariantList errorDetailsList;
                    for (const auto& detail : result.errorDetails) {
                        QVariantMap errorMap;
                        errorMap["index"] = detail.index;
                        errorMap["isLost"] = detail.isLost;
                        errorMap["isOutOfOrder"] = detail.isOutOfOrder;
                        errorMap["hasCrcError"] = detail.hasCrcError;
                        errorMap["bitErrors"] = detail.bitErrors;
                        errorMap["sentData"] = detail.sentData;
                        errorMap["receivedData"] = detail.receivedData;
                        errorDetailsList.append(errorMap);
                    }
                    details["errorDetails"] = errorDetailsList;

                    emit detailedResultsReady(details);
                });

        connect(&m_analyzer, &PacketAnalyzer::errorOccurred,
                [this](const QString& error) {
                    // Можно добавить обработку ошибок анализатора
                    qWarning() << "PacketAnalyzer error:" << error;
                });
    }

    void addSentPackets(const QVector<DataPacket>& packets) override {
        m_analyzer.addSentPackets(packets);
    }

    void addReceivedPackets(const QVector<DataPacket>& packets) override {
        m_analyzer.addReceivedPackets(packets);
    }

    void clear() override {
        m_analyzer.clear();
    }

    void analyze() override {
        m_analyzer.analyze();
    }

    void setCheckCRC(bool check) override {
        m_analyzer.setCheckCRC(check);
    }

    void setMaxReorderingWindow(int window) override {
        m_analyzer.setMaxReorderingWindow(window);
    }

    // Добавляем методы для получения статистики
    int sentCount() const {
        return m_analyzer.sentCount();
    }

    int receivedCount() const {
        return m_analyzer.receivedCount();
    }
};

#endif // PACKETANALYZER_ADAPTER_H
