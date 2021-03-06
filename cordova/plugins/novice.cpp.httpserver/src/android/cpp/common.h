#pragma once

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>
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
#include <functional>
#include <unordered_set>
namespace ph = std::placeholders;
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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
// #include <boost/signals2/signal.hpp>

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
    SVR_HTTP    = 1,
    SVR_SOCKS,
    SVR_HOME,
};
class Service
{
public:
    virtual int get_port() = 0;
    virtual int get_type(){return SVR_HTTP;}
    virtual ~Service(){}
};
template<typename T>
class Singleton {
public:
    static T& instance()
    {
        static T instance;
        return instance;
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton) = delete;

protected:
    Singleton() {}
};
class LYTimer
{   
public:
    LYTimer()
    :t_(std::chrono::steady_clock::now())
    {}
    void reset()
    {
        t_ = std::chrono::steady_clock::now();
    }
    int64_t elapsed_in_seconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - t_).count();
    }
private:
    std::chrono::steady_clock::time_point t_;
};
class CNNBase: public LYTimer
{
public:
    virtual void remove_self(bool noty_flag = true) = 0;
    virtual ~CNNBase(){}

};
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "server_ws.hpp"
typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y) boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z) boost::bind(&self_type::x, shared_from_this(),y,z)
enum {
    NEW_CONNECTION,
    WRITE_BUFFER,
    CLI_WRITE_BUFFER,
    CLOSE_CONNECTION,
    CLOSE_PEER_CNNS,
    
    NEW_CONNECTION_HOME,
    WRITE_BUFFER_HOME,
    CLOSE_CONNECTION_HOME,
    CLOSE_PEER_CNNS_HOME
};

enum
{
    max_length = 8 * 1024
};

extern TSQueue<std::string> cpp2java_que;
extern std::shared_ptr<boost::asio::io_context> g_io, g_socks_io, g_io_home;
extern std::string g_ms;
extern std::atomic<uint16_t> g_channel_id;
// extern boost::signals2::signal<void (uint16_t, const std::vector<uint8_t>& )> g_sig;