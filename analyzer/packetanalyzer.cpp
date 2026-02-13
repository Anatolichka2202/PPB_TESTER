#include "packetanalyzer.h"
#include <QDebug>
#include <QtMath>
#include <iostream>
#include "../core/utilits/crc.h"


PacketAnalyzer::PacketAnalyzer(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<AnalysisResult>();
    qRegisterMetaType<AnalysisResult::PacketErrorDetail>();
}

void PacketAnalyzer::addSentPacket(const DataPacket &packet)
{


    if (packet.counter > 255) {
        emit errorOccurred(QString("Invalid packet index: %1").arg(packet.counter));
        return;
    }

    bool valid = m_checkCRC ? checkPacketCRC(packet) : true;
    PacketInfo info(packet, m_sentSequenceCounter++, valid);

    m_sentPackets[packet.counter] = info;
    m_sentSequence[info.sequenceNumber] = packet.counter;
}

void PacketAnalyzer::addReceivedPacket(const DataPacket &packet)
{


    if (packet.counter > 255) {
        emit errorOccurred(QString("Invalid packet index: %1").arg(packet.counter));
        return;
    }

    bool valid = m_checkCRC ? checkPacketCRC(packet) : true;
    PacketInfo info(packet, m_receivedSequenceCounter++, valid);

    m_receivedPackets[packet.counter] = info;
    m_receivedSequence[info.sequenceNumber] = packet.counter;
}

void PacketAnalyzer::addSentPackets(const QVector<DataPacket> &packets)
{

    for (const DataPacket &packet : packets) {
        addSentPacket(packet);
    }
}

void PacketAnalyzer::addReceivedPackets(const QVector<DataPacket> &packets)
{

    for (const DataPacket &packet : packets) {
        addReceivedPacket(packet);
    }
}

PacketAnalyzer::AnalysisResult PacketAnalyzer::analyze()
{

    std::cout << "DEBUG: PacketAnalyzer::analyze() начат" << std::endl;
    std::cout << "DEBUG: Отправлено пакетов: " << m_sentPackets.size() << std::endl;
    std::cout << "DEBUG: Получено пакетов: " << m_receivedPackets.size() << std::endl;
    std::cout << "DEBUG: checkCRC = " << m_checkCRC << std::endl;

    emit analysisStarted();

    QElapsedTimer timer;
    timer.start();

    AnalysisResult result;

    // Базовые счетчики
    result.totalSent = m_sentPackets.size();
    result.totalReceived = m_receivedPackets.size();

    // Анализируем каждый отправленный пакет
    int processed = 0;
    for (auto it = m_sentPackets.begin(); it != m_sentPackets.end(); ++it) {
        uint8_t index = it.key();
        const PacketInfo &sentInfo = it.value();

        // Проверяем, есть ли полученный пакет с таким же индексом
        auto receivedIt = m_receivedPackets.find(index);

        if (receivedIt == m_receivedPackets.end()) {
            // Пакет потерян
            result.lostPackets++;
            result.lostPacketIndices.append(index);

            AnalysisResult::PacketErrorDetail detail;
            detail.index = index;
            detail.isLost = true;
            detail.isOutOfOrder = false;
            detail.hasCrcError = false;
            detail.bitErrors = 0;
            detail.sentData = packetToString(sentInfo.packet);
            detail.receivedData = "LOST";
            result.errorDetails.append(detail);

            processed++;
            if (result.totalSent > 0) {
                emit analysisProgress((processed * 100) / result.totalSent);
            }
            continue;
        }

        const PacketInfo &receivedInfo = receivedIt.value();

        // Проверяем порядок
        bool isOutOfOrder = false;
        if (sentInfo.sequenceNumber >= 0 && receivedInfo.sequenceNumber >= 0) {
            int expectedPosition = sentInfo.sequenceNumber;
            int actualPosition = receivedInfo.sequenceNumber;

            if (qAbs(expectedPosition - actualPosition) > m_maxWindow) {
                isOutOfOrder = true;
                result.outOfOrderPackets++;
                result.outOfOrderIndices.append(index);
            }
        }

        // Проверяем CRC полученного пакета
        bool crcValid = receivedInfo.isValid;
        if (!crcValid) {
            result.crcErrors++;
            result.crcErrorIndices.append(index);
        }

        // Считаем битовые ошибки
        int bitErrors = 0;
        if (crcValid || !m_checkCRC) {
            bitErrors = countBitErrors(sentInfo.packet, receivedInfo.packet);
            result.bitErrors += bitErrors;
            result.totalBitsCompared += 16; // 2 байта данных = 16 бит

            if (bitErrors == 0 && crcValid) {
                result.validPackets++;
            }
        }

        // Добавляем детали
        AnalysisResult::PacketErrorDetail detail;
        detail.index = index;
        detail.isLost = false;
        detail.isOutOfOrder = isOutOfOrder;
        detail.hasCrcError = !crcValid;
        detail.bitErrors = bitErrors;
        detail.sentData = packetToString(sentInfo.packet);
        detail.receivedData = packetToString(receivedInfo.packet);
        result.errorDetails.append(detail);

        processed++;
        if (result.totalSent > 0) {
            emit analysisProgress((processed * 100) / result.totalSent);
        }
    }

    // Проверяем лишние пакеты (полученные, но не отправленные)
    for (auto it = m_receivedPackets.begin(); it != m_receivedPackets.end(); ++it) {
        uint8_t index = it.key();
        if (!m_sentPackets.contains(index)) {
            result.lostPackets++;
            result.lostPacketIndices.append(index);

            const PacketInfo &receivedInfo = it.value();

            AnalysisResult::PacketErrorDetail detail;
            detail.index = index;
            detail.isLost = true;
            detail.isOutOfOrder = false;
            detail.hasCrcError = !receivedInfo.isValid;
            detail.bitErrors = 0;
            detail.sentData = "NOT SENT";
            detail.receivedData = packetToString(receivedInfo.packet);
            result.errorDetails.append(detail);
        }
    }

    // Рассчитываем rates
    if (result.totalSent > 0) {
        result.packetLossRate = static_cast<double>(result.lostPackets) / result.totalSent;
        result.outOfOrderRate = static_cast<double>(result.outOfOrderPackets) / result.totalSent;

        if (result.totalBitsCompared > 0) {
            result.ber = static_cast<double>(result.bitErrors) / result.totalBitsCompared;
        }
    }

    result.analysisTimeMs = timer.elapsed();

    emit analysisComplete(result);
    return result;
}

PacketAnalyzer::AnalysisResult PacketAnalyzer::analyze(
    const QVector<DataPacket> &sent,
    const QVector<DataPacket> &received)
{
    std::cout << "DEBUG: Вызов analyze с векторами" << std::endl;
    std::cout << "DEBUG: Размер sent: " << sent.size() << std::endl;
    std::cout << "DEBUG: Размер received: " << received.size() << std::endl;
    clear();
    addSentPackets(sent);
    addReceivedPackets(received);
    return analyze();
}

void PacketAnalyzer::clear()
{


    m_sentPackets.clear();
    m_receivedPackets.clear();
    m_sentSequence.clear();
    m_receivedSequence.clear();
    m_sentSequenceCounter = 0;
    m_receivedSequenceCounter = 0;
}

int PacketAnalyzer::countBitErrors(uint8_t a, uint8_t b) const
{
    int errors = 0;
    uint8_t xorResult = a ^ b;

    while (xorResult) {
        errors += xorResult & 1;
        xorResult >>= 1;
    }

    return errors;
}

int PacketAnalyzer::countBitErrors(const DataPacket &sent, const DataPacket &received) const
{
    int errors = 0;

    errors += countBitErrors(sent.data[0], received.data[0]);
    errors += countBitErrors(sent.data[1], received.data[1]);

    return errors;
}

bool PacketAnalyzer::checkPacketCRC(const DataPacket &packet) const
{
    uint8_t dataForCRC[3] = {packet.data[0], packet.data[1], packet.counter};
    uint8_t calculatedCRC = calculateCRC8(dataForCRC, 3);
    return calculatedCRC == packet.crc;
}

QString PacketAnalyzer::packetToString(const DataPacket &packet) const
{
    return QString("[%1 %2] idx:%3 crc:%4")
    .arg(packet.data[0], 2, 16, QChar('0'))
        .arg(packet.data[1], 2, 16, QChar('0'))
        .arg(packet.counter, 3, 10, QChar('0'))
        .arg(packet.crc, 2, 16, QChar('0'));
}
