#pragma once
#include "common.h"
// #include "server_ws.hpp"
#include "server_http.hpp"

#include "ws_svr.h"
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
// typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

class HttpSvr
{
    HttpServer server_;
    WsSvr ws_svr_;
    std::string pub_dir_, store_path_;
    std::map<std::string, std::shared_ptr<std::ofstream>> writers_;
public:
    HttpSvr(int port, const std::string& dir);
    void init();
    void ws_to_all(const std::string& json);
private:
    void static_dir(const std::string& dir);
    void serve_res();
    void post_test();
    void handle_upload();
    void emplace_ws();
    void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs, size_t len);
};