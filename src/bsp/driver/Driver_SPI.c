#include "Driver_SPI.h"

spi_device_mem_t *self = NULL;

SPI_HandleTypeDef hspi2;

static void deselect_all_ic(void);
static void mc33996_send_cmd(uint8_t cmd, uint16_t data, uint8_t wich_ic);
static void CS_Select(uint8_t which_ic);

static boolean relay_old[208] = {0};
static uint16_t status_ic[13] = {0};
typedef struct 
{
    GPIO_TypeDef *port;
    uint16_t pin;
} CS_Pin_t;

CS_Pin_t cs_pins[] = {
    {GPIOC, GPIO_PIN_6},   // CS1
    {GPIOC, GPIO_PIN_7},   // CS2
    {GPIOC, GPIO_PIN_8},   // CS3
    {GPIOC, GPIO_PIN_9},   // CS4
    {GPIOA, GPIO_PIN_8},   // CS5
    {GPIOA, GPIO_PIN_11},  // CS6
    {GPIOF, GPIO_PIN_6},   // CS7
    {GPIOA, GPIO_PIN_15},  // CS8
    {GPIOC, GPIO_PIN_11},  // CS9
    {GPIOC, GPIO_PIN_12},  // CS10
    {GPIOB, GPIO_PIN_3},   // CS11
    {GPIOB, GPIO_PIN_4},   // CS12
    {GPIOB, GPIO_PIN_6}    // CS13
};


void init_spi_device()
{
  self = (spi_device_mem_t *)malloc(sizeof(spi_device_mem_t));

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  // CS_1  -> PC6
  // CS_2  -> PC7
  // CS_3  -> PC8
  // CS_4  -> PC9
  // CS_5  -> PA8
  // CS_6  -> PA11
  // CS_7  -> PF6
  // CS_8  -> PA15
  // CS_9  -> PC11
  // CS_10 -> PC12
  // CS_11 -> PB3
  // CS_12 -> PB4
  // CS_13 -> PB6

  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);

  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, SET);


  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9
                        | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, SET);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, SET);

  __HAL_RCC_SPI2_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /**SPI2 GPIO Configuration
  PB13     ------> SPI2_SCK
  PB14     ------> SPI2_MISO
  PB15     ------> SPI2_MOSI
  */
  GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* SPI2 interrupt Init */
  HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SPI2_IRQn);
  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

  // UNRESET THE DEVICE
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_Delay(10);

  for (uint8_t ic = 0; ic < 13; ic++)
  {
    // 1. RESET software (pulizia registri SPI)
    mc33996_send_cmd(CMD_RESET, 0x0000, ic);

    // 2. Disabilita tutte le uscite (stato noto)
    mc33996_send_cmd(CMD_ON_OFF_CONTROL, 0x0000, ic);

    // 3. Abilita Open Load Detect su tutte le uscite
    //mc33996_send_cmd(CMD_OPEN_LOAD_CURRENT_ENABLE, 0xFFFF, ic);

    // 4. Global Shutdown / Retry (valore safe: retry ON)
    //mc33996_send_cmd(CMD_GLOBAL_SHUTDOWN_RETRY_CONTROL, 0x0001, ic);

    // 5. Disabilita PWM (tutto in ON/OFF puro)
    //mc33996_send_cmd(CMD_PWM_ENABLE, 0x0000, ic);

    // 6. AND/OR logic default (ON/OFF diretto)
    //mc33996_send_cmd(CMD_AND_OR_CTRL, 0x0000, ic);
  }
}

void set_relay_on(uint8_t wich_bit, boolean stat, uint8_t wich_ic)
{
  if (relay_old[wich_bit] != stat)
  {
    relay_old[wich_bit] = stat;
  }
  else
  {
    return;
  }
  unsigned char buf[3];

  buf[0] = CMD_ON_OFF_CONTROL;

  CS_Select(wich_ic);  // CSx


  // 0 -> 51 (52 tot)
  uint16_t val = 1 << (wich_bit - (16 * wich_ic));

  if (stat)
    status_ic[wich_ic] |= 1 << (wich_bit - (16 * wich_ic));
  else
    status_ic[wich_ic] &= ~(1 << (wich_bit - (16 * wich_ic)));

  memcpy(&buf[1], &status_ic[wich_ic], 2);

  HAL_Delay(1);
  HAL_SPI_Transmit(&hspi2, buf, 3U, 100U);
  HAL_Delay(1);
  deselect_all_ic();
  HAL_Delay(1);
}

static void deselect_all_ic()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, SET);
}

static void mc33996_send_cmd(uint8_t cmd, uint16_t data, uint8_t wich_ic)
{
  uint8_t buf[3];

  buf[0] = cmd;
  buf[1] = (data >> 8) & 0xFF;
  buf[2] = data & 0xFF;

  // seleziona IC
  deselect_all_ic();
  HAL_Delay(1);

  switch (wich_ic)
  {
  case 0:
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, RESET);
    break;
  case 1:
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET);
    break;
  case 2:
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, RESET);
    break;
  case 3:
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, RESET);
    break;
  }

  HAL_Delay(1);
  HAL_SPI_Transmit(&hspi2, buf, 3, 100);
  HAL_Delay(1);
  deselect_all_ic();
  HAL_Delay(1);
}


static void CS_Select(uint8_t which_ic)
{
    // Metti tutte le CS inattive
    for (uint8_t i = 0; i < 13; i++)
    {
        HAL_GPIO_WritePin(cs_pins[i].port, cs_pins[i].pin, GPIO_PIN_SET);
    }

    // Attiva solo la CS richiesta
    if (which_ic < 13)
    {
        HAL_GPIO_WritePin(cs_pins[which_ic].port,
                          cs_pins[which_ic].pin,
                          GPIO_PIN_RESET);
    }
}
