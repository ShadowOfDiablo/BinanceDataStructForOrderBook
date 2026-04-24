const WebSocket = require('ws');

const ws = new WebSocket('wss://stream.binance.com:9443/ws/btcusdt@depth@100ms');

ws.on('open', function open() {
  console.error('WebSocket connection opened');
});

ws.on('message', function incoming(data) {
  // Output raw JSON to stdout for piping to C++
  process.stdout.write(data + '\n');
});

ws.on('close', function close() {
  console.error('WebSocket connection closed');
});

ws.on('error', function error(err) {
  console.error('WebSocket error:', err);
});