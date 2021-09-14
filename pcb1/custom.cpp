#include "custom.h"
#include "tcn75.h"

/*___________________________________________________________________________*/

static volatile uint8_t ins = 0;
static volatile bool ins_change = false;

ISR(PCINT2_vect)
{
    ins = CustomBoard::read_inputs();
    ins_change = true;
}

void CustomBoard::io_init(void)
{
    /* relays PC2, PC3 */
    DDRC |= (1 << DDC2) | (1 << DDC3);
    PORTC &= ~((1 << PORTC2) | (1 << PORTC3));

    /* inputs PD3, PD4, PD5, PD6 */
    DDRD &= ~((1 << DDD3) | (1 << DDD4)
        | (1 << DDD5) | (1 << DDD6));

    /* disable pull-up */
    PORTD &= ~((1 << PORTD3) | (1 << PORTD4)
        | (1 << PORTD5) | (1 << PORTD6));

    /* enable Pin Change interrupt for inputs */
    PCMSK2 |= (1 << PCINT19) | (1 << PCINT20) |
        (1 << PCINT21) | (1 << PCINT22);
    PCICR |= 1 << PCIE2;

    /* initialize i2c and temperature sensor */
    Wire.begin();
    Wire.beginTransmission(TCN75_ADDR);
    Wire.write((uint8_t)TCN75_TEMPERATURE_REGISTER);
    Wire.endTransmission();
}

void CustomBoard::initialize(void)
{
    hw_init();
    io_init();
    usart_init();

    /* in sleep mode "power save" the device can be awakaned
     * by an interrupts on several peripherals :
     * timer2 interrupts, int0/int1, and more...
     * - see ATmega328p datasheet Table 10-1.
     */
    SMCR = SLEEP_MODE_PWR_SAVE; // this is an atomic write

    can_device::initialize();    
}

uint8_t CustomBoard::read_inputs(void)
{
    return (PIND & 0b01111000) >> PIND3;
}

int16_t CustomBoard::read_temperature(void)
{
    Wire.requestFrom(TCN75_ADDR, 2);

    uint8_t t1 = Wire.read();
    uint8_t t2 = Wire.read();

    int16_t i16_temp = tcn75_temp2int16(t1, t2);

    /* debug */
#if LOG_LEVEL_DBG
    char temp_str[30];
    float temp = tcn75_temp2float(t1, t2);
    sprintf(temp_str, "Temperature: %0.1f °C", (double) temp);
    usart_printl(temp_str);
#endif

    return i16_temp;
}