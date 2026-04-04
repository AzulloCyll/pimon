const express = require('express');
const { exec } = require('child_process');
const si = require('systeminformation');
const ip = "192.168.0.101";

const app = express();
const port = 5001; // Zmieniono na 5001 zgodnie z secrets.h dla kontynuacji PC Monitor

app.get('/shutdown', (req, res) => {
    console.log('Otrzymano sygnał zamknięcia PC - Wyłączam system...');
    exec('shutdown /s /f /t 10', (error, stdout, stderr) => {
        if (error) {
            console.error('Error:', error);
            return res.status(500).send('Błąd podczas wyłączania');
        }
    });
    res.send('Komputer zostanie wyłączony za 10 sekund.');
});

// Funkcja pomocnicza do pobierania temperatury GPU NVIDIA
const getGpuTemp = () => {
    return new Promise((resolve) => {
        exec('nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits', (err, stdout) => {
            if (err || !stdout) return resolve(0);
            resolve(parseFloat(stdout.trim()));
        });
    });
};

app.get('/pc-stats', async (req, res) => {
    try {
        const temp = await si.cpuTemperature();
        const load = await si.currentLoad();
        const mem = await si.mem();

        // Obliczanie procentowego użycia RAM
        const ramUsedPercent = (mem.active / mem.total) * 100;

        // Diagnostyka i wybór temperatury
        let pcTemp = temp.main || temp.max || 0;
        
        // Jeśli CPU nie podaje temperatury, spróbuj pobrać z GPU NVIDIA
        if (pcTemp === 0) {
            pcTemp = await getGpuTemp();
            if (pcTemp > 0) {
                console.log(`[DEBUG] Używam temperatury GPU: ${pcTemp}°C`);
            }
        }

        if (pcTemp === 0 && (!temp.main)) {
            console.log("[DEBUG] Brak danych o temperaturze CPU/GPU. Pełny obiekt temp:", JSON.stringify(temp));
        }

        const stats = {
            cpu: parseFloat(load.currentLoad.toFixed(1)), // % obciążenia CPU
            ram: parseFloat(ramUsedPercent.toFixed(1)),   // % użycia RAM
            temp: parseFloat(pcTemp.toFixed(1))           // Wybrana temperatura (°C)
        };

        console.log(`[${new Date().toISOString()}] PC Stats:`, stats);
        res.json(stats);
    } catch (error) {
        console.error("Błąd pobierania statystyk PC:", error);
        res.status(500).json({ error: "Błąd serwera przy pobieraniu statystyk" });
    }
});



app.listen(port, ip, () => {
    console.log(`Serwer statystyk PC działa na porcie ${port}`);
    console.log(`- Wyślij GET na http://${ip}:${port}/pc-stats aby zobaczyć dane`);
    console.log(`- Wyślij GET na http://${ip}:${port}/shutdown aby wyłączyć PC`);
});
