#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "util.h"
#include "http_svr.h"
using namespace std;
using namespace FL;

vector<shared_ptr<HttpSvr>> http_servers;
std::shared_ptr<UdpSvr> udp_server;
FreeNet::FreeNet() {

}

FreeNet::~FreeNet() {}

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
            auto i = std::find_if( http_servers.begin(), http_servers.end(), 
                [port](const auto& hs){ return hs->get_port() == port; } );  
            if(i != http_servers.end())
            {
                LOGI("thread[%s] same port(%d) already exist, just return", tid.c_str(), port);
            }
            else
            {
                LOGI("thread[%s] add new http server", tid.c_str());
                // g_io->stop();
                http_servers.push_back( make_shared<HttpSvr>(port, path) );
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
            http_servers.push_back( make_shared<HttpSvr>(port, path) );
            // udp_server = make_shared<UdpSvr>(port);  
            while (1)
            {
                g_io->poll(); 
            }            
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