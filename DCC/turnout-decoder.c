#include <NmraDcc.h>

// Pin Definitions
#define DCC_PIN PB0      // DCC input pin
#define GREEN_PIN PB1    // Green button output
#define RED_PIN PB2      // Red button output

// Hardcoded DCC Address
#define TURNOUT_ADDRESS 1 // Replace with desired NMRA turnout address

// DCC Decoder Object
NmraDcc Dcc;

// Function to handle turnout commands
void processTurnoutCommand(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
    if (Addr == TURNOUT_ADDRESS && OutputPower) {
        if (Direction == 0) {
            // Red button pressed
            digitalWrite(GREEN_PIN, LOW);
            digitalWrite(RED_PIN, HIGH);
        } else if (Direction == 1) {
            // Green button pressed
            digitalWrite(RED_PIN, LOW);
            digitalWrite(GREEN_PIN, HIGH);
        }
    }
}

// Callback function for DCC accessory turnout packets
void notifyDccAccTurnoutBoard(uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower) {
    uint16_t Addr = ((BoardAddr - 1) * 4) + OutputPair + 1;
    processTurnoutCommand(Addr, Direction, OutputPower);
}

void setup() {
    // Configure pins
    pinMode(DCC_PIN, INPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(RED_PIN, OUTPUT);

    // Initialize outputs
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);

    // Initialize the DCC decoder
    Dcc.pin(DCC_PIN, 1); // Enable pull-up resistor
    Dcc.initAccessoryDecoder(0x00, 0x01, FLAGS_MY_ADDRESS_ONLY, 0);
}

void loop() {
    // Process DCC messages
    Dcc.process();
}
