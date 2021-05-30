#include "utils.h"

/*___________________________________________________________________________*/

static const char caniot_msg_types[] PROGMEM = {
    "Command\0"
    "Telemetry\0"
    "Write-Attribute\0"
    "Read-Attribute\0"
    "\0" // two EOS to tell "print_prog_string" its the end of the array
};

static const char caniot_msg_query_resp[] PROGMEM = {
    "Query\0"
    "Response\0"
    "\0"
};

static const char caniot_msg_controllers[] PROGMEM = {
    "Main Controller (0)\0"
    "Controller (1)\0"
    "Controller (2)\0"
    "BROADCAST\0"
    "\0"
};

static const char caniot_msg_data_frame_types[] PROGMEM = {
    "U\0"
    "CR\0"
    "CRA\0"
    "CRT\0"
    "CRTTA\0"
    "CRTRAAA\0"
    "TTTT\0"
    ".\0"
    "\0"
};

void print_prog_string(PGM_P const pgm_string_array, const uint8_t elem)
{
    uint8_t index = 0;
    PGM_P pos = pgm_string_array;

    while(true)
    {
        if (index == elem)
        {
            usart_print_p(pos);
            return;
        }
        else
        {
            const size_t len = strlen_P(pos);

            if (len == 0)
            {
                return;
            }
            
            pos += len + 1;
            index++;
        }
    }
}



/*___________________________________________________________________________*/

static const char msg_can_recv_from[] PROGMEM = "CAN message received from ";
static const char msg_can_send_between[] PROGMEM = " CAN message send between ";


void print_can(const unsigned long id, const uint8_t * const buffer, const uint8_t len)
{
    usart_print_p(msg_can_recv_from);

    usart_hex16(id);

    usart_print(" : ");

    for(uint_fast8_t i = 0; i < len; i++)
    {
        usart_hex(buffer[i]);
        usart_transmit(' ');
    }
    usart_transmit('\n');
}

void print_can_expl(can_id_t id, const uint8_t * const buffer, const uint8_t len)
{
    print_prog_string(caniot_msg_query_resp, id.bitfields.query);

    usart_transmit(' ');

    print_prog_string(caniot_msg_types, id.bitfields.type);

    usart_print_p(msg_can_send_between);

    print_prog_string(caniot_msg_controllers, id.bitfields.controller);

    if (id.bitfields.id == DEVICE_BROADCAST)
    {
        usart_print(" and BROADCAST : ");
    }
    else
    {
        usart_print(" to device D");
        usart_u8(id.bitfields.id);
        usart_print(" : ");
        print_prog_string(caniot_msg_data_frame_types, id.bitfields.id >> 3);
        usart_transmit(' ');
        usart_u8(id.bitfields.id & 0x7);
        usart_transmit(' ');
    }

    for (uint_fast8_t i = 0; i < len; i++)
    {
        usart_hex(buffer[i]);
        usart_transmit(' ');
    }
    
    usart_transmit('\n');
}

/*___________________________________________________________________________*/