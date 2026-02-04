#include "board_manager.h"
#include "Driver_485.h"
#include "Driver_Analog.h"
#include "Driver_SPI.h"
#include "generalInclude.h"


void manage_incoming_messages()
{
    if(get_readyness())
    {
        data_buffer_t* receive_buf = get_receive_buffer();
        data_buffer_t* trasmit_buf = get_transmit_buffer();
        for(int i = 0; i < 4; ++i)
        {
            for(int j = 0; j < 16; j++)
            {
                uint8_t index = (i*16)+j;

                if(index <= 51)
                {
                    set_relay_on(index,*(boolean*)(&receive_buf->str.recv.relay_stat[index]), i);
                }
            }
        }                                                       
    }
}