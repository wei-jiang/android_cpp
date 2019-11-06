#pragma once
#include "common.h"
#include "db.h"
#include "server_http.hpp"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class HttpHome: public Service
{
    std::shared_ptr<HttpServer> server_;
    std::shared_ptr<DB> db_;
    std::string assets_dir_, home_path_;
    int port_;
public:
    HttpHome(int port, const std::string& dir);
    ~HttpHome();
    void init();
    virtual int get_type(){return SVR_HOME;}
    int get_port(){return port_;}
private:
    void static_dir();
    void handle_sql();
    void handle_cors();
    void check_pass();
    void res_json(std::shared_ptr<HttpServer::Response> response, json& data);
};