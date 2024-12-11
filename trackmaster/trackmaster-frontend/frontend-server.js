import express from 'express';

const app = express();
const PORT = 3000;
const BACKEND_URL = 'http://192.168.0.196:5000/send-command';

app.get('/', (req, res) => {
    res.send(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Trackmaster Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            margin: 0;
            padding: 20px;
        }
        select, button, input {
            margin: 10px;
            padding: 10px;
            font-size: 16px;
        }
        .buttons {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 10px;
        }
    </style>
</head>
<body>
    <h1>Trackmaster Control</h1>
    <label for="decoderAddress">Adresa decodorului:</label>
    <select id="decoderAddress">
        ${Array.from({ length: 13 }, (_, i) => `<option value="${i + 1}">Adresa ${i + 1}</option>`).join('')}
    </select>
    <label for="duration">Durata (secunde):</label>
    <input type="number" id="duration" value="10" min="0" max="255">
    <div class="buttons">
        ${Array.from({ length: 5 }, (_, i) => `<button onclick="togglePin(${i})">Pin ${i}</button>`).join('')}
    </div>
    <script>
        const backendUrl = "${BACKEND_URL}";

        function togglePin(pin) {
            const address = document.getElementById('decoderAddress').value;
            const duration = document.getElementById('duration').value;
            const state = 1; // Activam iesirea (ON)

            fetch(backendUrl, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    address: Number(address),
                    pin: Number(pin),
                    state: state,
                    duration: Number(duration)
                }),
            })
            .then(response => response.json())
            .then(data => console.log('Server response:', data))
            .catch(error => console.error('Error:', error));
        }
    </script>
</body>
</html>`);
});

app.listen(PORT, () => {
    console.log(`Frontend server ruleaza pe http://localhost:${PORT}`);
});
