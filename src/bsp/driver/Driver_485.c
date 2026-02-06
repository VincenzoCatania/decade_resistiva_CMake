#include "Driver_485.h"

// PA9 TX
// PA10 RX
// USART 1

const char MSG_HEADER_FROM_MASTER[HEADER_LEN] = {'T', 'X', 'T', 'M', 'S'};
const char MSG_HEADER_FROM_SLAVE[HEADER_LEN] = {
	'T',
	'X',
	'T',
	'S',
	'M',
};

static const char my_kind = 'C';  // C IS DECADE
static char byte = 0x0;

static ring_buffer_t buf = {0};
static void receive_buf(unsigned char byte);

UART_HandleTypeDef huart1;

static Driver_UART_Mem *self = NULL;
static void uart1_set_baudrate(uint32_t periph_clock, uint32_t baudrate);

static void MasterACK();

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
	return ch;
}

void Init_485(void)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	self = (Driver_UART_Mem *)malloc(sizeof(Driver_UART_Mem));
	memset(self, 0, sizeof(Driver_UART_Mem));

	self->state = WAIT_SOH;
	self->header_index = 0;
	self->data_index = 0;

	GPIO_InitTypeDef GPIO_InitStruct_TX = {0};
	GPIO_InitTypeDef GPIO_InitStruct_RX = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct_TX.Pin = GPIO_PIN_9;
	GPIO_InitStruct_TX.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct_TX.Pull = GPIO_NOPULL;
	GPIO_InitStruct_TX.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_TX.Alternate = GPIO_AF1_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_TX);

	GPIO_InitStruct_RX.Pin = GPIO_PIN_10;
	GPIO_InitStruct_RX.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct_RX.Pull = GPIO_NOPULL;
	GPIO_InitStruct_RX.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_RX.Alternate = GPIO_AF1_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_RX);

	__HAL_RCC_USART1_CLK_ENABLE();

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 28800;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

	HAL_UART_Init(&huart1);

	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	HAL_UART_Receive_IT(&huart1, &byte, 1); // Ricevi 1 byte in interrupt
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (self == NULL)
		return;

	if (huart->Instance == USART1)
	{
		receive_buf(byte);

		HAL_UART_Receive_IT(&huart1, &byte, 1);
	}
}

void manage_uart_buffer()
{

	unsigned char current_byte_to_parse;
	while (buf.pop_index != buf.push_index)
	{

		current_byte_to_parse = buf.byte[buf.pop_index++];
		if (buf.pop_index >= 400)
		{
			buf.pop_index = 0;
		}

		if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4))
		{
			return;
		}

		switch (self->state)
		{
		case WAIT_SOH:
			if (current_byte_to_parse == SOH)
			{
				self->header_index = 0;
				self->state = WAIT_HEADER;
			}
			break;

		case WAIT_HEADER:
			if (current_byte_to_parse == MSG_HEADER_FROM_MASTER[self->header_index])
			{
				self->header_index++;
				if (self->header_index == HEADER_LEN)
				{
					self->data_index = 0;
					self->state = WAIT_INCIPIT;
				}
			}
			else
			{
				self->header_index = 0;
				self->state = WAIT_SOH;
			}
			break;

		case WAIT_INCIPIT:
			if (current_byte_to_parse == 'A' || current_byte_to_parse == 'B')
			{
				self->state = READ_DATA;
			}
			else
			{
				self->state = WAIT_SOH;
			}
			break;

		case READ_DATA:
			self->read_buffer.bytes[self->data_index++] = current_byte_to_parse;
			if (self->data_index == DATA_LEN)
			{
				self->state = WAIT_EOT;
			}
			break;

		case WAIT_EOT:
			if (current_byte_to_parse == EOT)
			{
				self->requestReady = TRUE;
				MasterACK();
			}
			self->state = WAIT_SOH;
			break;

		default:
			self->state = WAIT_SOH;
			break;
		}
	}
}

static void MasterACK()
{
	if (!HAL_GPIO_ReadPin(GPIOC, 4))
	{
		return;
	}

	char ACK[228] = {0};

	ACK[0] = SOH;
	memcpy(&ACK[1], &MSG_HEADER_FROM_SLAVE, 5);
	ACK[6] = my_kind;
	memcpy(&ACK[7], &self->write_buffer, sizeof(data_buffer_t));
	ACK[227] = EOT;

	HAL_UART_Transmit(&huart1, (uint8_t *)ACK, 228, 500);
}

char* get_transmit_buffer()
{
	if (self == NULL)
		return;

	self->requestReady = FALSE;
	return (char *)&self->write_buffer;
}

char *get_receive_buffer()
{
	if (self == NULL)
		return;

	self->requestReady = FALSE;
	return (char *)&self->read_buffer;
}

boolean get_readyness()
{
	if (self == NULL)
		return;

	return self->requestReady;
}

static void receive_buf(unsigned char byte)
{
	buf.byte[buf.push_index++] = byte;
	if (buf.push_index >= 400)
	{
		buf.push_index = 0;
	}
}