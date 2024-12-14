#define F_CPU 1200000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define MY_ADDRESS 2

#define MESSAGE_DURATION_MS 30 // Approximate time for a full message (3 bytes at 1200 baud)
#define MAX_PINS 5   // Maximum number of pins to control (PB1-PB5)

volatile uint8_t count = 0;
volatile uint8_t address_valid = 0;
volatile uint8_t active_pin = 0;

void uart_init(void) {
    DDRB = 0xFE; // PB0 as input, PB1-PB5 as outputs
    PORTB = 0x00; // Ensure all outputs start LOW
    PCMSK |= (1 << PCINT0); // Enable pin change interrupt for PB0
    GIMSK |= (1 << PCIE);   // Enable pin change interrupts
    sei();                  // Enable global interrupts
}

void activate_pin(uint8_t active_pin, uint8_t duration) {
    if (duration == 0) {
        PORTB &= ~(1 << active_pin); // Turn the pin OFF
    } else {
        PORTB |= (1 << active_pin); // Turn the pin ON
        if (duration < 255) {       // Only process duration if it's not "forever"
            while (duration > 0) {
                _delay_ms(100);
                duration--;
            }
            PORTB &= ~(1 << active_pin); // Turn the pin OFF after the duration
        }
        // If duration is 255, the pin remains ON forever
    }
}

void turn_off_pin(void) {
    if (active_pin >= 1 && active_pin <= MAX_PINS) {
        PORTB &= ~(1 << active_pin); // Turn off the active pin
        active_pin = 0;
    }
}

uint8_t uart_receive_byte(void) {
    uint8_t data = 0;
    uint8_t bit = 0; // Initialize bit counter
    _delay_us(416); // Align to middle of start bit

    while (bit < 8) {
        _delay_us(833); // Wait for one bit time
        if (PINB & (1 << PB0)) {
            data |= (1 << bit);
        }
        bit++;
    }
    _delay_us(833); // Wait for stop bit
    return data;
}

void reset_receiver_state(void) {
    count = 0;
    address_valid = 0;
    active_pin = 0;
}

void block_receiver(void) {
    GIMSK &= ~(1 << PCIE); // Disable pin change interrupts
    _delay_ms(MESSAGE_DURATION_MS); // Wait for the standard message duration
    GIMSK |= (1 << PCIE);  // Re-enable pin change interrupts
}

void process_received_byte(uint8_t byte) {
    count++;
    if (count == 1) {
        if (byte == MY_ADDRESS) {
            address_valid = 1; // Address matches
        } else {
            block_receiver();  // Ignore irrelevant message
            count = 0;         // Reset count for the next potential message
        }
    } else if (count == 2 && address_valid) {
        active_pin = byte; // Save the pin number
    } else if (count == 3 && address_valid) {
        uint8_t duration = byte; // Duration in tenths of seconds
        activate_pin(active_pin, duration);
        count = 0; // Prepare for the next message
        address_valid = 0; // Reset address validation for the next message
    }
}

ISR(PCINT0_vect) {
    if (!(PINB & (1 << PB0))) { // Start bit detected
        uint8_t byte = uart_receive_byte();
        process_received_byte(byte);
    }
}

int main(void) {
    uart_init();
    while (1);
}
