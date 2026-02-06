#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "GeneralInclude.h"
#include <stdint.h>
#include <string.h>

#define SOH  0x01
#define EOT  0x04
#define HEADER_LEN 5
#define DATA_LEN_M 4
#define DATA_LEN 220


typedef enum 
{
    WAIT_SOH,
    WAIT_HEADER,
	WAIT_INCIPIT,
    READ_DATA,
    WAIT_EOT
}ParserState;


typedef struct 
{
	volatile unsigned char byte[400];
	volatile uint32_t push_index;
	volatile uint32_t pop_index;
}ring_buffer_t;


typedef struct  // do not modify order!!!!
{
	unsigned char relay_stat[208];
}protocol_receive_data_t;

typedef struct 
{
	unsigned char relay_stat[208];
}protocol_transmit_data_t;

typedef union 
{
	protocol_receive_data_t recv;
	protocol_transmit_data_t trsnmt;
}protocol_data_t;

typedef union 
{
	protocol_data_t str;
	unsigned char bytes[DATA_LEN];
}data_buffer_t;


typedef struct 
{
	uint8_t my_slot;
	uint8_t my_occurrence_number;

	ParserState state;
	uint8_t data_index;
	boolean requestReady;
	uint8_t header_index;
	data_buffer_t read_buffer;
	data_buffer_t write_buffer;
}Driver_UART_Mem;

void manage_uart_buffer();

void Init_485(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
boolean get_readyness();
char* get_receive_buffer();
char* get_transmit_buffer();

#endif // DRIVER_485_H
