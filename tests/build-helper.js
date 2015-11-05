/*global require, process, global*/

var exec = require("child_process").exec;
var lang = require("lively.lang");

var defaultOpts = {
  cwd: process.cwd() + "/build",
  steps: [
    {cmd: "cmake", args: ["-DCMAKE_BUILD_TYPE=RELEASE", "-DCMAKE_EXPORT_COMPILE_COMMANDS=1", ".."]},
    {cmd: "make", args: []}
  ]
}

// build({}, err => console.log(err || "ok"));

function build(opts, thenDo) {
  opts = lang.obj.merge(defaultOpts, opts);
  console.log(opts.cwd);
  lang.arr.mapAsyncSeries(opts.steps,
    (step, _, n) => {
      console.log(">>> %s", step.cmd);
      var cmdString = step.cmd + " " + (step.args || []).join(" ");
      var proc = exec(cmdString, {cwd: opts.cwd}, n);
      proc.stderr.on("data", d => console.log(String(d)));
      proc.stdout.on("data", d => console.log(String(d)));
      proc.on("exit", () => console.log("<<< %s", step.cmd))
    },
    thenDo);
}

module.exports = {
  build: build
}
