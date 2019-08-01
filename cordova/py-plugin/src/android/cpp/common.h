#pragma once

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <regex>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <map>
#include <queue>
#include <unordered_set>
#define BOOST_SPIRIT_THREADSAFE
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/property_tree/ptree.hpp> 
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <android/log.h>
#define TAG "cpp_svr"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,    TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,     TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,     TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,    TAG, __VA_ARGS__)

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;
template<typename T>
class TSQueue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    TSQueue()
    {}
    TSQueue(TSQueue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue=other.data_queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        value=data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
enum 
{
    SVR_HTTP = 0,
    SVR_SOCKS,
};
class Service
{
public:
    virtual int get_port() = 0;
    virtual int get_type(){return SVR_HTTP;}
    virtual ~Service(){}
};



extern std::shared_ptr<boost::asio::io_context> g_io, g_socks_io;
extern std::string g_ms;