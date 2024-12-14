#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h> // Required for _delay_ms()

#define BEAT 300000

// Global variables
volatile uint32_t micros_counter = 0;
volatile uint32_t start_time = 0;

void init_pins() {
// Configure pins
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5); // PB1-PB4 as LEDs, PB5 for debugging
    DDRB &= ~(1 << PB0); // Set PB0 as input (UART RX pin)
    PORTB |= (1 << PB0); // Enable pull-up on PB0

    // Enable pin change interrupt for PB0
    GIMSK |= (1 << PCIE);       // Enable Pin Change Interrupts
    PCMSK |= (1 << PCINT0);     // Enable interrupt for PB0
}

// Timer Setup
void setup_timer() {
    TCCR0A = 0;                  // Normal mode
    TCCR0B = (1 << CS01);        // Prescaler: 8
    TIMSK0 = (1 << TOIE0);       // Enable Timer Overflow Interrupt
}

uint32_t micros(void) {
    uint32_t count;
    cli(); // Disable interrupts for atomic read
    count = micros_counter + (TCNT0 * 67) / 10;
    sei(); // Re-enable interrupts
    return count;
}

// Timer Overflow Interrupt Service Routine
ISR(TIM0_OVF_vect) {
    micros_counter += 1707; // Increment micros counter for 1.2 MHz clock and prescaler of 8
}

// Pin Change Interrupt Service Routine
ISR(PCINT0_vect) {
    uint32_t current_time = micros();

    // if (PINB & (1 << PB0)) { // PB0 is LOW
    //     // Light up PB1
    //     PORTB |= (1 << PB1);
    // } else {
    //     PORTB &= ~(1 << PB1);
    // }

    if (PINB & (1 << PB0)) {
        // sync bit detected
        // delay half a beat:
        start_time = current_time + (BEAT/2);
    }
}

int main(void) {
    init_pins();
    setup_timer();
    sei(); // Enable global interrupts

    while (1);
}