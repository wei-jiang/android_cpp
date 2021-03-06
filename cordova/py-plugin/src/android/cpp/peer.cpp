#include "peer.h"
#include "udp.h"
#include "http_svr.h"

using namespace std;
Peer::Peer(HttpSvr * http_svr)
:http_svr_(http_svr)
{
    server_ = http_svr->server_;
    udp_ = http_svr->udp_;
    init();
}
void Peer::init()
{
    mount_pub_svr();
    dismount_pub_svr();
    connect_to_peer();

}
void Peer::dismount_pub_svr()
{
    server_->resource["^/dismount_pub_svr$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        json res;
        try
        {
            auto data = json::parse(request->content);
            auto svr_addr = data["svr_addr"].get<string>();
            LOGI("dismount_pub_svr : svr_addr=%s;", svr_addr.c_str());
            udp_->off_svr(svr_addr);
            res["ret"] = 0;
        }
        catch (const exception &e)
        {
            // response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
            res["ret"] = -1;
            res["msg"] = e.what();
        }
        http_svr_->res_json(response, res);
        // response->write(res.dump(), header);
    };
}
void Peer::mount_pub_svr()
{
    server_->resource["^/mount_pub_svr$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        json res;
        try
        {
            auto data = json::parse(request->content);
            auto svr_addr = data["svr_addr"].get<string>();
            auto id = data["id"].get<string>();
            auto token = data["token"].get<uint32_t>();
            LOGI("mount_pub_svr : svr_addr=%s; id=%s; token=%u", svr_addr.c_str(), id.c_str(), token );
            udp_->on_svr(svr_addr, id, token);
            res["ret"] = 0;
        }
        catch (const exception &e)
        {
            // response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
            res["ret"] = -1;
            res["msg"] = e.what();
        }
        http_svr_->res_json(response, res);
        // response->write(res.dump(), header);
    };
}
void Peer::connect_to_peer()
{
    server_->resource["^/connect_to_peer$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        json res;

        try
        {
            auto data = json::parse(request->content);
            auto id = data["id"].get<string>();
            auto ep = data["ep"].get<string>();
     
            LOGI("connect_to_peer : id=%s; ep=%s; ", id.c_str(), ep.c_str() );
            auto it = peers_.find(id);
            if( it != peers_.end() )
            {
                it->second->before_destroy();
            }
            peers_[id] = make_shared<PeerHandler>(udp_, ep);
                
            res["ret"] = 0;
        }
        catch (const exception &e)
        {
            // response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
            res["ret"] = -1;
            res["msg"] = e.what();
        }
        http_svr_->res_json(response, res);
        // response->write(res.dump(), header);
    };
}