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
        std::string get_noty();
        std::string get_str(){
            return "string from c++ 666";
        }
    };
}