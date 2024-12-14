#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h> // Required for _delay_ms()

// Global variables
volatile uint32_t micros_counter = 0;
volatile uint32_t start_time = 0;
volatile uint8_t bit_count = 0;
volatile uint8_t receiving = 0;   // Flag to indicate receiving state
volatile uint8_t received_value = 0;
volatile uint8_t value_ready = 0; // Flag to indicate a valid value is ready

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
    static uint8_t bit_buffer = 0;

    // Detect start bit
    if (!receiving && !(PINB & (1 << PB0))) { // Start bit detected (LOW)
        receiving = 1;
        PORTB |= (1 << PB5); // Debug: Indicate start bit detected
        start_time = current_time + 125000; // Wait for midpoint of first bit
        bit_count = 0;
        bit_buffer = 0;
    } else if (receiving) {
        // Process data bits
        if (bit_count < 8) {
            if (current_time >= start_time) {
                uint8_t bit = (PINB & (1 << PB0)) ? 1 : 0; // Read bit value
                bit_buffer |= (bit << bit_count); // Store bit in buffer
                bit_count++;
                start_time += 250000; // Move to the next bit midpoint
            }
        } else if (bit_count == 8) {
            // Stop bit detected
            if (PINB & (1 << PB0)) { // Check stop bit (HIGH)
                received_value = bit_buffer;
                value_ready = 1; // Mark value as ready
                PORTB &= ~(1 << PB5); // Debug: Turn off PB5 after processing
            } else {
                received_value = 0; // Invalid stop bit
                value_ready = 0;
            }
            receiving = 0; // Reset state
        }
    }
}

void update_leds() {
    static uint8_t last_value = 0; // Track last displayed value
    uint8_t i;

    if (value_ready) {
        if (received_value != last_value) { // Only update on new valid value
            last_value = received_value;

            // Debugging: Light PB5 based on the received value
            for (i = 0; i < received_value; i++) {
                PORTB |= (1 << PB5);
                _delay_ms(100);
                PORTB &= ~(1 << PB5);
                _delay_ms(100);
            }

            // Update LEDs
            PORTB &= ~((1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4)); // Turn off all LEDs
            if (received_value > 0) {
                uint8_t led = PB1;
                for (i = 0; i < received_value && i < 4; i++) { // Now includes PB4
                    PORTB |= (1 << led++);
                }
            }
        }

        value_ready = 0; // Clear ready flag
    }
}

int main(void) {
    // Configure pins
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5); // PB1-PB4 as LEDs, PB5 for debugging
    DDRB &= ~(1 << PB0); // Set PB0 as input (UART RX pin)
    PORTB |= (1 << PB0); // Enable pull-up on PB0

    // Enable pin change interrupt for PB0
    GIMSK |= (1 << PCIE);       // Enable Pin Change Interrupts
    PCMSK |= (1 << PCINT0);     // Enable interrupt for PB0

    setup_timer();
    sei(); // Enable global interrupts

    while (1) {
        update_leds(); // Update LEDs based on received value
    }
}
