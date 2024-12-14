#define F_CPU 1200000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#define DELAY 1000

void uart_init(void) {
    DDRB |= (1 << PB0); // Set PB0 as output
    PORTB |= (1 << PB0); // Set idle state to HIGH
}

void uart_send_byte(uint8_t data) {
    // Start bit
    PORTB &= ~(1 << PB0);
    _delay_us(833); // 1200 baud delay

    // Send 8 data bits (LSB first)
    uint8_t bit = 0;
    while (bit < 8) {
        if (data & (1 << bit)) {
            PORTB |= (1 << PB0); // Data HIGH
        } else {
            PORTB &= ~(1 << PB0); // Data LOW
        }
        _delay_us(833);
        bit++;
    }

    // Stop bit
    PORTB |= (1 << PB0);
    _delay_us(833);
}

void uart_send_message(uint8_t *message) {
    uint8_t i = 0;
    while (i < 3) { // Standard message length is 3
        uart_send_byte(message[i]);
        i++;
    }
}

uint8_t random_between(uint8_t min, uint8_t max) {
    return min + (rand() % (max - min + 1)); // Generate random number in range [min, max]
}

int main(void) {
    uint8_t message[] = {1, 5, 5}; // Address, Pin, Duration in tenths of seconds
    uint8_t toggle = 1;           // Toggle variable for address alternation
    uart_init();

    // Seed the random number generator
    srand(eeprom_read_word((uint16_t*)0x00) ^ TCNT0); // Example seeding using EEPROM and timer

    while (1) {
        message[0] = toggle;                  // Set the address
        message[1] = random_between(1, 5);   // Random pin number (1–5)
        message[2] = random_between(0, 50);  // Random activation duration (1–10 tenths of seconds)

        uart_send_message(message);
        _delay_ms(1000);                     // Delay of 1 second

        toggle = (toggle == 1) ? 2 : 1;      // Alternate between 1 and 2
    }
}
