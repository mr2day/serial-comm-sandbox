const { execSync } = require('child_process');

// GPIO settings
const GPIO_PIN = 12;
execSync(`sudo pinctrl set ${GPIO_PIN} op`); // Set GPIO pin as output

// UART settings
const BAUD_RATE = 1200; // 1200 baud
const BIT_DURATION_MS = Math.round(1000 / BAUD_RATE); // Duration of one bit in milliseconds

// Seed random number generator
function seedRandom() {
    const seed = parseInt(execSync('cat /dev/urandom | tr -dc 0-9 | head -c 5').toString(), 10);
    Math.seedrandom(seed);
}

// Helper function to send a signal
function sendSignal(state, durationMs) {
    const cmd = `sudo pinctrl set ${GPIO_PIN} ${state === 'HIGH' ? 'dh' : 'dl'}`;
    execSync(cmd);
    setTimeout(() => {
        execSync(`sudo pinctrl set ${GPIO_PIN} dl`);
    }, durationMs);
}

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

// Generate a random number between min and max
function randomBetween(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

// Generator logic
function startGenerator() {
    let toggle = 1; // Toggle variable for address alternation
    seedRandom(); // Seed the random number generator

    setInterval(() => {
        const message = [
            toggle,                  // Address (1 or 2)
            randomBetween(1, 5),     // Random pin number (1–5)
            randomBetween(0, 50),    // Random activation duration (0–50 tenths of seconds)
        ];
        console.log(`Sending message: ${message}`);
        sendMessage(message);
        toggle = toggle === 1 ? 2 : 1; // Alternate address
    }, 1000); // 1-second delay
}

// Start the generator
startGenerator();
