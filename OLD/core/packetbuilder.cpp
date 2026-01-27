#include "packetbuilder.h"
#include <QDebug>
#include <QRandomGenerator>

QByteArray PacketBuilder::createTURequest(uint16_t address, TechCommand command)
{
    return createRequest(address, static_cast<uint8_t>(command),
                         Sign::TU, 0, nullptr);
}

QByteArray PacketBuilder::createFURequest(uint16_t address, uint8_t period,
                                          const uint8_t fuData[3])
{
    // Для ФУ команда всегда 0 (по ТЗ)
    return createRequest(address, 0, Sign::FU, period, fuData);
}

QByteArray PacketBuilder::createStatusRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::TS);
}

QByteArray PacketBuilder::createResetRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::TC);
}

QByteArray PacketBuilder::createPRBSTransmitRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::PRBS_M2S);
}

QByteArray PacketBuilder::createPRBSReceiveRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::PRBS_S2M);
}

QByteArray PacketBuilder::createDroppedPacketsRequest(uint16_t address)
{
    return createTURequest(address, TechCommand::DROP);
}

QByteArray PacketBuilder::createFUTransmitRequest(uint16_t address)
{
    // ФУ передача: команда 0, период 0
    return createFURequest(address, 0, nullptr);
}

QByteArray PacketBuilder::createFUReceiveRequest(uint16_t address, uint8_t period)
{
    return createFURequest(address, period, nullptr);
}

QByteArray PacketBuilder::createFUReceiveRequest(uint16_t address, uint8_t period,
                                                 const uint8_t fuData[3])
{
    return createFURequest(address, period, fuData);
}

// === парсинг ТУ ОК пакета ===
bool PacketBuilder::parsePPBResponse(const QByteArray& data, PPBResponse& response)
{
    if (data.size() != 4) {
        qDebug() << "Неверный размер ответа ППБ:" << data.size() << "ожидается 4";
        return false;
    }

    // Копируем данные
    memcpy(&response, data.constData(), sizeof(PPBResponse));



    // Проверяем CRC
    uint8_t calculatedCrc = calculateCRC8((uint8_t*)data.constData(), 3);
    if (calculatedCrc != response.crc) {
        qDebug() << "Ошибка CRC в ответе ППБ: рассчитано" << calculatedCrc
                 << "получено" << response.crc;
        return false;
    }

    qDebug() << "Ответ от адреса: 0x" << QString::number(response.address, 16).right(4).toUpper()
             << " (битовая маска)";
    if (response.address & 0x0001) qDebug() << "  - ППБ1";
    if (response.address & 0x0002) qDebug() << "  - ППБ2";
    if (response.address & 0x0004) qDebug() << "  - ППБ3";
    if (response.address & 0x0008) qDebug() << "  - ППБ4";
    if (response.address & 0x0010) qDebug() << "  - ППБ5";
    if (response.address & 0x0020) qDebug() << "  - ППБ6";
    if (response.address & 0x0040) qDebug() << "  - ППБ7";
    if (response.address & 0x0080) qDebug() << "  - ППБ8";

    return true;
}
// === парсинг ФУ ОК пакета
bool PacketBuilder::parseBridgeResponse(const QByteArray& data, BridgeResponse& response)
{
    if (data.size() != 4) {
        qDebug() << "Неверный размер ответа бриджа:" << data.size() << "ожидается 4";
        return false;
    }

    memcpy(&response, data.constData(), sizeof(BridgeResponse));

    qDebug() << "Ответ бриджа: адрес=" << response.address
             << ", команда=0x" << QString::number(response.command, 16)
             << ", статус=" << (int)response.status;

    return true;
}
// Парсинг старого ок пакета
bool PacketBuilder::parseOkResponse(const QByteArray& data, OkResponse& response)
{
    if (data.size() != static_cast<int>(sizeof(OkResponse))) {
        qDebug() << "Неверный размер OK-ответа:" << data.size()
                 << "ожидается:" << sizeof(OkResponse);
        return false;
    }

    memcpy(&response, data.constData(), sizeof(OkResponse));

    qDebug() << "OK-ответ: адрес=" << response.address
             << ", команда=0x" << QString::number(response.command, 16)
             << ", статус=" << response.status;

    return true;
}

bool PacketBuilder::parseDataPacket(const QByteArray& data, DataPacket& packet)
{
    if (data.size() != static_cast<int>(sizeof(DataPacket))) {
        qDebug() << "Неверный размер пакета данных:" << data.size()
                 << "ожидается:" << sizeof(DataPacket);
        return false;
    }

    memcpy(&packet, data.constData(), sizeof(DataPacket));
    return true;
}

bool PacketBuilder::checkDataPacketCRC(const DataPacket& packet)
{
    uint8_t calculatedCrc = calculateCRC8(packet.data, 3);
    return (calculatedCrc == packet.crc);
}

DataPacket PacketBuilder::createTestDataPacket(uint8_t data1, uint8_t data2, uint8_t data3)
{
    DataPacket packet;
    packet.data[0] = data1;
    packet.data[1] = data2;
    packet.data[2] = data3;
    packet.crc = calculateCRC8(packet.data, 3);
    return packet;
}

QVector<DataPacket> PacketBuilder::createTestDataSequence()
{
    QVector<DataPacket> sequence;
    sequence.reserve(256);

    for (int i = 0; i < 256; ++i) {
        // Генерируем тестовые данные (можно изменить логику генерации)
        uint8_t data1 = static_cast<uint8_t>(i);
        uint8_t data2 = static_cast<uint8_t>(i * 2);
        uint8_t data3 = static_cast<uint8_t>(i * 3);

        sequence.append(createTestDataPacket(data1, data2, data3));
    }

    return sequence;
}

QString PacketBuilder::compareSequences(const QVector<DataPacket>& sent,
                                        const QVector<DataPacket>& received)
{
    if (sent.size() != received.size()) {
        return QString("Размеры последовательностей не совпадают: отправлено %1, получено %2")
            .arg(sent.size()).arg(received.size());
    }

    int errors = 0;
    QStringList errorDetails;

    for (int i = 0; i < sent.size(); ++i) {
        const DataPacket& sentPacket = sent[i];
        const DataPacket& receivedPacket = received[i];

        // Проверяем данные
        if (memcmp(sentPacket.data, receivedPacket.data, 3) != 0) {
            errors++;
            errorDetails.append(QString("Пакет %1: отправлено [%2 %3 %4], получено [%5 %6 %7]")
                                    .arg(i)
                                    .arg(sentPacket.data[0], 2, 16, QChar('0'))
                                    .arg(sentPacket.data[1], 2, 16, QChar('0'))
                                    .arg(sentPacket.data[2], 2, 16, QChar('0'))
                                    .arg(receivedPacket.data[0], 2, 16, QChar('0'))
                                    .arg(receivedPacket.data[1], 2, 16, QChar('0'))
                                    .arg(receivedPacket.data[2], 2, 16, QChar('0')));
        }

        // Проверяем CRC
        if (sentPacket.crc != receivedPacket.crc) {
            errors++;
            errorDetails.append(QString("Пакет %1: CRC не совпадает (отправлено: %2, получено: %3)")
                                    .arg(i)
                                    .arg(sentPacket.crc, 2, 16, QChar('0'))
                                    .arg(receivedPacket.crc, 2, 16, QChar('0')));
        }
    }

    if (errors == 0) {
        return "Все пакеты совпадают";
    } else {
        return QString("Найдено %1 ошибок:\n%2")
            .arg(errors)
            .arg(errorDetails.join("\n"));
    }
}

// ===== PRIVATE METHODS =====

QByteArray PacketBuilder::createRequest(uint16_t address, uint8_t command,
                                        Sign sign, uint8_t period,
                                        const uint8_t fuData[3])
{
    BaseRequest request;

    request.address = address;
    request.command = command;
    request.sign = static_cast<uint8_t>(sign);
    request.fu_period = period;

    if (fuData) {
        memcpy(request.fu_data, fuData, 3);
    } else {
        memset(request.fu_data, 0, 3);
    }

    return QByteArray(reinterpret_cast<const char*>(&request), sizeof(request));
}
