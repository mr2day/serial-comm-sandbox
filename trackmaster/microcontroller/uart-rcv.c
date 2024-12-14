#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Globals
volatile uint32_t micros_counter = 0; // Timer counter for micros()
volatile uint8_t receiving = 0;       // Flag to indicate UART reception in progress
volatile uint8_t received_value = 0;  // Store the decoded UART value
volatile uint8_t bit_index = 0;       // Current bit index
volatile uint32_t start_time = 0;     // Start time for sampling

// Function declarations
uint32_t micros(void);
void update_leds(uint8_t value);

// Timer Overflow Interrupt (for micros())
ISR(TIM0_OVF_vect) {
    micros_counter += 256; // Increment micros counter on overflow
}

// Pin Change Interrupt (Start Bit Detection)
ISR(PCINT0_vect) {
    if (!(PINB & (1 << PB0)) && !receiving) { // Start bit detected (LOW)
        receiving = 1;         // Start UART reception
        received_value = 0;    // Clear the received value
        bit_index = 0;         // Reset bit index
        start_time = micros(); // Record start time
    }
}

// Timer Compare Interrupt (Sampling Data Bits)
ISR(TIM0_COMPA_vect) {
    if (receiving) {
        if (bit_index < 8) { // Sample each data bit
            if (PINB & (1 << PB0)) {
                received_value |= (1 << bit_index); // Set bit to HIGH
            } else {
                received_value &= ~(1 << bit_index); // Set bit to LOW
            }
            bit_index++; // Move to the next bit
        } else { // Stop bit reached
            receiving = 0;       // End reception
            update_leds(received_value); // Update LEDs
        }
    }
}

// micros() implementation
uint32_t micros(void) {
    uint32_t count;
    cli(); // Disable interrupts for atomic read
    count = micros_counter + TCNT0;
    sei(); // Re-enable interrupts
    return count * 8 / 12; // Convert ticks to microseconds
}

// Update LEDs based on the received value
void update_leds(uint8_t value) {
    uint8_t i; // Loop variable moved outside for compatibility
    for (i = 0; i < 5; i++) {
        if (value & (1 << i)) {
            PORTB |= (1 << (PB1 + i)); // Turn on LED
        } else {
            PORTB &= ~(1 << (PB1 + i)); // Turn off LED
        }
    }
}

// Timer Setup for Sampling
void setup_sampling_timer(void) {
    TCCR0A = (1 << WGM01);           // CTC Mode
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler: 64
    OCR0A = 117;                     // Compare Match: 250 ms
    TIMSK0 |= (1 << OCIE0A);         // Enable Compare Match Interrupt
}

// Timer Setup for micros()
void setup_timer(void) {
    TCCR0A = 0;                     // Normal mode
    TCCR0B = (1 << CS01);           // Prescaler: 8
    TIMSK0 |= (1 << TOIE0);         // Enable Timer Overflow Interrupt
}

int main(void) {
    // Configure PB1 to PB5 as outputs (LEDs)
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5);

    // Configure PB0 as input with pull-up
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);

    // Enable Pin Change Interrupt for PB0
    GIMSK |= (1 << PCIE);       // Enable Pin Change Interrupts
    PCMSK |= (1 << PCINT0);     // Enable interrupt for PB0

    // Setup timers
    setup_timer();            // Timer for micros()
    setup_sampling_timer();   // Timer for sampling data bits

    // Enable global interrupts
    sei();

    // Main loop does nothing
    while (1);
}
