const { execSync } = require('child_process');

// GPIO settings
const GPIO_PIN = 12;
execSync(`sudo pinctrl set ${GPIO_PIN} op`); // Set GPIO pin as output

// Helper function to send a signal
function sendSignal(state, durationMs) {
    const cmd = `sudo pinctrl set ${GPIO_PIN} ${state === 'HIGH' ? 'dh' : 'dl'}`;
    execSync(cmd);
    setTimeout(() => {
        execSync(`sudo pinctrl set ${GPIO_PIN} dl`);
    }, durationMs);
}

// UART Settings
const BAUD_RATE = 1200; // 1200 baud
const BIT_DURATION_MS = Math.round(1000 / BAUD_RATE); // Duration of one bit in milliseconds

// Send a single byte
function sendByte(byte) {
    // Start bit (LOW)
    sendSignal('LOW', BIT_DURATION_MS);

    // Data bits (LSB first)
    for (let i = 0; i < 8; i++) {
        const bitState = (byte & (1 << i)) ? 'HIGH' : 'LOW';
        sendSignal(bitState, BIT_DURATION_MS);
    }

    // Stop bit (HIGH)
    sendSignal('HIGH', BIT_DURATION_MS);
}

// Send a UART message (array of bytes)
function sendMessage(message) {
    for (const byte of message) {
        sendByte(byte);
    }
}

// Generator logic
function startGenerator() {
    let toggle = 1; // Toggle address between 1 and 2
    setInterval(() => {
        const message = [
            toggle, // Address (1 or 2)
            Math.floor(Math.random() * 5) + 1, // Random pin number (1–5)
            Math.floor(Math.random() * 10) + 1, // Random duration (1–10 tenths of a second)
        ];
        console.log(`Sending message: ${message}`);
        sendMessage(message);
        toggle = toggle === 1 ? 2 : 1; // Alternate address
    }, 1000); // 1-second interval
}

// Start the generator
startGenerator();
