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


bool PacketBuilder::parseDataPacket(const QByteArray& data, DataPacket& packet) {
    if (data.size() != static_cast<int>(sizeof(DataPacket))) {
        qDebug() << "Неверный размер пакета данных:" << data.size()
                 << "ожидается:" << sizeof(DataPacket);
        return false;
    }

    memcpy(&packet, data.constData(), sizeof(DataPacket));

    // Проверяем CRC
    uint8_t dataForCRC[3] = {packet.data[0], packet.data[1], packet.counter};
    uint8_t calculatedCrc = calculateCRC8(dataForCRC, 3);

    if (calculatedCrc != packet.crc) {
        qDebug() << "Ошибка CRC в пакете данных: рассчитано" << calculatedCrc
                 << "получено" << packet.crc;
        return false;
    }

    return true;
}
bool PacketBuilder::checkDataPacketCRC(const DataPacket& packet)
{
    uint8_t dataForCRC[3] = {packet.data[0], packet.data[1], packet.counter};
    return calculateCRC8(dataForCRC, 3) == packet.crc;
}

DataPacket PacketBuilder::createTestDataPacket(uint8_t data1, uint8_t data2, uint8_t data3)
{
    DataPacket packet;
    packet.data[0] = data1;
    packet.data[1] = data2;
    packet.counter = data3;
    packet.crc = calculateCRC8(packet.data, 3);
    return packet;
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
