#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "util.h"
#include "http_svr.h"
#include "socks.h"
using namespace std;
using namespace FL;

static vector<shared_ptr<Service>> servers;
std::shared_ptr<UdpSvr> udp_server;
FreeNet::FreeNet() {

}

FreeNet::~FreeNet() {}
void FreeNet::udp_on_svr(const std::string& svr_addr, int32_t id, uint32_t token)
{
    if(g_socks_io){
        g_socks_io->post([svr_addr, id, token]{
            udp_server->on_svr(svr_addr, id, token);
        });
    }
}
void FreeNet::udp_off_svr(const std::string& svr_addr)
{
    if(g_socks_io){
        g_socks_io->post([svr_addr]{
            udp_server->off_svr(svr_addr);
        });
    }
}
std::string FreeNet::get_noty()
{
    // LOGI("begin FreeNet::get_noty()");
    auto r = cpp2java_que.wait_and_pop();
    // LOGI("end FreeNet::get_noty()---%s", r->c_str());
    return *r;
}
void FreeNet::start_socks(int port)
{
    LOGI("in FreeNet::start_socks(int port): %d", port);
    if(g_socks_io){
        g_socks_io->post([port](){
            auto tid = Util::get_tid();        
            auto i = std::find_if( servers.begin(), servers.end(), 
                [port](const auto& s){ return s->get_port() == port; } );  
            auto json = Util::to_json({
                    {"cmd", "chg_socks_port_back"},
                    {"ret", "0"},
                    {"port", std::to_string(port)}
                });
            if(i != servers.end())
            {
                LOGI("thread[%s] same port(%d) already exist, just return", tid.c_str(), port);
                if(SVR_SOCKS != (*i)->get_type()){
                    json = Util::to_json({
                        {"cmd", "chg_socks_port_back"},
                        {"ret", "-1"},
                        {"port", std::to_string(port)}
                    });
                }
            }
            else
            {
                LOGI("thread[%s] add new socks proxy server", tid.c_str());
                servers.push_back( make_shared<Socks>(port) );
            }
            cpp2java_que.push(json);
        });
    }
    else
    {
        std::thread t([port]() {
            auto tid = Util::get_tid();
            LOGI("thread[%s] start socks proxy ++++++++++++++++++++++++++++", tid.c_str());  
            try
            {
                g_socks_io = make_shared<boost::asio::io_context>();
                servers.push_back( make_shared<Socks>(port) );
                udp_server = make_shared<UdpSvr>(port);
                for(auto&& s: servers) LOGI("thread[%s] port=%d;type=%d", tid.c_str(), s->get_port(), s->get_type());
                g_socks_io->run();          
            }
            catch(const std::exception& e)
            {
                LOGE( "start services failed: %s", e.what() );
            }  
            LOGI("thread[%s] exit ------------------------------", tid.c_str());      
        });
        t.detach();
    }
    
    
}
int FreeNet::start_http(int port, const string& path)
{
    LOGI("in FreeNet::start_http(int port): %d, static path : %s", port, path.c_str());
    if(g_io){
        // LOGI("111111111111111111111111111111111");
        g_io->post([port, path](){
            // LOGI("22222222222222222222222222222222222");
            auto tid = Util::get_tid();        
            for(auto&& s: servers) LOGI("thread[%s] port=%d;type=%d", tid.c_str(), s->get_port(), s->get_type());
            auto i = std::find_if( servers.begin(), servers.end(), 
                [port](const auto& s){ return s->get_port() == port; } );  
            auto json = Util::to_json({
                    {"cmd", "chg_http_port_back"},
                    {"ret", "0"},
                    {"port", std::to_string(port)}
                });
            if(i != servers.end())
            {
                LOGI("thread[%s] same port(%d) already exist, just return", tid.c_str(), port);
                if(SVR_HTTP != (*i)->get_type()){
                    json = Util::to_json({
                        {"cmd", "chg_http_port_back"},
                        {"ret", "-1"},
                        {"port", std::to_string(port)}
                    });
                }               
            }
            else
            {
                LOGI("thread[%s] add new http server", tid.c_str());
                // g_io->stop();
                servers.push_back( make_shared<HttpSvr>(port, path) );                
                // g_io->restart();
                // g_io->run();  
                // LOGI("after io_service run");    
            }
            cpp2java_que.push(json);
        });
        return 0;
    }
    std::thread t([port, path]() {
        // UdpSvr us(port);
        // HttpSvr server(port, path);
        auto tid = Util::get_tid();
        LOGI("thread[%s] start ++++++++++++++++++++++++++++", tid.c_str());  
        try
        {
            g_io = make_shared<boost::asio::io_context>();
            servers.push_back( make_shared<HttpSvr>(port, path) );
            // udp_server = make_shared<UdpSvr>(port);  
            // do not use while/poll, too much power consumed
            cpp2java_que.push(Util::to_json({
                {"cmd", "http_ready"},
                {"port", std::to_string(port)}
            }));
            g_io->run();          
        }
        catch(const std::exception& e)
        {
            LOGE( "start services failed: %s", e.what() );
        }  
        LOGI("thread[%s] exit ------------------------------", tid.c_str());      
    });
    t.detach();
    
    return 0;
}