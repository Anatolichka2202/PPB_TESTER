#ifndef CRC_H
#define CRC_H

#include <cstdint>

extern const unsigned char Crc8Table[256];

uint8_t calculateCRC8(const uint8_t *data, uint8_t len);

#endif // CRC_H
