#pragma once
#include "common.h"

#include <server_http.hpp>

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;


class HttpSvr;
class UdpSvr;
class Peer
{
    std::shared_ptr<HttpServer> server_;
    std::shared_ptr<UdpSvr> udp_;
    HttpSvr * http_svr_;

public:
    Peer(HttpSvr * http_svr);
    void init();
private:
    void mount_pub_svr();
    void connect_to_peer();

};