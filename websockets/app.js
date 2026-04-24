const socket = new WebSocket('wss://stream.binance.com:9443/ws/btcusdt@depth');

socket.addEventListener('open', function (event) {
    console.log('WebSocket connection opened');
});

socket.addEventListener('message', function (event) {
    console.log('Received data:', event.data);
});

socket.addEventListener('close', function (event) {
    console.log('WebSocket connection closed');
});

socket.addEventListener('error', function (event) {
    console.error('WebSocket error:', event);
});