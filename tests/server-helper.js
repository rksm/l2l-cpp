/*global require, process, global*/

var spawn = require("child_process").spawn;
var lang = require("lively.lang");

var defaultOpts = {
  binDir: process.cwd() + "/build",
  port: 10301,
  host: "0.0.0.0"
}

function start(opts, thenDo) {
  opts = lang.obj.merge(defaultOpts, opts);
  var x = spawn(
        "pwd", [],
        {cwd: opts.binDir});
  x.stdout.on("data", d => console.log(String(d)));

  var proc = spawn(
        "./l2l-cpp",
        ["--port", opts.port, "--host", opts.host],
        {cwd: opts.binDir}),
      server = {process: proc};
  var startErr;
  function errH(err) { startErr = err; }
  proc.once("error", errH);
  proc.stdout.on("data", d => console.log(String(d)));
  proc.stderr.on("data", d => console.log(String(d)));
  proc.on("exit", () => console.log("server closed"));
  setTimeout(() => {
    proc.removeListener("error", errH);
    thenDo && thenDo(startErr, server);
  }, 100);
  
  return server;
}

function stop(server, thenDo) {
  if (server && server.process) {
    server.process.once("exit", () => thenDo && thenDo());
    server.process.kill("SIGKILL");
  } else {
    thenDo && thenDo();
  }
}

module.exports = {
  start: start,
  stop: stop
}