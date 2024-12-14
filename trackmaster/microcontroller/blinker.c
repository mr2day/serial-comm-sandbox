#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t micros_counter = 0;  // Tracks microseconds
volatile uint32_t last_toggle_time = 0; // Last time the LED toggled

// micros() Implementation
uint32_t micros(void) {
    uint32_t count;
    cli(); // Disable interrupts for atomic read
    count = micros_counter + (TCNT0 * 67) / 10;
    sei(); // Re-enable interrupts
    return count; // Return microseconds
}

// Timer Setup
void setup_timer() {
    TCCR0A = 0;                     // Normal mode
    TCCR0B = (1 << CS01);           // Prescaler: 8
    TIMSK0 = (1 << TOIE0);          // Enable Timer Overflow Interrupt
}

void blink() {
    uint32_t current_time = micros(); // Get current time in microseconds
    if (current_time - last_toggle_time >= 1000000) { // 1-second interval
        PORTB ^= (1 << PB0);        // Toggle LED
        last_toggle_time = current_time; // Update toggle time
    }
}

// Timer Overflow Interrupt Service Routine
ISR(TIM0_OVF_vect) {
    micros_counter += 1707;          // Increment micros counter on overflow
    blink();
}

int main(void) {
    DDRB |= (1 << PB0);   // Set PB0 as output
    PORTB |= (1 << PB0);  // Start with LED ON
    setup_timer();
    sei();                // Enable global interrupts
    while (1);            // No operations in the main loop
}
