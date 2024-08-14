#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include "usart.h"

#define MODBUS_ADDRESS 1
#define MODBUS_FUNCTION_READ 0x03

void modbus_process(uint8_t *rxBuffer, uint16_t *inputregister,  uint8_t *coils, UART_HandleTypeDef *huart);

#endif /* MODBUS_H */
