#include "Driver_SPI.h"
#include <stdlib.h>
#include <string.h>

/* ======================= DEFINES ======================= */

#define NUM_IC 13
#define TOTAL_CHANNELS (NUM_IC * 16)

/* ======================= GLOBALS ======================= */

spi_device_mem_t *self = NULL;
SPI_HandleTypeDef hspi2;

static boolean relay_old[TOTAL_CHANNELS] = {0};
static uint16_t status_ic[NUM_IC] = {0};

/* ======================= CS MAP ======================= */

static uint32_t cs_pins[NUM_IC][2] =
{
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

/* ======================= PROTOTYPES ======================= */

static void deselect_all_ic(void);
static void CS_Select(uint8_t which_ic);
static void mc33996_send_cmd(uint8_t cmd, uint16_t data, uint8_t which_ic);

/* ======================= INIT ======================= */

void init_spi_device(void)
{
    self = (spi_device_mem_t *)malloc(sizeof(spi_device_mem_t));

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO CLOCK ENABLE */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* ================= RESET PIN ================= */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ================= CS GPIO INIT ================= */

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    /* GPIOA */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* GPIOB */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* GPIOC */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
                          GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* GPIOF */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* Tutte le CS inattive */
    deselect_all_ic();

    /* ================= SPI2 GPIO ================= */

    __HAL_RCC_SPI2_CLK_ENABLE();

    GPIO_InitStruct.Pin       = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ================= SPI2 INIT ================= */

    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial     = 7;
    hspi2.Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    hspi2.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;

    if (HAL_SPI_Init(&hspi2) != HAL_OK)
        Error_Handler();

    /* ================= UNRESET DEVICE ================= */

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_Delay(10);

    /* ================= MC33996 INIT ================= */

    for (uint8_t ic = 0; ic < NUM_IC; ic++)
    {
        mc33996_send_cmd(CMD_RESET, 0x0000, ic);
        mc33996_send_cmd(CMD_ON_OFF_CONTROL, 0x0000, ic);
    }
  
}

/* ======================= RELAY CONTROL ======================= */

void set_relay_on(uint8_t wich_bit, boolean stat, uint8_t wich_ic)
{
    if (wich_ic >= NUM_IC || wich_bit >= TOTAL_CHANNELS)
        return;

    if (relay_old[wich_bit] == stat)
        return;

    relay_old[wich_bit] = stat;

    uint8_t bit_pos = wich_bit - (16 * wich_ic);
    if (bit_pos >= 16)
        return;

    if (stat)
        status_ic[wich_ic] |=  (1 << bit_pos);
    else
        status_ic[wich_ic] &= ~(1 << bit_pos);

    uint8_t buf[3];
    buf[0] = CMD_ON_OFF_CONTROL;
    buf[1] = (status_ic[wich_ic] >> 8) & 0xFF;
    buf[2] = status_ic[wich_ic] & 0xFF;


    CS_Select(wich_ic);
    HAL_Delay(1);

    HAL_SPI_Transmit(&hspi2, buf, 3, 100);

    HAL_Delay(1);
    deselect_all_ic();
    HAL_Delay(1);
}

/* ======================= LOW LEVEL ======================= */

static void deselect_all_ic(void)
{
    for (uint8_t i = 0; i < NUM_IC; i++)
        HAL_GPIO_WritePin(cs_pins[i][0], cs_pins[i][1], GPIO_PIN_SET);
}

static void CS_Select(uint8_t which_ic)
{
    deselect_all_ic();

    if (which_ic < NUM_IC)
        HAL_GPIO_WritePin(cs_pins[which_ic][0],
                          cs_pins[which_ic][1],
                          GPIO_PIN_RESET);
}

static void mc33996_send_cmd(uint8_t cmd, uint16_t data, uint8_t which_ic)
{
    if (which_ic >= NUM_IC)
        return;

    uint8_t buf[3];
    buf[0] = cmd;
    buf[1] = (data >> 8) & 0xFF;
    buf[2] = data & 0xFF;

    CS_Select(which_ic);
    HAL_Delay(1);
    HAL_SPI_Transmit(&hspi2, buf, 3, 100);
    HAL_Delay(1);
}
