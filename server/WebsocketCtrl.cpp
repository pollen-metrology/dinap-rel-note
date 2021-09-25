//
// Created by bertrand on 26/09/2021.
//

#include "WebsocketCtrl.h"

#include <list>

struct WebsocketCtrlImpl {
  std::list<WebSocketConnectionPtr> connections;
};

WebsocketCtrl::WebsocketCtrl(Builder &builder) : mImpl(std::make_shared<WebsocketCtrlImpl>()) {
  builder.OnBuild([this]{
    for (const auto &conn: mImpl->connections) {
      if (conn->connected())
        conn->send("reload");
    }
  });
}

void WebsocketCtrl::handleNewMessage(const WebSocketConnectionPtr &, std::string &&, const WebSocketMessageType &) {

}

void WebsocketCtrl::handleNewConnection(const HttpRequestPtr &, const WebSocketConnectionPtr &conn) {
  mImpl->connections.push_back(conn);
}

void WebsocketCtrl::handleConnectionClosed(const WebSocketConnectionPtr &) {

}

