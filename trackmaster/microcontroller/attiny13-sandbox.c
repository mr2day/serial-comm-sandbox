#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Interrupt Service Routine for Pin Change Interrupt
ISR(PCINT0_vect) {
    if (!(PINB & (1 << PB0))) {  // Check if PB0 is grounded (LOW)
        PORTB |= (1 << PB1);     // Turn on LED on PB1
        PORTB &= ~(1 << PB2);    // Turn off LED on PB2
    } else {                    // PB0 is not grounded (HIGH)
        PORTB &= ~(1 << PB1);    // Turn off LED on PB1
        PORTB |= (1 << PB2);     // Turn on LED on PB2
    }
}

int main(void)
{
    DDRB |= (1 << PB1);  // Set PB1 as output
    DDRB |= (1 << PB2);  // Set PB2 as output
    DDRB |= (1 << PB3);  // Set PB3 as output
    DDRB |= (1 << PB4);  // Set PB4 as output
    DDRB |= (1 << PB5);  // Set PB5 as output
    DDRB &= ~(1 << PB0); // Set PB0 as input
    PORTB |= (1 << PB0); // Enable pull-up resistor on PB0

    // Enable pin change interrupt for PB0
    GIMSK |= (1 << PCIE);       // Enable Pin Change Interrupts
    PCMSK |= (1 << PCINT0);     // Enable interrupt for PB0

    // Enable global interrupts
    sei();

    while (1);
}
