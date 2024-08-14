#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length);

#endif /* CRC_H */
