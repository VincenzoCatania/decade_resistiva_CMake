#include "Driver_analog.h"
#include "Driver_485.h"
static void OutputHigh(GPIO_TypeDef *port, uint32_t pin);
static void OutputLow(GPIO_TypeDef *port, uint32_t pin);
static void select_channel_mux(uint8_t channel);

static void populate_values(uint32_t *to_mean, uint32_t *to_max,
                            uint32_t *to_min);

static driver_analog_mem_t *self = NULL;

extern ADC_HandleTypeDef hadc;

static uint32_t adc_channels[] =
    {
        ADC_CHANNEL_3,
        ADC_CHANNEL_4,
        ADC_CHANNEL_5,
        ADC_CHANNEL_6};

static uint32_t sample_period_table[5][2] =
    {
        {T_10_SAMPLE, 5},
        {T_20_SAMPLE, 10},
        {T_30_SAMPLE, 15},
        {T_40_SAMPLE, 20},
        {T_50_SAMPLE, 30},
};

static uint32_t sample_numbers_table[5][2] =
    {
        {T_10_MSEC, 1},
        {T_20_MSEC, 2},
        {T_30_MSEC, 5},
        {T_40_MSEC, 10},
        {T_50_MSEC, 20},
};

static void read_ADC();

void init_driver_analog(void)
{

  // GPIO
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // calibrazione
  if (ADC1->CR & ADC_CR_ADEN)
  {
    ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN)
      ;
  }

  ADC1->CR |= ADC_CR_ADCAL;
  while (ADC1->CR & ADC_CR_ADCAL)
    ; // attesa fine calibrazione
  ADC1->CR |= ADC_CR_ADEN;

  ADC1->CHSELR = ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6;
  ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;

  // ADC->CCR |= ADC_CCR_VREFEN;

  ADC1->CR |= ADC_CR_ADEN;                 /* (3) */
  while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) /* (4) */
  {
  }

  self = (driver_analog_mem_t *)malloc(sizeof(driver_analog_mem_t));
  memset(self, 0, sizeof(driver_analog_mem_t));
  memset(self->zeros_on_amp, 0, 8*sizeof(uint32_t));
  self->mux_status.byte = 0;
  self->analog_poll_status.lastRawReadCh3_Mux[0] = 0;
  self->discard = 0U;
  self->current_num_samples = 'A';
  self->current_sample_period = 'A';
}

void set_analyze_param(sel_sample_period_t period, sel_sample_number_t n_samples)
{

  self->current_num_samples = n_samples;
  self->current_sample_period = period;
  self->internal_counter = 0;
  self->acquired_samples = 0;
}

void manage_analog_module()
{
  uint16_t cur_sample_period = 0;
  uint16_t cur_number_samples = 0;
  for (int i = 0; i < 5; ++i)
  {
    if (sample_period_table[i][0] == self->current_sample_period)
    {
      cur_sample_period = sample_period_table[i][1];
    }

    if (sample_numbers_table[i][0] == self->current_num_samples)
    {
      cur_number_samples = sample_numbers_table[i][1];
    }
  }

  if (cur_sample_period > 0 &&
      cur_number_samples > 0)
  {
    ++self->internal_counter;
    if (self->internal_counter >= cur_sample_period) // acquire one sample
    {
      self->internal_counter = 0;
      ++self->acquired_samples;

      read_ADC();
    }
  }

}

static void populate_values(uint32_t *to_mean, uint32_t *to_max,
                            uint32_t *to_min)
{
  // Ciclo su tutti i 16 canali
  for (int i = 0; i < 16; i++)
  {
    uint32_t val;

    // --- Mean ---
    val = self->AC3_mean_Mux[i];
    // Inversione byte: ABCD -> DCBA
    to_mean[i] = ((val & 0x000000FF) << 24) | // D -> A
                 ((val & 0x0000FF00) << 8) |  // C -> B
                 ((val & 0x00FF0000) >> 8) |  // B -> C
                 ((val & 0xFF000000) >> 24);  // A -> D

    // --- Min ---
    val = self->AC3_min_Mux[i];
    to_min[i] = ((val & 0x000000FF) << 24) |
                ((val & 0x0000FF00) << 8) |
                ((val & 0x00FF0000) >> 8) |
                ((val & 0xFF000000) >> 24);

    // --- Max ---
    val = self->AC3_max_Mux[i];
    to_max[i] = ((val & 0x000000FF) << 24) |
                ((val & 0x0000FF00) << 8) |
                ((val & 0x00FF0000) >> 8) |
                ((val & 0xFF000000) >> 24);
  }
}

static void select_channel_mux(uint8_t channel)
{

  if (channel > 15)
  {
    return; // 4-bit max
  }

  self->mux_status.byte = channel;

  (channel & 0x01) ? OutputHigh(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_0) : OutputLow(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_0);

  (channel & 0x02) ? OutputHigh(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_1) : OutputLow(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_1);

  (channel & 0x04) ? OutputHigh(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_2) : OutputLow(MUX_ADDRESS_PORT_OTHERPINS, MUX_ADDRESS_PIN_2);

  (channel & 0x08) ? OutputHigh(MUX_ADDRESS_PORT_PIN3, MUX_ADDRESS_PIN_3_C) : OutputLow(MUX_ADDRESS_PORT_PIN3, MUX_ADDRESS_PIN_3_C);
}

static void read_ADC()
{
  uint32 cnt = 0;
  /* Performs the AD conversion */
  ADC1->CR |= ADC_CR_ADSTART; /* Start the ADC conversion */
  for (int i = 0; i < 4; i++)
  {
    while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* Wait end of conversion */
    {
      ++cnt;
      if (cnt >= 100)
      {
        cnt = 0;
        break;
      }
    }
    uint16_t val = (uint16_t)ADC1->DR;
    if (!self->discard)
    {
      uint32_t unconverted = (uint32_t)(((float)val / 4095.0f) * 3300.0f);
      if (i == 0)
      {
        if(self->zeros_on_amp[self->mux_status.byte] == 0 && unconverted >= 2200 && unconverted <= 2600)
        {
          self->zeros_on_amp[self->mux_status.byte] = unconverted;
        }
        if(self->mux_status.byte < 8 && self->zeros_on_amp[self->mux_status.byte] != 0) // CONVERSIONE 8 CANALI AMPEROMETRICHE
        {
          if(unconverted >= self->zeros_on_amp[self->mux_status.byte])
          {
            unconverted -= self->zeros_on_amp[self->mux_status.byte];
            unconverted *= 10;
          }
          else
          {
            unconverted = 0;
          }
        }
        else // CONVERSIONE 8 CANALI VOLTMETRICHE 
        {
          unconverted *= 50;
        }
        self->analog_poll_status.lastRawReadCh3_Mux[self->mux_status.byte] = unconverted; /* Store the ADC conversion result */
      }
      if (i == 1)
      {
        self->analog_poll_status.lastRawReadCh4_VddCheck = unconverted; /* Store the ADC conversion result */
      }
      if (i == 2)
      {
        self->analog_poll_status.lastRawReadCh5_Vref = unconverted; /* Store the ADC conversion result */
      }
      if (i == 3)
      {
        self->analog_poll_status.lastRawReadCh6_VcoilCheck = unconverted; /* Store the ADC conversion result */
      }
    }
  }

  while (ADC1->ISR & ADC_ISR_EOS == 0)
  {
    ++cnt;
    if (cnt >= 100)
    {
      cnt = 0;
      break;
    }
  }
  ADC1->ISR |= ADC_ISR_EOS;

  ADC1->CFGR1 ^= ADC_CFGR1_SCANDIR; /* Toggle the scan direction */
  self->discard = (ADC1->CFGR1 & ADC_CFGR1_SCANDIR);

  return self->analog_poll_status;
}

static void OutputHigh(GPIO_TypeDef *port, uint32_t pin)
{
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}
static void OutputLow(GPIO_TypeDef *port, uint32_t pin)
{
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}