#include "misc.h"

#include "message.h"

/*___________________________________________________________________________*/

static const char caniot_msg_types[] PROGMEM = {
    "Command\0"
    "Telemetry\0"
    "Write-Attribute\0"
    "Read-Attribute\0"
    "\0" /* two EOS to tell "print_prog_string" its the end of the array */
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
    "CRTAAA\0"
    "TTTT\0"
    ".\0"
    "\0"
};

void print_prog_string(PGM_P const pgm_string_array, const uint8_t elem)
{
    uint8_t index = 0;
    PGM_P pos = pgm_string_array;

    while (true) {
        if (index == elem) {
            usart_print_p(pos);
            return;
        } else {
            const size_t len = strlen_P(pos);

            if (len == 0) {
                return;
            }

            pos += len + 1;
            index++;
        }
    }
}

void print_prog_type(const type_t type)
{
    print_prog_string(caniot_msg_types, (uint8_t)type);
}

void print_prog_query(const query_t qr)
{
    print_prog_string(caniot_msg_query_resp, (uint8_t)qr);
}

void print_prog_data_type(const data_type_t dt)
{
    print_prog_string(caniot_msg_data_frame_types, (uint8_t)dt);
}

void print_prog_controller(const controller_t ctrl)
{
    print_prog_string(caniot_msg_controllers, (uint8_t)ctrl);
}

static const char msg_can_recv_from[] PROGMEM = "CAN message received from ";
static const char msg_can_send_between[] PROGMEM = " CAN message send between ";


void print_can(const unsigned long id, const uint8_t* const buffer, const uint8_t len)
{
    usart_print_p(msg_can_recv_from);
    usart_hex16(id);
    usart_print(" : ");
    for (uint_fast8_t i = 0; i < len; i++) {
        usart_hex(buffer[i]);
        usart_transmit(' ');
    }
    usart_transmit('\n');
}

void print_can_expl(Message& can_msg)
{
    print_can_expl(can_msg.id, can_msg.buffer, can_msg.len);
}

void print_can_expl(can_id_t id, const uint8_t* const buffer, const uint8_t len)
{
    static const char error_frame_msg[] PROGMEM = "Error frame : error = 0x";
    static const char and_broadcast_msg[] PROGMEM = " and BROADCAST : ";
    static const char and_device_msg[] PROGMEM = " and device D";

    usart_transmit('[');
    usart_hex16(id.value);
    usart_print("] ");

    if (id.is_error()) {
        usart_print_p(error_frame_msg);
        usart_hex(buffer[0]);
    } else {
        print_prog_query((query_t)id.bitfields.query);
        usart_transmit(' ');
        print_prog_type((type_t)id.bitfields.type);
        usart_print_p(msg_can_send_between);
        print_prog_controller((controller_t)id.bitfields.controller);

        if (id.is_broadcast()) {
            usart_print_p(and_broadcast_msg);
        } else {
            usart_print_p(and_device_msg);
            usart_u8(id.bitfields.device_id);
            usart_transmit(' ');
            print_prog_data_type((data_type_t)id.bitfields.device_type);
            usart_print(" : ");
        }

        for (uint_fast8_t i = 0; i < len; i++) {
            usart_hex(buffer[i]);
            usart_transmit(' ');
        }
    }
    usart_transmit('\n');
}

void print_time_sec(uint32_t time_sec)
{
    static const char uptime_msg[] = "uptime : ";
    static const char seconds_msg[] = " seconds";
    usart_print_p(uptime_msg);
    usart_u16(time_sec);
    usart_print_p(seconds_msg);
}

/*___________________________________________________________________________*/

void debug_masks_filters(void)
{
    static const char msg[] PROGMEM = "DEVICE_RX";

#define PRINT_PGM_DEVICE_RX_MSG() usart_print_p(msg)

    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("M0 ");
    usart_hex16(DEVICE_RXM0);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F0 ");
    usart_hex16(DEVICE_RXF0);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F1 ");
    usart_hex16(DEVICE_RXF1);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("M1 ");
    usart_hex16(DEVICE_RXM1);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F2 ");
    usart_hex16(DEVICE_RXF2);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F3 ");
    usart_hex16(DEVICE_RXF3);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F4 ");
    usart_hex16(DEVICE_RXF4);
    usart_transmit('\n');
    PRINT_PGM_DEVICE_RX_MSG();
    usart_print("F5 ");
    usart_hex16(DEVICE_RXF5);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/

void print_attr_ref(attr_ref_t* attr_ref_p)
{
    usart_print("attr_ref 0x");
    usart_hex16((uint16_t) attr_ref_p);
    usart_print(" section=");
    usart_u8(attr_ref_p->section);
    usart_print(" options=");
    usart_u8(attr_ref_p->options);
    usart_print(" offset=");
    usart_u8(attr_ref_p->offset);
    usart_print(" size=");
    usart_u8(attr_ref_p->read_size);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/

#define CANIOT_ERROR_STR(err, name) {err, name}
static const struct {
    uint8_t err;
    char name[sizeof("EKEYSECTION")];
} errors_str[] PROGMEM = {
    CANIOT_ERROR_STR(CANIOT_OK, "OK"),
    CANIOT_ERROR_STR(CANIOT_ENPROC, "ENPROC"),
    CANIOT_ERROR_STR(CANIOT_ECMD, "ECMD"),
    CANIOT_ERROR_STR(CANIOT_EKEY, "EKEY"),
    CANIOT_ERROR_STR(CANIOT_ETIMEOUT, "ETIMEOUT"),
    CANIOT_ERROR_STR(CANIOT_EBUSY, "EBUSY"),
    CANIOT_ERROR_STR(CANIOT_EFMT, "EFMT"),
    CANIOT_ERROR_STR(CANIOT_EHANDLERC, "EHANDLERC"),
    CANIOT_ERROR_STR(CANIOT_EHANDLERT, "EHANDLERT"),
    CANIOT_ERROR_STR(CANIOT_ETELEMETRY, "ETELEMETRY"),
    CANIOT_ERROR_STR(CANIOT_ENOINIT, "ENOINIT"),
    CANIOT_ERROR_STR(CANIOT_EDRIVER, "EDRIVER"),
    CANIOT_ERROR_STR(CANIOT_EKEYSECTION, "EKEYSECTION"),
    CANIOT_ERROR_STR(CANIOT_EKEYATTR, "EKEYATTR"),
    CANIOT_ERROR_STR(CANIOT_EKEYPART, "EKEYPART"),
    CANIOT_ERROR_STR(CANIOT_EREADONLY, "EREADONLY"),
    CANIOT_ERROR_STR(CANIOT_ENULL, "ENULL"),
    CANIOT_ERROR_STR(CANIOT_EREADATTR, "EREADATTR"),
    CANIOT_ERROR_STR(CANIOT_EWRITEATTR, "EWRITEATTR"),
    CANIOT_ERROR_STR(CANIOT_EENOCB, "EENOCB"),
    CANIOT_ERROR_STR(CANIOT_EECB, "EECB"),
    CANIOT_ERROR_STR(CANIOT_ENIMPL, "ENIMPL"),
    CANIOT_ERROR_STR(CANIOT_EUNDEF, "EUNDEF"),
};

void print_caniot_error(uint8_t err)
{
    for (uint_fast8_t i = 0; i < ARRAY_SIZE(errors_str); i++) {
        if ((uint8_t) pgm_read_byte(&errors_str[i].err) == err) {
            PRINT_PROGMEM_STRING(caniot_str, "CANIOT_");
            usart_print_p(errors_str[i].name);
            break;
        }
    }
}

/*___________________________________________________________________________*/