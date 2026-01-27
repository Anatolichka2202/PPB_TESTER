#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include "ppbprotocol.h"
#include "crc.h"
#include <QByteArray>
#include <QVector>

class PacketBuilder
{
public:

    // Преобразование индекса ППБ (0-15) в битовый адрес
    uint16_t indexToAddress(int ppbNumber)  // ppbNumber от 1 до 16
    {
        if (ppbNumber < 1 || ppbNumber > 16)
            return 0xFFFF; // Широковещание на все 16 ППБ

        return (1 << (ppbNumber - 1));  // ppbNumber=1 -> 1<<0=0x0001
    }
    // Проверка, является ли адрес валидным для одного ППБ
    static bool isSinglePPBAddress(uint16_t address) {
        // Проверяем, что установлен ровно один бит
        return address && !(address & (address - 1));
    }

    // === БАЗОВЫЕ ЗАПРОСЫ ===

    // Создать базовый TU-запрос (все что после sign игнорируется, ставим 0)
    static QByteArray createTURequest(uint16_t address, TechCommand command);

    // Создать базовый FU-запрос
    static QByteArray createFURequest(uint16_t address, uint8_t period,
                                      const uint8_t fuData[3] = nullptr);

    // === КОНКРЕТНЫЕ КОМАНДЫ  ===

    // Команда TS (опрос состояния)
    static QByteArray createStatusRequest(uint16_t address);

    // Команда сброса (TC)
    static QByteArray createResetRequest(uint16_t address);

    // Запуск тестовой последовательности ПЕРЕДАЧИ (PRBS_M2S)
    static QByteArray createPRBSTransmitRequest(uint16_t address);

    // Запуск тестовой последовательности ПРИЕМА (PRBS_S2M)
    static QByteArray createPRBSReceiveRequest(uint16_t address);

    // Запрос отброшенных пакетов ФУ (DROP)
    static QByteArray createDroppedPacketsRequest(uint16_t address);

    // === ФУ КОМАНДЫ ===

    // ФУ передача (команда 0, период 0)
    static QByteArray createFUTransmitRequest(uint16_t address);

    // ФУ прием с периодом
    static QByteArray createFUReceiveRequest(uint16_t address, uint8_t period);

    // ФУ прием с данными
    static QByteArray createFUReceiveRequest(uint16_t address, uint8_t period,
                                             const uint8_t fuData[3]);

    // === ОБРАБОТКА ОТВЕТОВ ===

    // Распарсить ответ от ППБ (с CRC) - для TU команд
    static bool parsePPBResponse(const QByteArray& data, PPBResponse& response);

    // Распарсить ответ от бриджа (без CRC) - для FU команд
    static bool parseBridgeResponse(const QByteArray& data, BridgeResponse& response);

    // Распарсить OK-ответ СТАРЫЙ (4 байта)
    static bool parseOkResponse(const QByteArray& data, OkResponse& response);

    // Распарсить пакет данных (4 байта с CRC)
    static bool parseDataPacket(const QByteArray& data, DataPacket& packet);

    // Проверить CRC пакета данных
    static bool checkDataPacketCRC(const DataPacket& packet);



    // === ТЕСТОВЫЕ ПОСЛЕДОВАТЕЛЬНОСТИ ===

    // Создать пакет данных для тестовой последовательности (с CRC)
    static DataPacket createTestDataPacket(uint8_t data1, uint8_t data2, uint8_t data3);

    // Создать последовательность из 256 пакетов данных
    static QVector<DataPacket> createTestDataSequence();

    // Проверить последовательность пакетов (сравнить отправленные и полученные)
    static QString compareSequences(const QVector<DataPacket>& sent,
                                    const QVector<DataPacket>& received);

    // === УТИЛИТЫ ===

    // Проверить, является ли адрес широковещательным
    static bool isBroadcastAddress(uint16_t address) { return address == BROADCAST_ADDRESS; }

    // Получить размер базового запроса
    static constexpr size_t baseRequestSize() { return sizeof(BaseRequest); }

    // Получить размер OK-ответа
    static constexpr size_t okResponseSize() { return sizeof(OkResponse); }

    // Получить размер пакета данных
    static constexpr size_t dataPacketSize() { return sizeof(DataPacket); }

private:
    // Вспомогательный метод для создания пакета
    static QByteArray createRequest(uint16_t address, uint8_t command,
                                    Sign sign, uint8_t period = 0,
                                    const uint8_t fuData[3] = nullptr);
};

#endif // PACKETBUILDER_H
