import { execSync } from 'child_process';

// Configurare GPIO
const GPIO_PIN = 12; // Pinul GPIO folosit pentru transmitere
execSync(`sudo pinctrl set ${GPIO_PIN} op`); // Seteaza GPIO ca iesire

// Functie pentru a trimite HIGH si LOW pentru un interval specificat
function sendSignal(state, durationMs) {
    const cmd = `sudo pinctrl set ${GPIO_PIN} ${state === 'HIGH' ? 'dh' : 'dl'}`;
    execSync(cmd);
    setTimeout(() => {
        execSync(`sudo pinctrl set ${GPIO_PIN} dl`); // Revenire la LOW
    }, durationMs);
}

// Functie pentru transmiterea unui bit
function sendBit(bit) {
    if (bit) {
        sendSignal('HIGH', 1); // HIGH timp de 1 ms
        sendSignal('LOW', 0.33); // LOW timp de 0.33 ms
    } else {
        sendSignal('LOW', 0.33); // LOW timp de 0.33 ms
    }
}

// Functie pentru transmiterea unui byte (8 biti)
function sendByte(byte) {
    for (let i = 0; i < 8; i++) {
        const bit = (byte >> (7 - i)) & 1; // Transmite de la MSB la LSB
        sendBit(bit);
    }
}

// Functie pentru transmiterea sincronizarii (preamble)
function sendSync() {
    console.log('Trimit sincronizare...');
    sendSignal('HIGH', 1000); // HIGH timp de 1 sec
    sendSignal('LOW', 1000);  // LOW timp de 1 sec
}

// Functie pentru transmiterea unui pachet complet
function sendPulsePacket(address, pin, state, duration) {
    console.log(`Transmit pachet: Adresa=${address}, Pin=${pin}, Stare=${state}, Durata=${duration}`);

    // Calculare checksum
    const checksum = (address + pin + state + duration) & 0xFF;

    // Sincronizare
    sendSync();

    // Header
    console.log('Trimit header...');
    sendByte(0xAA);

    // Campuri
    console.log('Transmit campuri...');
    sendByte(address);
    sendByte(pin);
    sendByte(state);
    sendByte(duration);

    // Checksum
    console.log(`Trimit checksum: ${checksum}`);
    sendByte(checksum);

    // Footer
    console.log('Trimit footer...');
    sendByte(0x55);

    // Reset
    console.log('Trimit reset...');
    sendSignal('LOW', 1330); // LOW timp de 1.33 sec
}

// Bucla principala
function main() {
    console.log('Incep transmisia...');

    // Exemplu: Transmite valori ciclic
    const packets = [
        { address: 1, pin: 0, state: 1, duration: 5 },
        { address: 1, pin: 1, state: 1, duration: 10 },
        { address: 1, pin: 2, state: 1, duration: 15 },
    ];

    let index = 0;

    setInterval(() => {
        const packet = packets[index];
        sendPulsePacket(packet.address, packet.pin, packet.state, packet.duration);

        index = (index + 1) % packets.length; // Trecem la urmatorul pachet
    }, 5000); // Pauza de 5 secunde intre pachete
}

main();
