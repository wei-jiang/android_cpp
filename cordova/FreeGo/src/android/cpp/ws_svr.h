#pragma once
#include "common.h"
#include "server_ws.hpp"


typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

class WsSvr
{
    WsServer ws_server_;
public:
    WsSvr();
    void init();
    WsServer& get_ws_svr()
    {
        return ws_server_;
    }
    void to_all(const std::string& json);
private:
    void ep_for_cpp();
};