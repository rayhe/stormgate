const { WebSocketServer } = require('ws');
const net = require('net');

const WS_PORT = 8080;
const MUD_HOST = '127.0.0.1';
const MUD_PORT = 4000;

const wss = new WebSocketServer({ port: WS_PORT });
console.log(`WebSocket proxy listening on port ${WS_PORT}, forwarding to ${MUD_HOST}:${MUD_PORT}`);

wss.on('connection', (ws, req) => {
  const ip = req.headers['x-forwarded-for'] || req.socket.remoteAddress;
  console.log(`[+] ${ip} connected`);

  const tcp = net.createConnection(MUD_PORT, MUD_HOST);

  tcp.on('connect', () => {
    console.log(`[+] ${ip} -> MUD connected`);
  });

  tcp.on('data', (buf) => {
    // Strip telnet IAC sequences (0xFF ...)
    const clean = stripTelnet(buf);
    if (clean.length > 0 && ws.readyState === 1) {
      ws.send(clean);
    }
  });

  tcp.on('close', () => {
    console.log(`[-] ${ip} MUD disconnected`);
    ws.close();
  });

  tcp.on('error', (err) => {
    console.error(`[!] ${ip} TCP error: ${err.message}`);
    ws.close();
  });

  ws.on('message', (msg) => {
    if (tcp.writable) {
      tcp.write(msg);
    }
  });

  ws.on('close', () => {
    console.log(`[-] ${ip} disconnected`);
    tcp.destroy();
  });

  ws.on('error', (err) => {
    console.error(`[!] ${ip} WS error: ${err.message}`);
    tcp.destroy();
  });
});

function stripTelnet(buf) {
  const out = [];
  let i = 0;
  while (i < buf.length) {
    if (buf[i] === 0xFF && i + 1 < buf.length) {
      const cmd = buf[i + 1];
      if (cmd >= 0xFB && cmd <= 0xFE && i + 2 < buf.length) {
        i += 3; // WILL/WONT/DO/DONT + option byte
      } else if (cmd === 0xFA) {
        // Subnegotiation: skip until IAC SE (0xFF 0xF0)
        i += 2;
        while (i < buf.length - 1) {
          if (buf[i] === 0xFF && buf[i + 1] === 0xF0) { i += 2; break; }
          i++;
        }
      } else if (cmd === 0xFF) {
        out.push(0xFF); // Escaped 0xFF
        i += 2;
      } else {
        i += 2; // Other 2-byte commands
      }
    } else {
      out.push(buf[i]);
      i++;
    }
  }
  return Buffer.from(out);
}
