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
    std::string assets_dir_, store_path_;
    std::map<std::string, std::shared_ptr<std::ofstream>> writers_;
    int port_;
public:
    HttpSvr(int port, const std::string& dir);
    ~HttpSvr();
    void init();
    void ws_to_all(const std::string& json);
    int get_port(){return port_;}
private:
    void static_dir(const std::string& dir);
    void serve_res();
    void get_files();
    void handle_upload();
    void emplace_ws();
    void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs, size_t len);
    void routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t);
};