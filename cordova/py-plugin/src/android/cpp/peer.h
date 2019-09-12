#pragma once
#include <server_http.hpp>
#include "common.h"
#include "peer_handler.h"


using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class HttpSvr;
class UdpSvr;


class Peer
{
    std::shared_ptr<HttpServer> server_;
    std::shared_ptr<UdpSvr> udp_;
    HttpSvr * http_svr_;
    std::map<std::string, std::shared_ptr<PeerHandler>> peers_;
public:
    Peer(HttpSvr * http_svr);
    void init();
private:
    void mount_pub_svr();
    void dismount_pub_svr();
    void connect_to_peer();

};