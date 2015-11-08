/*global require, process, global*/


var WebSocketClient = require('websocket').client;
var url = require("url");
var lang = require("lively.lang");
var uuid = require('node-uuid');

var defaultOpts = {
  protocol: "ws:",
  slashes: true,
  hostname: "0.0.0.0",
  port: 10301
}

function makeClient(opts) {
  opts = lang.obj.merge(defaultOpts, opts);

  var ws = new WebSocketClient();
  ws.id = "test-client-" + uuid.v4();
  ws.on('connectFailed', function(error) { console.log('Connect Error: ' + error.toString()); });
  ws.on('connect', function(connection) {
    ws.connection = connection;
    ws.connection.on("message", m => {
      if (m.type === "utf8") {
        try { m = JSON.parse(m.utf8Data); }
        catch (e) {}
      }
      ws.emit("message", m);
    });
  });
  ws.connect(url.format(opts));
  return ws;
}

function start(opts, thenDo) {
  var ws = makeClient(opts);
  if (thenDo) {
    thenDo = lang.fun.once(thenDo);
    ws.once("connect", () => {
      ws.connection.sendUTF(JSON.stringify({sender: ws.id, action: "register"}));
      ws.once("message", () => thenDo(null, ws));
    });
    ws.once('connectFailed', (err) => thenDo(err, ws));
  }
  return ws;
}

function stop(client, thenDo) {
  thenDo && thenDo();
}

function startN(n, opts, thenDo) {
  lang.fun.waitForAll(
    {timeout: 3000},
    lang.arr.range(1,n).map(() => n => start(opts, n)),
    (err, results) => thenDo && thenDo(err, lang.arr.flatten(results)));
}

function stopN(clients, thenDo) {
  lang.fun.waitForAll(
    {timeout: 3000},
    clients.map(c => n => stop(c, n)),
    (err, results) => thenDo && thenDo(err));
}

module.exports = {
  start: start,
  stop: stop,
  startN: startN,
  stopN: stopN
}
