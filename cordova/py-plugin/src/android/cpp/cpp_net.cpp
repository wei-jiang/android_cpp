#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "http_svr.h"
using namespace std;
using namespace FL;

FreeNet::FreeNet() {

}

FreeNet::~FreeNet() {}

int FreeNet::start_svr(int port, const string& path)
{
    LOGI("in FreeNet::start_svr(int port): %d, static path : %s", port, path.c_str());
    std::thread t([=]() {
        UdpSvr us(port);
        HttpSvr server(port, path);
        g_io->run();
    });
    t.detach();
    return 0;
}