const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });

console.log("=== Servidor Proximity Chat Iniciado (Voz + GPS) ===");

wss.on('connection', function connection(ws) {
    console.log("[Node] Novo jogador conectado!");

    ws.on('message', function incoming(message, isBinary) {
        wss.clients.forEach(function each(client) {
            if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(message, { binary: isBinary });
            }
        });
    });

    ws.on('close', () => console.log("[Node] Jogador desconectado."));
});
