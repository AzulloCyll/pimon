const express = require('express');
const { exec } = require('child_process');

const app = express();
const port = 6541;

app.get('/shutdown', (req, res) => {
    exec('shutdown /s /f /t 0', (error, stdout, stderr) => {
        if (error) {
            console.error('Error:', error);
            return res.status(500).send('Błąd podczas wyłączania');
        }
    });
    res.send('Komputer jest wyłączany...');
});

app.listen(port, () => {
    console.log(`Serwer działa na porcie ${port}`);
    console.log('Wyślij GET na /shutdown aby wyłączyć PC');
});
