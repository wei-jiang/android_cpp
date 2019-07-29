#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "http_svr.h"
using namespace std;
using namespace FL;

std::shared_ptr<HttpSvr> http_server;
std::shared_ptr<UdpSvr> udp_server;
FreeNet::FreeNet() {
    // std::thread t([=]() {
    //     LOGI("g_io->run()");
    //     g_io->run();
    // });
    // t.detach();
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
    std::thread t([=]() {
        // UdpSvr us(port);
        // HttpSvr server(port, path);
        std::thread::id this_id = std::this_thread::get_id();
        stringstream ss;
        ss << this_id;
        string tid = ss.str();
        LOGI("thread[%s] start +++", tid.c_str());  
        try
        {
            if(g_io) g_io->stop();
            g_io = make_shared<boost::asio::io_context>();
            http_server = make_shared<HttpSvr>(port, path);
            udp_server = make_shared<UdpSvr>(port);            
            g_io->run();           
        }
        catch(const std::exception& e)
        {
            LOGE( "start services failed: %s", e.what() );
        }  
        LOGI("thread[%s] exit ---", tid.c_str());      
    });
    t.detach();
    
    return 0;
}