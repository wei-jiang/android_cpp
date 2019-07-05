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
        int start_svr(int port, const std::string& path);
        std::string get_str(){
            return "string from c++ 666";
        }
    };
}