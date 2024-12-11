// tiny-uart.js

import { exec } from 'child_process';
import util from 'util';

const execPromise = util.promisify(exec);

// Configurare pentru GPIO
const GPIO_PIN = 12;
const BIT_DURATION = 104; // �n microsecunde pentru aproximativ 9600 baud

// Func?ie pentru setarea pinului
const setPinState = async (state) => {
    const cmd = `sudo pinctrl set ${GPIO_PIN} ${state === 'HIGH' ? 'dh' : 'dl'}`;
    try {
        await execPromise(cmd);
    } catch (err) {
        console.error(`Error setting GPIO state: ${err.message}`);
    }
};

// Func?ie pentru transmisie UART software
const uartTransmit = async (byte) => {
    // Start bit (LOW)
    await setPinState('LOW');
    await new Promise((resolve) => setTimeout(resolve, BIT_DURATION));

    // Transmitere bi?i de date (LSB primul)
    for (let i = 0; i < 8; i++) {
        const bit = (byte >> i) & 1;
        await setPinState(bit ? 'HIGH' : 'LOW');
        await new Promise((resolve) => setTimeout(resolve, BIT_DURATION));
    }

    // Stop bit (HIGH)
    await setPinState('HIGH');
    await new Promise((resolve) => setTimeout(resolve, BIT_DURATION));
};

// Func?ie pentru a trimite secven?a de date
const sendSequence = async () => {
    const sequence = [3, 2, 1];

    while (true) {
        for (const number of sequence) {
            console.log(`Sending number: ${number}`);
            await uartTransmit(number);
            await new Promise((resolve) => setTimeout(resolve, 2000)); // Pauza 2 secunde
        }
    }
};

// Pornire secven?a
sendSequence().catch((err) => console.error('Error in sendSequence:', err));
