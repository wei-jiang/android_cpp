#pragma once
#include "common.h"
#include "server_ws.hpp"


typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

class WsSvr
{
    WsServer ws_server_;
    typedef std::function<void(pt::ptree&, std::shared_ptr<WsServer::Connection> cnn)> Handlers;
    std::map<std::string, Handlers> handlers_;
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
    void return_json(std::shared_ptr<WsServer::Connection> cnn, pt::ptree&);

};