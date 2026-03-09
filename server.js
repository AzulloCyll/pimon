const express = require('express');
const si = require('systeminformation');
const { exec } = require('child_process');

const app = express();
const PORT = 5000;

// Aby móc odczytywać JSON z requestów POST
app.use(express.json());

app.post('/shutdown', (req, res) => {
    console.log(`[${new Date().toISOString()}] Otrzymano sygnał wyłączenia Raspberry Pi:`, req.body);
    res.status(200).json({ status: "ok", message: "Shutting down Raspberry Pi..." });

    // Wykonanie komendy systemowej zamknięcia z 1-sekundowym opóźnieniem (aby serwer zdążył odpowiedzieć)
    setTimeout(() => {
        exec("sudo poweroff", (error, stdout, stderr) => {
            if (error) {
                console.error(`Błąd przy wyłączaniu: ${error.message}`);
                return;
            }
            if (stderr) {
                console.error(`Błąd z komendy: ${stderr}`);
                return;
            }
            console.log(`Zamykanie systemu: ${stdout}`);
        });
    }, 1000);
});

app.get('/stats', async (req, res) => {
    try {
        // Pobieranie danych z systemu
        const temp = await si.cpuTemperature();
        const load = await si.currentLoad();
        const mem = await si.mem();

        // Obliczanie procentowego użycia RAM (aktywnej pamięci w stosunku do całości)
        const ramUsedPercent = (mem.active / mem.total) * 100;

        // Budowanie obiektu z kluczami, których oczekuje kod C++ w AtomS3
        const stats = {
            cpu: parseFloat(load.currentLoad.toFixed(1)), // % obciążenia CPU
            ram: parseFloat(ramUsedPercent.toFixed(1)),   // % użycia RAM
            temp: parseFloat((temp.main || 0).toFixed(1)) // Temperatura w °C
        };

        console.log(`[${new Date().toISOString()}] Wysyłam dane:`, stats);
        
        // Wysłanie JSONa
        res.json(stats);

    } catch (error) {
        console.error("Błąd odczytu danych systemowych:", error);
        res.status(500).json({ error: "Błąd serwera" });
    }
});

app.listen(PORT, '0.0.0.0', () => {
    console.log(`Serwer monitora RPi uruchomiony!`);
    console.log(`Nasłuchuje żądań GET na http://0.0.0.0:${PORT}/stats`);
});
