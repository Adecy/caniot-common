#include "hw_init.h"

// @see "init" function from arduino in "wiring.c"

void hw_init(void)
{
	// this needs to be called before setup() or some functions won't
	// work there
    sei();

    // on the ATmega168, timer 0 is also used for fast hardware pwm
    // (using phase-correct PWM would mean that timer 0 overflowed half as often
    // resulting in different millis() behavior on the ATmega8 and ATmega168)

    // millis used by can library
    sbi(TCCR0A, WGM01);
    sbi(TCCR0A, WGM00);

    // set timer 0 prescale factor to 64
    // this combination is for the standard 168/328/1280/2560
    sbi(TCCR0B, CS01);
    sbi(TCCR0B, CS00);

    // enable timer 0 overflow interrupt
    sbi(TIMSK0, TOIE0);

    // timers 1 and 2 are used for phase-correct hardware pwm
    // this is better for motors as it ensures an even waveform
    // note, however, that fast pwm mode can achieve a frequency of up
    // 8 MHz (with a 16 MHz clock) at 50% duty cycle
#if ARDUINO_ENABLE_FAST_PWM
    TCCR1B = 0;
    sbi(TCCR1B, CS11);
    sbi(TCCR1B, CS10);

    sbi(TCCR1A, WGM10);
#endif

    // set timer 2 prescale factor to 64
    sbi(TCCR2B, CS22);
    sbi(TCCR2A, WGM20);

    // set a2d prescaler so we are inside the desired 50-200 KHz range.
#if ARDUINO_ENABLE_ANALOG
    sbi(ADCSRA, ADPS2);
    sbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
    sbi(ADCSRA, ADEN);  // enable disable analog
#endif

    // the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
	UCSR0B = 0;
}
