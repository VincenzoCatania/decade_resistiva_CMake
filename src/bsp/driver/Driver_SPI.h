#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H
#include "generalInclude.h"


#define CMD_ON_OFF_CONTROL           0b00000000
#define CMD_OPEN_LOAD_CURRENT_ENABLE 0b00000100
#define CMD_GLOBAL_SHUTDOWN_RETRY_CONTROL 00001000
#define CMD_SFPD_CONTROL_BASE 00001100
#define CMD_PWM_ENABLE 0b00010000
#define CMD_AND_OR_CTRL 0b00010100
#define CMD_RESET 0b00011000
#define CMD_SO_RESPONSE 0b00000000

#define SET_0_TO_7_RELAY0 0b00000001
#define SET_0_TO_7_RELAY1 0b00000010
#define SET_0_TO_7_RELAY2 0b00000100
#define SET_0_TO_7_RELAY3 0b00001000
#define SET_0_TO_7_RELAY4 0b00010000
#define SET_0_TO_7_RELAY5 0b00100000
#define SET_0_TO_7_RELAY6 0b01000000
#define SET_0_TO_7_RELAY7 0b10000000

#define SET_0_TO_7_RELAY8  0b00000001
#define SET_0_TO_7_RELAY9  0b00000010
#define SET_0_TO_7_RELAY10 0b00000100
#define SET_0_TO_7_RELAY11 0b00001000
#define SET_0_TO_7_RELAY12 0b00010000
#define SET_0_TO_7_RELAY13 0b00100000
#define SET_0_TO_7_RELAY14 0b01000000
#define SET_0_TO_7_RELAY15 0b10000000

// 1. Abbasso CS
// 2. Mando primo byte comando on off e poi i 16 bit a seguire la configurazione 
//    delle uscite, 1 chiude a massa l'uscita
// 3. Alzo CS e il driver effettua cosa ho chiesto 

// PB12 NRST
// PB13 CLK
// PB14 MISO
// PB15 MOSI

// PC6 CS4
// PC7 CS3
// PC8 CS2
// PC9 CS1


typedef struct 
{
    uint8_t bytes[2];
}relay_status_t;

typedef struct 
{
    relay_status_t relays[4];
    uint8_t cs_status[4];
}spi_device_mem_t;


void init_spi_device();

void set_relay_on(uint8_t wich_bit, boolean stat, uint8_t wich_ic);




#endif // DRIVER_SPI_H