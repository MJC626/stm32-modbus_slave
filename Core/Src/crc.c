#include "crc.h"

uint16_t modbus_crc16(uint8_t *buffer, uint16_t buffer_length) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < buffer_length; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
