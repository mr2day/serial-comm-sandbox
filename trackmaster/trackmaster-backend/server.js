import express from 'express';
import cors from 'cors';
import { execSync } from 'child_process';

const app = express();
const PORT = 5000;

// Activeaza CORS
app.use(cors());

// Middleware pentru parsing JSON
app.use(express.json());

// Functie pentru trimiterea protocolului catre Attiny13
function sendCommandToMicrocontroller(address, pin, state, duration) {
    const command = `${address} ${pin} ${state} ${duration}`;
    const bytes = command.split(' ').map((v) => parseInt(v, 10));
    console.log(`Comanda trimisa: Address=${address}, Pin=${pin}, State=${state}, Duration=${duration}`);
    console.log(`Bytes trimisi:`, bytes);

    // Trimite comanda prin GPIO
    bytes.forEach((byte) => {
        for (let i = 0; i < 8; i++) {
            const bit = (byte >> i) & 1;
            execSync(`sudo pinctrl set 12 ${bit ? 'dh' : 'dl'}`); // Trimite bitul
            execSync('sudo pinctrl set 12 dl'); // Tranzitie LOW
        }
    });
    console.log(`Comanda trimisa cu succes.`);
}

// Endpoint pentru primirea comenzilor
app.post('/send-command', (req, res) => {
    const { address, pin, state, duration } = req.body;

    if (
        address >= 1 && address <= 13 && // Adresa valida
        pin >= 0 && pin <= 4 &&          // Pin valid
        (state === 0 || state === 1) &&  // Stare valida
        duration >= 0 && duration <= 255 // Durata valida
    ) {
        sendCommandToMicrocontroller(address, pin, state, duration);
        res.status(200).json({ status: 'success', message: 'Command sent' });
    } else {
        res.status(400).json({ status: 'error', message: 'Invalid parameters' });
    }
});

// Pornire server
app.listen(PORT, () => {
    console.log(`Backend server ruleaza pe http://192.168.0.196:${PORT}`);
});
 