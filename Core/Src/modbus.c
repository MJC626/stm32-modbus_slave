#include "modbus.h"
#include "crc.h"
#include "usart.h"

#define MODBUS_ADDRESS 1         //从站地址
#define MODBUS_FUNCTION_READ_INPUT_REGISTERS 0x04//功能码
#define MODBUS_FUNCTION_WRITE_SINGLE_COIL 0x05//功能码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION       0x01// 非法功能码
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS   0x02//非法数据地址
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE      0x03//非法数据值

// 异常发送函数声明
void send_exception_response(UART_HandleTypeDef *huart, uint8_t function_code, uint8_t exception_code);

// Modbus 处理函数
void modbus_process(uint8_t *rxBuffer, uint16_t *inputregister, uint8_t *coils, UART_HandleTypeDef *huart) {
    uint16_t crc = modbus_crc16(rxBuffer, 6);
    if (crc == (rxBuffer[6] | (rxBuffer[7] << 8))) {
        uint8_t function_code = rxBuffer[1];
        uint8_t response[256];  // 回复的缓冲区

        switch (function_code) {
            case MODBUS_FUNCTION_READ_INPUT_REGISTERS: // 功能码 03
            {
                uint16_t start_address = (rxBuffer[2] << 8) | rxBuffer[3];
                uint16_t quantity = (rxBuffer[4] << 8) | rxBuffer[5];

                if (start_address + quantity <= 11) { // 地址不超过11
                    response[0] = MODBUS_ADDRESS;
                    response[1] = function_code;
                    response[2] = quantity * 2;
                    for (int i = 0; i < quantity; i++) {
                        response[3 + i * 2] = inputregister[start_address + i] >> 8;
                        response[4 + i * 2] = inputregister[start_address + i] & 0xFF;
                    }
                    uint16_t response_crc = modbus_crc16(response, 3 + quantity * 2);
                    response[3 + quantity * 2] = response_crc & 0xFF;
                    response[4 + quantity * 2] = (response_crc >> 8) & 0xFF;
                    HAL_UART_Transmit(huart, response, 5 + quantity * 2, HAL_MAX_DELAY);
                } else {
                    send_exception_response(huart, function_code, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
                }
            }
                break;

            case MODBUS_FUNCTION_WRITE_SINGLE_COIL: // 功能码 05
            {
                uint16_t coil_address = (rxBuffer[2] << 8) | rxBuffer[3];
                uint16_t coil_value = (rxBuffer[4] << 8) | rxBuffer[5];

                if (coil_address < 3) {
                    // 更新线圈数组中的值
                    if (coil_value == 0xFF00) {
                        coils[coil_address] = 1;  // 开
                    } else if (coil_value == 0x0000) {
                        coils[coil_address] = 0;  // 关
                    } else {
                        // 如果 coil_value 不是 0xFF00 或 0x0000，发送异常响应并返回
                        send_exception_response(huart, function_code, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
                        return;
                    }
                    response[0] = MODBUS_ADDRESS;
                    response[1] = function_code;
                    response[2] = rxBuffer[2];
                    response[3] = rxBuffer[3];
                    response[4] = rxBuffer[4];
                    response[5] = rxBuffer[5];
                    uint16_t response_crc = modbus_crc16(response, 6);
                    response[6] = response_crc & 0xFF;
                    response[7] = (response_crc >> 8) & 0xFF;
                    HAL_UART_Transmit(huart, response, 8, HAL_MAX_DELAY);
                } else {
                    send_exception_response(huart, function_code, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
                }
            }
                break;

            default:
                // 不支持的功能吗
                send_exception_response(huart, function_code, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
                break;
        }
    }
}

// 发送异常响应的函数
void send_exception_response(UART_HandleTypeDef *huart, uint8_t function_code, uint8_t exception_code) {
    uint8_t exception_response[5];
    exception_response[0] = MODBUS_ADDRESS;
    exception_response[1] = function_code | 0x80; // Exception response
    exception_response[2] = exception_code;
    uint16_t exc_crc = modbus_crc16(exception_response, 3);
    exception_response[3] = exc_crc & 0xFF;
    exception_response[4] = (exc_crc >> 8) & 0xFF;
    HAL_UART_Transmit(huart, exception_response, 5, HAL_MAX_DELAY);
}

