// file    : fcount.c
// author  : Rhys Thomas
// created : 2016-12-14
/* frequency counter.
 * todo:
 * [ ] get it working with 1Hz square wave
 * [ ] use comparator on ADC (see T3 lab)
 */

#define F_CPU 1000000L // 1MHz internal clock
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "debug.h"

typedef enum {false, true} bool;

volatile bool print=false; // flag to tell main to print
volatile uint16_t overflow=0; // number times the system overflows
volatile float new_freq=0; // calculated frequency from ext. interrupt
volatile uint8_t count=0;


/* current issues:
 * All code works and external interrupt behaves as expected.
 * However when timer1 is involved the external interrupt falls
 * over. This can be seen by the led on PB4 flashing rather than
 * staying high for one clock cycle. What is going on here?
 */
void timer1_init(void)
{
    TCCR1 |= _BV(CS10); // no prescaling
    TIMSK |= _BV(TOIE1); // enable overflow interrupt
}

void extInterrupt(void)
{
    MCUCR |= _BV(ISC01) | _BV(ISC00); // trigger interrupt on rising edge
    GIMSK |= _BV(INT0); // set external interrupt
}

ISR(INT0_vect)
{
    count++;

    // if first rising edge, reset counters
    if(count == 1) {
        TCNT1 = 0;
        overflow = 0;
        PORTB |= _BV(PB4);
    }
    // else, calculate freq and reset count
    else {
        count = 0; // reset count
        // calculate measured frequency
        uint16_t total_ticks = (overflow*255)+TCNT1;
        new_freq = F_CPU/total_ticks;
        print = true;
        PORTB &= ~_BV(PB4);
    }
}

// count number of overflows
ISR(TIMER1_OVF){overflow++;}

int main(void)
{
    DDRB |= _BV(PB4);
    PORTB &= ~_BV(PB4);

    //timer1_init(); // this is breaking things
    extInterrupt();
    initSerial(PB3);
    sei();

    char str[80];

    for(;;){
        /* everything handled by interrupts */
        if(print) {
            sprintf(str,"freq: %.2f\r\n",new_freq);
            sendStrBySerial(str);
            print = false; // reset print flag
        }
    }
    return 0;
}
