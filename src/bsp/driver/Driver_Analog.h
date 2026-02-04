#ifndef DRIVER_ANALOG_H
#define DRIVER_ANALOG_H
#include "generalInclude.h"

#define CHANNEL_NUMBER 4

#define MUX_ADDRESS_PIN_0 (1 << 0)
#define MUX_ADDRESS_PIN_1 (1 << 1)
#define MUX_ADDRESS_PIN_2 (1 << 2)
#define MUX_ADDRESS_PORT_OTHERPINS GPIOB

#define MUX_ADDRESS_PIN_3_C (1 << 5)
#define MUX_ADDRESS_PORT_PIN3 GPIOC

#define MUX_ENABLE_PIN (1 << 10)
#define MUX_ENABLE_PORT GPIOB




typedef union
{
    struct bits
    {
        uint8_t ADDRES_0 : 1;
        uint8_t ADDRES_1 : 1;
        uint8_t ADDRES_2 : 1;
        uint8_t ADDRES_3 : 1;
    };
    uint8_t byte;

}mux_status_t;


typedef enum 
{
    T_10_MSEC = 'A',
    T_20_MSEC = 'B',
    T_30_MSEC = 'C',
    T_40_MSEC = 'D',
    T_50_MSEC = 'E'
}sel_sample_period_t;

typedef enum 
{
    T_10_SAMPLE = 'A',
    T_20_SAMPLE = 'B',
    T_30_SAMPLE = 'C',
    T_40_SAMPLE = 'D',
    T_50_SAMPLE = 'E'
}sel_sample_number_t;


typedef struct
{
    uint32_t lastRawReadCh3_Mux[16];
    uint32_t lastRawReadCh4_VddCheck;
    uint32_t lastRawReadCh5_Vref;
    uint32_t lastRawReadCh6_VcoilCheck;
}analog_poll_t;



typedef struct 
{
    analog_poll_t analog_poll_status;

    uint32_t AC3_mean_Mux[16]; 
    uint32_t AC3_max_Mux[16]; 
    uint32_t AC3_min_Mux[16]; 

    uint32_t AC4_mean_VddCheck;
    uint32_t AC5_mean_Vref;
    uint32_t AC6_mean_VcoilCheck;


    uint8_t acquired_samples;
    sel_sample_number_t current_num_samples;
    sel_sample_period_t current_sample_period;
    uint32_t internal_counter;
    mux_status_t mux_status;
    uint32_t zeros_on_amp[8];
    boolean discard;
}driver_analog_mem_t;

void set_analyze_param(sel_sample_period_t period, sel_sample_number_t n_samples);
void init_driver_analog(void);




#endif // DRIVER_ANALOG_H