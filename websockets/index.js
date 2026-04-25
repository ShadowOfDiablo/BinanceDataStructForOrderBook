const WebSocket = require('ws');
const https = require('https');

const SYMBOL = (process.argv[2] || 'BNBBTC').toUpperCase();
const WS_URL = `wss://stream.binance.com:9443/ws/${SYMBOL.toLowerCase()}@depth@100ms`;
const SNAPSHOT_URL = `https://api.binance.com/api/v3/depth?symbol=${SYMBOL}&limit=5000`;

function fetchSnapshot() {
    return new Promise((resolve, reject) => {
        https.get(SNAPSHOT_URL, res => {
            let data = '';
            res.on('data', chunk => data += chunk);
            res.on('end', () => {
                try { resolve(JSON.parse(data)); }
                catch (e) { reject(e); }
            });
        }).on('error', reject);
    });
}

async function main() {
    const eventBuffer = [];
    let synced = false;

    const ws = new WebSocket(WS_URL);
    ws.on('open',  () => console.error('WebSocket opened'));
    ws.on('close', () => console.error('WebSocket closed'));
    ws.on('error', err => console.error('WebSocket error:', err));

    ws.on('message', data => {
        const event = JSON.parse(data.toString());
        if (synced) {
            process.stdout.write(JSON.stringify(event) + '\n');
        } else {
            eventBuffer.push(event);
        }
    });

    // Wait for at least one buffered event so we have a firstU
    await new Promise(resolve => {
        const id = setInterval(() => {
            if (eventBuffer.length > 0) { clearInterval(id); resolve(); }
        }, 50);
    });

    // Fetch snapshot; retry until lastUpdateId >= U of the first buffered event
    let snapshot;
    const firstU = eventBuffer[0].U;
    while (true) {
        snapshot = await fetchSnapshot();
        if (snapshot.lastUpdateId >= firstU) break;
        console.error(`Snapshot too old (lastUpdateId=${snapshot.lastUpdateId} < firstU=${firstU}), retrying...`);
        await new Promise(r => setTimeout(r, 500));
    }

    // Send snapshot to C++ as a typed message
    process.stdout.write(JSON.stringify({
        type: 'snapshot',
        symbol: SYMBOL,
        lastUpdateId: snapshot.lastUpdateId,
        bids: snapshot.bids,
        asks: snapshot.asks,
    }) + '\n');

    // Flush buffered events — C++ will discard any where u <= snapshot.lastUpdateId
    for (const event of eventBuffer) {
        process.stdout.write(JSON.stringify(event) + '\n');
    }

    // Switch to direct streaming
    synced = true;
}

main().catch(err => { console.error('Fatal:', err); process.exit(1); });
