#define F_CPU 1200000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define BEAT 300

volatile uint32_t micros_counter = 0;
volatile uint32_t start_time = 0;

void init_pins() {
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);  // Set PB0, PB1 and PB2 as outputs
    PORTB |= (1 << PB0); // Ensure the line starts HIGH
}

uint8_t binaryStringToDecimal(const char* binaryString) {
    uint8_t decimalValue = 0;
    int i;
    // Iterate over the string, converting each bit
    for (i = 0; i < strlen(binaryString); i++) {
        decimalValue = (decimalValue << 1) | (binaryString[i] - '0');
    }
    return decimalValue;
}

void blink_led_1(times) {
    uint8_t i;
    for (i = 0; i < times; i++) {
        PORTB |= (1 << PB1);
        _delay_ms(BEAT);
        PORTB &= ~(1 << PB1);
        _delay_ms(BEAT);
    }
}

void send_binary_string(const char *str) {
    uint8_t i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '0') {
            PORTB &= ~(1 << PB0); // Turn off LED
        } else if (str[i] == '1') {
            PORTB |= (1 << PB0);  // Turn on LED
        }
        _delay_ms(BEAT); // Add delay to observe the change
    }
}

void send_sync_bit() {
    PORTB &= ~(1 << PB0);
    _delay_ms(BEAT);
    PORTB |= (1 << PB0);
}

void show_finish() {
    PORTB |= (1 << PB2);
}

int main(void) {
    init_pins();
    const char* binaryString = "00000101"; // Binary string
    uint8_t decimalValue = binaryStringToDecimal(binaryString);
    blink_led_1(decimalValue);
    _delay_ms(1000);
    send_sync_bit();
    send_binary_string(binaryString);
    show_finish();
    while (1);           // No operations in the main loop
}
