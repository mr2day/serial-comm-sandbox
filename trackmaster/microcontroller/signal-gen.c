#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t micros_counter = 0;
volatile uint32_t start_time = 0;
volatile uint8_t state = 0;
volatile uint8_t bit_index = 0;
volatile uint8_t value_index = 0; // Tracks the current value being sent

// Configuration: Define the array of values to send
const uint8_t values[] = {2, 5, 3}; // Example: series of values
const uint8_t num_values = sizeof(values) / sizeof(values[0]); // Number of values to send

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

// Signal Generation Steps
void initial_idle(uint32_t current_time) {
    if (current_time - start_time >= 2000000) { // 2 seconds HIGH
        state = 0; // Transition to start bit
        start_time = current_time;
    }
}

void send_start_bit(uint32_t current_time) {
    if (current_time - start_time >= 250000) { // 250 ms for start bit
        PORTB &= ~(1 << PB0); // LOW for start bit
        state = 1;
        start_time = current_time;
    }
}

void send_data_bit(uint32_t current_time) {
    if (current_time - start_time >= 250000) { // 250 ms per bit
        if (bit_index < 8) {
            if (values[value_index] & (1 << bit_index)) {
                PORTB |= (1 << PB0); // HIGH for '1'
            } else {
                PORTB &= ~(1 << PB0); // LOW for '0'
            }
            bit_index++;
            start_time = current_time;
        } else {
            bit_index = 0;
            state = 2; // Move to stop bit
            start_time = current_time;
        }
    }
}

void send_stop_bit(uint32_t current_time) {
    if (current_time - start_time >= 250000) { // 250 ms for stop bit
        PORTB |= (1 << PB0); // HIGH
        if (value_index < num_values - 1) {
            value_index++; // Move to the next value in the array
            state = 0;     // Start next byte
        } else {
            state = 3;     // No more values to send
        }
        start_time = current_time;
    }
}

// Timer Overflow Interrupt Service Routine
ISR(TIM0_OVF_vect) {
    micros_counter += 1707;
    uint32_t current_time = micros_counter;

    switch (state) {
        case 255: // Initial idle state
            initial_idle(current_time);
            break;
        case 0: // Start bit
            send_start_bit(current_time);
            break;
        case 1: // Data bits
            send_data_bit(current_time);
            break;
        case 2: // Stop bit
            send_stop_bit(current_time);
            break;
        case 3: // Final idle (remain HIGH)
            break;
    }
}

int main(void) {
    DDRB |= (1 << PB0);  // Set PB0 as output
    PORTB |= (1 << PB0); // Ensure the line starts HIGH
    setup_timer();
    sei();               // Enable global interrupts

    start_time = 0;      // Initialize timer
    state = 255;         // Begin in initial idle state

    while (1);           // No operations in the main loop
}
