#ifndef PACKETANALYZER_H
#define PACKETANALYZER_H

// Вместо полных путей используем относительные или копируем нужные структуры
#include <cstdint>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QElapsedTimer>
#include <QString>
#include "../core/communication/ppbprotocol.h"
#include "../core/utilits/crc.h"


class PacketAnalyzer : public QObject
{
    Q_OBJECT

public:
    struct AnalysisResult {
        // Основные метрики
        double ber = 0.0;                    // Bit Error Rate
        double packetLossRate = 0.0;         // Rate потери пакетов
        double outOfOrderRate = 0.0;         // Rate пакетов не в порядке

        // Количественные показатели
        int totalSent = 0;
        int totalReceived = 0;
        int validPackets = 0;
        int lostPackets = 0;
        int outOfOrderPackets = 0;
        int crcErrors = 0;
        int bitErrors = 0;
        int totalBitsCompared = 0;

        // Детализация
        QVector<uint8_t> lostPacketIndices;
        QVector<uint8_t> outOfOrderIndices;
        QVector<uint8_t> crcErrorIndices;

        // Время анализа
        qint64 analysisTimeMs = 0;

        // Детали ошибок по пакетам
        struct PacketErrorDetail {
            uint8_t index;
            bool isLost;
            bool isOutOfOrder;
            bool hasCrcError;
            int bitErrors;
            QString sentData;
            QString receivedData;
        };
        QVector<PacketErrorDetail> errorDetails;

        QString toString() const {
            QString result;
            result += QString("=== Результаты анализа ===\n");
            result += QString("Отправлено пакетов: %1\n").arg(totalSent);
            result += QString("Получено пакетов:  %1\n").arg(totalReceived);
            result += QString("Потери:            %1 (%2%)\n")
                          .arg(lostPackets).arg(packetLossRate * 100, 0, 'f', 2);
            result += QString("Не в порядке:      %1 (%2%)\n")
                          .arg(outOfOrderPackets).arg(outOfOrderRate * 100, 0, 'f', 2);
            result += QString("Ошибки CRC:        %1\n").arg(crcErrors);
            result += QString("Битовые ошибки:    %1\n").arg(bitErrors);
            result += QString("BER:               %1 (%.3f%%)\n")
                          .arg(ber, 0, 'g', 6).arg(ber * 100, 0, 'f', 3);
            result += QString("Общее сравнено бит: %1\n").arg(totalBitsCompared);
            result += QString("Время анализа:     %1 мс\n").arg(analysisTimeMs);

            return result;
        }
    };

    explicit PacketAnalyzer(QObject *parent = nullptr);

    // Основные методы
    void addSentPacket(const DataPacket &packet);
    void addReceivedPacket(const DataPacket &packet);
    void addSentPackets(const QVector<DataPacket> &packets);
    void addReceivedPackets(const QVector<DataPacket> &packets);

    // Анализ
    AnalysisResult analyze();
    AnalysisResult analyze(const QVector<DataPacket> &sent,
                           const QVector<DataPacket> &received);

    // Очистка данных
    void clear();

    // Геттеры
    int sentCount() const { return m_sentPackets.size(); }
    int receivedCount() const { return m_receivedPackets.size(); }

    // Настройки
    void setCheckCRC(bool check) { m_checkCRC = check; }
    void setMaxReorderingWindow(int window) { m_maxWindow = window; }
    bool checkCRC() const { return m_checkCRC; }
    int maxReorderingWindow() const { return m_maxWindow; }

signals:
    void analysisStarted();
    void analysisProgress(int percent);
    void analysisComplete(const AnalysisResult &result);
    void errorOccurred(const QString &error);

private:
    struct PacketInfo {
        DataPacket packet;
        int sequenceNumber;  // Порядковый номер получения/отправки
        bool isValid;

        PacketInfo() : sequenceNumber(-1), isValid(false) {}
        PacketInfo(const DataPacket &p, int seq, bool valid)
            : packet(p), sequenceNumber(seq), isValid(valid) {}
    };

    // Вспомогательные методы
    int countBitErrors(uint8_t a, uint8_t b) const;
    int countBitErrors(const DataPacket &sent, const DataPacket &received) const;
    bool checkPacketCRC(const DataPacket &packet) const;
    QString packetToString(const DataPacket &packet) const;

    // Данные
    QMap<uint8_t, PacketInfo> m_sentPackets;
    QMap<uint8_t, PacketInfo> m_receivedPackets;

    QMap<int, uint8_t> m_sentSequence;
    QMap<int, uint8_t> m_receivedSequence;

    int m_sentSequenceCounter = 0;
    int m_receivedSequenceCounter = 0;

    // Настройки
    bool m_checkCRC = true;
    int m_maxWindow = 20;  // Окно для детектирования реордеринга

};

// Регистрация типов для использования с QVariant
Q_DECLARE_METATYPE(PacketAnalyzer::AnalysisResult)
Q_DECLARE_METATYPE(PacketAnalyzer::AnalysisResult::PacketErrorDetail)

#endif // PACKETANALYZER_H
