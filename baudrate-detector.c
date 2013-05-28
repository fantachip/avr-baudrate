/* 
BAUD RATE RS232 DETECTOR EXAMPLE 
 - tested on atmega8
 
Baud rate is measured by looking for level changes on int0 pin and 
measuring the time between the shortest level change. That is our 
bit time and thus our bit rate. 

Connect using usb to TTL serial: 
- "picocom -b 38400 /dev/ttyUSB0"

Example board using ATMega88 running at 18.432Mhz.
*/

#define F_CPU 18432000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "baudrate.h"

const u32 BD_TICKS_PER_SEC = F_CPU;

//Counted time
volatile u32 clock_datetime = 0;

//Initialise the clock
void clock_init(){
	TIMSK1 |= (1<<TOIE1);
	TCCR1B |= (1<<CS10);
}

//Overflow itnerrupt
ISR(TIMER1_OVF_vect)
{
	clock_datetime ++;
}

//Return time
ticks_t clock_ticks(){
	cli();
	ticks_t time = (clock_datetime * 0x10000) + TCNT1;
	sei();
	return time;
}

ISR(INT0_vect){
	bd_state_changed();
}

#define BAUD_RATE 38400
#define BAUD_PRESCALE (F_CPU/(BAUD_RATE*16L)-1)


void uart_putchar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

char uart_getchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}
static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
	
void uart_init(){
	UBRR0L = (uint8_t)BAUD_PRESCALE;
	UBRR0H = BAUD_PRESCALE >> 8; 

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	
	stdout = &uart_output;
	stdin  = &uart_input;
}

u08 poll_pin(){
	return BIT(D, 2, PIN) != 0;
}

int main(){
	// enable pin change int0
	EICRA |= _BV(ISC01) ; //falling edge
  EIMSK |= _BV(INT0); //enable INT0 interrupt 
  
	clock_init();
	uart_init();
	
	sei();
	
	BIT(B, 1, DDR) = OUTPUT; 
	BIT(D, 2, DDR) = INPUT; 
	
	bd_init(&clock_ticks, &poll_pin);
	
	while(1){
		printf("\x1b[H\x1b[2J"); // cursor home, clear screen (vt100)
		printf("========================================================\r\n");
		printf("===              BAUD RATE CALCULATOR                ===\r\n");
		printf("===                 running on AVR                   ===\r\n");
		printf("===       visit https://github.com/fantachip         ===\r\n");
		printf("========================================================\r\n");
		
		u32 baud = bd_baudrate();
		//u32 baud = bd_measure(); 
		printf("Baudrate measured: [%-6ldbps]\r", baud * 2); 
		// x2 because we are measuring only falling edge intervals
		// if int0 is set to trigger on "any edge" then we can use "baud" value as is
		bd_reset(); 
		_delay_ms(500); 
	}
}
