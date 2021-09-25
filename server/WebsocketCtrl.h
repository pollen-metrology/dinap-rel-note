//
// Created by bertrand on 26/09/2021.
//

#pragma once

#include "Builder.h"
#include <drogon/WebSocketController.h>

using namespace drogon;

struct WebsocketCtrlImpl;

class [[maybe_unused]] WebsocketCtrl : public drogon::WebSocketController<WebsocketCtrl, false> {
public:
  explicit WebsocketCtrl(Builder &);

  const static constexpr char * websocket = R"(
<script>
(function() {
  var ws = new WebSocket('ws://' + window.location.host + '/ws');
  ws.onmessage = function (msg) {
      if (msg.data === 'reload') {
          window.location.reload();
      }
  };
})();
</script>)";

  void handleNewMessage(const WebSocketConnectionPtr &,
                        std::string &&,
                        const WebSocketMessageType &) override;

  void handleNewConnection(const HttpRequestPtr &,
                           const WebSocketConnectionPtr &) override;

  void handleConnectionClosed(const WebSocketConnectionPtr &) override;

  WS_PATH_LIST_BEGIN
    //list path definitions here;
    WS_PATH_ADD("/ws");
  WS_PATH_LIST_END

private:
  std::shared_ptr<WebsocketCtrlImpl> mImpl;
};



