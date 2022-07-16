'use strict';

var wss = new WebSocket("wss://xlinyum.com:3307/admin");
//let wss = new WebSocket("wss://192.168.110.206:8000/admin");
wss.onopen = function () {
    console.log("websocket open.");
}
wss.onerror = function (ev) {
    console.log("websocket error.");
}

wss.onmessage = function(data){
    console.log("Recv: " + data.data);
}

wss.onclose = function (ev) {
    console.log("websocket close.");
}

document.getElementById('sendBtn').onclick = function () {
    let msg = document.getElementById('msg').value;
    if (msg != "")
    {
        console.log("Send: " + msg);
        wss.send(msg);
    }
}
