#define F_CPU 1200000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#define BIT_MICROS 500000
#define BLINK_DELAY 300

volatile uint32_t micros_counter = 0;
volatile uint32_t start_time = 0;

void init_pins() {
    DDRB |= (1 << PB0) | (1 << PB1);  // Set PB0 and PB1 as outputs
    // PORTB |= (1 << PB0); // Ensure the line starts HIGH
}

// Timer Setup
void setup_timer() {
    TCCR0A = 0;                  // Normal mode
    TCCR0B = (1 << CS01);        // Prescaler: 8
    TIMSK0 = (1 << TOIE0);       // Enable Timer Overflow Interrupt
}

void turn_on_led_1() {
    PORTB |= (1 << PB1);
}

void turn_off_led_1() {
    PORTB &= ~(1 << PB1);
}

void toggle_led_1() {
    PORTB ^= (1 << PB1);
}

void blink_led_1(times) {
    uint8_t i;
    for (i = 0; i < times; i++) {
        turn_on_led_1();
        _delay_ms(BLINK_DELAY);
        turn_off_led_1();
        _delay_ms(BLINK_DELAY);
    }
}

uint32_t micros(void) {
    uint32_t count;
    cli(); // Disable interrupts for atomic read
    count = micros_counter + (TCNT0 * 67) / 10;
    sei(); // Re-enable interrupts
    return count;
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

// Timer Overflow Interrupt Service Routine
ISR(TIM0_OVF_vect) {
    static uint32_t lastBitMicros = 0;
    static uint32_t byte_buffer = 0;
    uint32_t actMicros, bitMicros;
    
    micros_counter += 1707; // Increment micros counter for 1.2 MHz clock and prescaler of 8
    
    actMicros = micros(); // Get current time in microseconds
    bitMicros = actMicros - lastBitMicros;
    if (bitMicros >= BIT_MICROS) { // 0.5-second interval
        PORTB ^= (1 << PB0); // Toggle LED // this is a "beat", an interval of half second, that we will use as a bit duration
        // the bit of this "beat" can be 0 or 1; LOW or HIGH
        // let's count these bits and form a byte
        // what if we send 8 zeroes?
        byte_buffer++;
        lastBitMicros = actMicros; // Update toggle time
    }
    if (byte_buffer == 8) {
        // a byte is completed; blink the led!
        const char* binaryString = "00000011";
        uint8_t decimalValue = binaryStringToDecimal(binaryString);
        blink_led_1(decimalValue);
        
        byte_buffer = 0;
    }
}

int main(void) {
    init_pins();
    setup_timer();
    sei();               // Enable global interrupts
    while (1);           // No operations in the main loop
}