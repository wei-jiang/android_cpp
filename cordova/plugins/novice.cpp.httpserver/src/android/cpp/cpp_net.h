#pragma once

#include <string>
#include <vector>

// this class export to java
namespace FL {
    class FreeNet
    {
    public:
        FreeNet();
        virtual ~FreeNet();
        int start_http(int port, const std::string& path);
        void start_socks(int port);
        // true: start; false: stop
        void udp_on_svr(const std::string& svr_addr, int32_t id, uint32_t token);
        void udp_off_svr(const std::string& svr_addr);
        std::string get_noty();
        std::string get_str(){
            return "string from c++ 666";
        }
    };
}