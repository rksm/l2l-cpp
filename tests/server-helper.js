/*global require, process, global*/

var spawn = require("child_process").spawn;
var lang = require("lively.lang");
var uuid = require('node-uuid');

var defaultOpts = {
  binDir: process.cwd() + "/build/examples",
  port: 10301,
  host: "0.0.0.0"
}

function start(opts, thenDo) {
  opts = lang.obj.merge(defaultOpts, opts);

  if (!opts.id) opts.id = "cpp-server-" + uuid.v4();
  var args = ["--port", opts.port, "--host", opts.host, "--id", opts.id],
      proc = spawn("./l2l-cpp-commandline", args, {cwd: opts.binDir}),
      server = {process: proc, id: opts.id},
      startErr;

  function errH(err) { startErr = err; }
  proc.once("error", errH);
  proc.stdout.on("data", d => console.log(String(d)));
  proc.stderr.on("data", d => console.log(String(d)));
  // proc.on("exit", () => console.log("server closed"));
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