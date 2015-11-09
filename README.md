# l2l-cpp

A C++ websocket server based on
[websocketpp](https://github.com/zaphoyd/websocketpp) that allows message sends
via the Lively-2-Lively protocol.

## Usage


```c++
  auto echoService = l2l::createLambdaService("echo",
    [](Json::Value msg, std::shared_ptr<l2l::L2lServer> server) {
      Json::Value answer;
      answer["data"] = msg["data"];
      server->answer(msg, answer);
    });

  l2l::Services services{echoService, delayedService, binaryService};

  int port = 10001;
  std::string host = "0.0.0.0";
  std::string id = "my-test-server";
  auto server = l2l::startServer(host, port, id, services);
```

For a more complex example see [](examples/command-line.cpp).

## License

MIT