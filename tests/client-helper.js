/*global require, process, global*/


var WebSocketClient = require('websocket').client;
var url = require("url");
var lang = require("lively.lang");

var defaultOpts = {
  protocol: "ws:",
  slashes: true,
  hostname: "0.0.0.0",
  port: 10301
}

function makeClient(opts) {
  opts = lang.obj.merge(defaultOpts, opts);

  var ws = new WebSocketClient();
  ws.on('connectFailed', function(error) { console.log('Connect Error: ' + error.toString()); });
  ws.on('connect', function(connection) { ws.connection = connection; });
  ws.connect(url.format(opts));
  return ws;
}

function start(opts, thenDo) {
  var ws = makeClient(opts);
  ws.once("connect", () => thenDo && thenDo(null, ws));
  ws.once('connectFailed', (err) => thenDo && thenDo(err, ws));
  return ws;
}

function stop(server, thenDo) {
  thenDo && thenDo();
}

module.exports = {
  start: start,
  stop: stop
}