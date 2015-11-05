/*global require, process, before, beforeEach, afterEach, describe, it, expect, global*/

var chai = module.require("chai");
var chaiSubset = module.require("chai-subset")
var expect = chai.expect; chai.use(chaiSubset);
var lang = module.require("lively.lang");

var buildHelper = module.require("./build-helper.js");
var serverHelper = module.require("./server-helper.js");
var clientHelper = module.require("./client-helper.js");

var done = err => console.log(err ? err : "done");

var server, client;

describe('1 client, 1 server', function() {

  this.timeout(40000);


  before(done => buildHelper.build({}, done));

  beforeEach(done =>
    lang.fun.composeAsync(
      n => serverHelper.start({port: 10401}, n),
      (_server, n) => { server = _server; n(); },
      n => clientHelper.start({port: 10401}, n),
      (_client, n) => { client = _client; n(); }
    )(done));

  afterEach(done =>
    lang.fun.composeAsync(
      n => serverHelper.stop(server, n),
      n => clientHelper.stop(client, n)
    )(done));

  // afterEach(done => serverHelper.stop(server, done));
// server.process
// server.process.kill("SIGKILL")
  it("server echoes client", done => {
    // var server = serverHelper.start({port: 10401});
    // serverHelper.stop(server);
    // var ws = clientHelper.start({port: 10401});
    var messages = [];
    client.on("message", msg => messages.push(msg))
    lang.fun.composeAsync(
      n => client.connection.sendUTF(JSON.stringify({action: "echo", data: "test 123"}), n),
      n => lang.fun.waitFor(() => !!messages.length, n),
      n => {
        expect(messages).to.have.length(1);
        n();
      }
      // n => client.connection.sendUTF('{"msg": "hello"}', n)
    )(done)
  });
});
