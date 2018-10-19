#include "cpp_net.h"

#include "common.h"
#include "udp.h"
#include "http_svr.h"
using namespace std;
using namespace FL;

FreeNet::FreeNet() {
    g_logger = spdlog::rotating_logger_mt("cpp_svr", "/sdcard/cpp_svr.log", 1048576 * 2, 3);
    g_logger->set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1));
    g_logger->trace("in c++ FreeNet::FreeNet()");
}

FreeNet::~FreeNet() {}

int FreeNet::start_svr(int port, const string& path)
{
    g_logger->info("in FreeNet::start_svr(int port): {}, static path : {}", port, path);
    std::thread t([=]() {
        UdpSvr us(port);
        HttpSvr server(port, path);
        
        g_logger->trace("c++ web server started at: {}", port);
        // g_logger->debug("c++ web server started at: {}", port);
        // g_logger->info("c++ web server started at: {}", port);
        // g_logger->warn("c++ web server started at: {}", port);
        // g_logger->error("c++ web server started at: {}", port);
        // g_logger->critical("c++ web server started at: {}", port);
        g_io->run();
    });
    t.detach();
    return 0;
}