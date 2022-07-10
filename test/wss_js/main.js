'use strict';

var wss = new WebSocket("ws://127.0.0.1:3307/admin");
//let wss = new WebSocket("wss://192.168.110.206:8000/admin");
wss.onopen = function () {
    console.log("websocket open.");
    wss.send("Hello World");
}
wss.onerror = function (ev) {
    console.log("websocket error.");
}

wss.onclose = function (ev) {
    console.log("websocket close.");
}
