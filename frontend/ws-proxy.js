const { WebSocketServer } = require('ws');
const net = require('net');

const WS_PORT = 8080;
const MUD_HOST = '127.0.0.1';
const MUD_PORT = 4000;

const IAC = 0xFF;
const WILL = 0xFB;
const WONT = 0xFC;
const DO = 0xFD;
const DONT = 0xFE;
const SB = 0xFA;
const SE = 0xF0;
const TELOPT_ECHO = 1;
const TELOPT_GMCP = 201;

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
    const { text, gmcp } = parseTelnet(buf, tcp);
    // Send any GMCP messages as JSON text frames
    for (const msg of gmcp) {
      if (ws.readyState === 1) {
        ws.send(JSON.stringify({ gmcp: msg.pkg, data: msg.data }));
      }
    }
    // Send normal text
    if (text.length > 0 && ws.readyState === 1) {
      ws.send(text);
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

/**
 * Parse telnet data: strip IAC sequences, extract GMCP subnegotiations.
 * Auto-responds IAC DO GMCP when server sends IAC WILL GMCP.
 * Returns { text: Buffer, gmcp: [{ pkg, data }] }
 */
function parseTelnet(buf, tcp) {
  const out = [];
  const gmcp = [];
  let i = 0;

  while (i < buf.length) {
    if (buf[i] === IAC && i + 1 < buf.length) {
      const cmd = buf[i + 1];
      if (cmd >= WILL && cmd <= DONT && i + 2 < buf.length) {
        const opt = buf[i + 2];
        // Auto-negotiate GMCP: respond DO to WILL
        if (cmd === WILL && opt === TELOPT_GMCP && tcp.writable) {
          tcp.write(Buffer.from([IAC, DO, TELOPT_GMCP]));
          console.log('[GMCP] Negotiated GMCP with server');
        }
        // Forward echo control to client (password masking)
        if (opt === TELOPT_ECHO) {
          gmcp.push({ pkg: 'Echo', data: { off: cmd === WILL } });
        }
        i += 3;
      } else if (cmd === SB) {
        // Subnegotiation: IAC SB <option> <data...> IAC SE
        const option = i + 2 < buf.length ? buf[i + 2] : 0;
        i += 3; // skip IAC SB option
        const start = i;
        while (i < buf.length - 1) {
          if (buf[i] === IAC && buf[i + 1] === SE) { break; }
          i++;
        }
        if (option === TELOPT_GMCP) {
          const payload = buf.slice(start, i).toString('utf8');
          const spaceIdx = payload.indexOf(' ');
          if (spaceIdx > 0) {
            try {
              gmcp.push({
                pkg: payload.substring(0, spaceIdx),
                data: JSON.parse(payload.substring(spaceIdx + 1))
              });
            } catch (e) {
              // Malformed GMCP JSON, skip
            }
          } else {
            gmcp.push({ pkg: payload, data: {} });
          }
        }
        i += 2; // skip IAC SE
      } else if (cmd === IAC) {
        out.push(0xFF); // Escaped 0xFF
        i += 2;
      } else {
        i += 2; // Other 2-byte commands (e.g. IAC GA)
      }
    } else {
      out.push(buf[i]);
      i++;
    }
  }

  return { text: Buffer.from(out), gmcp };
}
