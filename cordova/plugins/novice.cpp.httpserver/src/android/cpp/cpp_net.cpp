#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "util.h"
#include "http_svr.h"
#include "socks.h"
using namespace std;
using namespace FL;

vector<shared_ptr<Service>> servers;
std::shared_ptr<UdpSvr> udp_server;
FreeNet::FreeNet() {

}

FreeNet::~FreeNet() {}
void FreeNet::start_socks_proxy(int port)
{
    LOGI("in FreeNet::start_socks_proxy(int port): %d", port);
    if(g_socks_io){
        g_socks_io->post([port](){
            auto tid = Util::get_tid();        
            auto i = std::find_if( servers.begin(), servers.end(), 
                [port](const auto& s){ return s->get_port() == port; } );  
            if(i != servers.end())
            {
                LOGI("thread[%s] same port(%d) already exist, just return", tid.c_str(), port);
            }
            else
            {
                LOGI("thread[%s] add new socks proxy server", tid.c_str());
                servers.push_back( make_shared<Socks>(port) );
            }
        });
    }
    else
    {
        std::thread t([=]() {
            auto tid = Util::get_tid();
            LOGI("thread[%s] start socks proxy ++++++++++++++++++++++++++++", tid.c_str());  
            try
            {
                g_socks_io = make_shared<boost::asio::io_context>();
                servers.push_back( make_shared<Socks>(port) );
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
int FreeNet::start_svr(int port, const string& path)
{
    // if( !g_ms.empty() )
    // {
    //     LOGI("cpp server already started, do nothing");
    //     return 0;
    // }
    LOGI("in FreeNet::start_svr(int port): %d, static path : %s", port, path.c_str());
    if(g_io){
        // LOGI("111111111111111111111111111111111");
        g_io->post([port, path](){
            // LOGI("22222222222222222222222222222222222");
            auto tid = Util::get_tid();        
            auto i = std::find_if( servers.begin(), servers.end(), 
                [port](const auto& hs){ return hs->get_port() == port; } );  
            if(i != servers.end())
            {
                LOGI("thread[%s] same port(%d) already exist, just return", tid.c_str(), port);
            }
            else
            {
                LOGI("thread[%s] add new http server", tid.c_str());
                // g_io->stop();
                servers.push_back( make_shared<HttpSvr>(port, path) );
                // udp_server = make_shared<UdpSvr>(port);
                // g_io->restart();
                // g_io->run();  
                // LOGI("after io_service run");
            }
        });
        return 0;
    }
    std::thread t([=]() {
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