module.require("code-pump").start();

var Mocha = module.require('mocha'),
    fs = module.require('fs'),
    path = module.require('path');

var mocha = new Mocha({ui: 'bdd', reporter: 'list'});
mocha.addFile("tests/all.js");

mocha.run(failures => {
  console.log("Mocha run ended");
  console.log(failures);
  // process.on('exit', () => process.exit(failures))
});
