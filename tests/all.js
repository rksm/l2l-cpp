/*global require, process, before, beforeEach, afterEach, describe, it, expect, global*/

var chai = module.require("chai");
var chaiSubset = module.require("chai-subset")
var expect = chai.expect; chai.use(chaiSubset);
var lang = module.require("lively.lang");

var buildHelper = module.require("./build-helper.js");
var serverHelper = module.require("./server-helper.js");
var clientHelper = module.require("./client-helper.js");

var done = err => console.log(err ? err : "done");

var server, clients, receivedMessages;

describe('3 clients, 1 server --', function() {

  this.timeout(40000);

  before(done => buildHelper.build({}, done));

  beforeEach(done => {
    clients = [];
    receivedMessages = {};
    lang.fun.composeAsync(
      n => serverHelper.start({port: 10401}, n),
      (_server, n) => { server = _server; n(); },
      n => clientHelper.startN(3, {port: 10401}, n),
      (_clients, n) => {
        _clients.forEach(c => {
          clients.push(c);
          receivedMessages[c.id] = [];
          c.on("message", m => receivedMessages[c.id].push(m));
        });
        n();
      }
    )(done);
  });

  afterEach(done =>
    lang.fun.composeAsync(
      n => clientHelper.stopN(clients, n),
      n => setTimeout(n, 100),
      n => serverHelper.stop(server, n)
    )(done));

  it("echoes", done => {
    var msg = {action: "echo", data: "test 123", sender: clients[0].id};
    lang.fun.composeAsync(
      n => clients[0].connection.sendUTF(JSON.stringify(msg), n),
      n => clients[0].once("message", m => n(null, m)),
      (_, n) => {
        expect(receivedMessages[clients[0].id]).to.deep.equal([{target: clients[0].id, sender: server.id, action: "echoResponse", data: "test 123"}]);
        expect(receivedMessages[clients[1].id]).to.have.length(0);
        expect(receivedMessages[clients[2].id]).to.have.length(0);
        n();
      }
    )(done);
  });

  it("server can list connections", done => {
    var msg = {action: "list-connections"};
    lang.fun.composeAsync(
      n => clients[0].connection.sendUTF(JSON.stringify(msg), n),
      n => clients[0].once("message", m => n(null, m)),
      (msg, n) => {
        expect(msg.data).to.include(clients[0].id);
        expect(msg.data).to.include(clients[1].id);
        expect(msg.data).to.include(clients[2].id);
        n();
      }
    )(done);
  });

  it("client to client", done => {
    var msg = {action: "foo", data: {foo: "bar"}, sender: clients[0].id, target: clients[1].id};
    lang.fun.composeAsync(
      n => clients[0].connection.sendUTF(JSON.stringify(msg), n),
      n => clients[1].once("message", m => n(null, m)),
      (_, n) => {
        expect(receivedMessages[clients[1].id]).to.deep.equal([{target: clients[1].id, sender: clients[0].id, action: "foo", data: {foo: "bar"}}]);
        expect(receivedMessages[clients[0].id]).to.have.length(0);
        expect(receivedMessages[clients[2].id]).to.have.length(0);
        n();
      }
    )(done);
  });

  it("server initialized send", done => {
    var msg = {action: "delayed-send", data: {action: "foo", payload: {testData: 123}, target: clients[1].id, delay: 400}, sender: clients[0].id};
    lang.fun.composeAsync(
      n => clients[0].connection.sendUTF(JSON.stringify(msg), n),
      n => setTimeout(n, 100),
      n => {
        expect(receivedMessages[clients[0].id]).to.have.length(0);
        expect(receivedMessages[clients[1].id]).to.have.length(0);
        expect(receivedMessages[clients[2].id]).to.have.length(0);
        clients[1].once("message", m => n(null, m));
      },
      (_, n) => {
        expect(receivedMessages[clients[1].id]).to.deep.equal([{target: clients[1].id, sender: server.id, action: "foo", data: {testData: 123}}]);
        expect(receivedMessages[clients[0].id]).to.have.length(0);
        expect(receivedMessages[clients[2].id]).to.have.length(0);
        n();
      }
    )(done);
  });

  it("server sends binary", done => {
    var msg = {action: "binary", data: {}, sender: clients[0].id};
    lang.fun.composeAsync(
      n => clients[0].connection.sendUTF(JSON.stringify(msg), n),
      n => clients[0].once("message", m => n(null, m)),
      (m, n) => {
        expect(m.type).to.equal("binary");
        expect(m.binaryData).to.have.length(10);
        expect(String(m.binaryData)).to.equal("ABCDEFGHIJ");
        n();
      }
    )(done);
  });

  // it("broadcasts", done => {
  //   lang.fun.composeAsync(
  //     n => clients[0].connection.sendUTF(JSON.stringify({action: "broadcast", data: {foo: "bar"}}), n),
  //     n => setTimeout(n, 200),
  //     (msg, n) => {
  //       expect(msg).to.deep.equal({sender: server.id, action: "echoResponse", data: "test 123"})
  //       n();
  //     }
  //   )(done);
  // });

});
