#pragma once
#include "common.h"



using boost::asio::ip::tcp;
using namespace std::chrono;


class Tunnel;
class HomeCnn1;

class HomeCnn1 : public std::enable_shared_from_this<HomeCnn1>, public CNNBase
{
    typedef HomeCnn1 self_type;
public:
    typedef std::map< uint32_t, std::shared_ptr<HomeCnn1> > ID2CNN;
    static std::map< std::string, ID2CNN> s_peer2cnn;
    static int s_home_port;
    HomeCnn1(Tunnel* t);
    ~HomeCnn1();
    void start(const std::string& pid, uint32_t s_no);
    void do_read();
    void on_read(boost::system::error_code ec, std::size_t length);
    void do_write(const std::vector<uint8_t>& data);
    void on_write(boost::system::error_code ec, std::size_t length);
    bool send_cmd2peer(uint8_t cmd);
    bool send_by_ws(const std::vector<uint8_t>& data);
    void remove_self(bool noty_flag = true);
private:
    void do_connect(std::function<void()> ok_handler = nullptr);
private:    
    std::string pid_;
    uint32_t s_no_;
    std::shared_ptr<tcp::socket> sock_;
    uint8_t read_buff_[max_length];
    std::vector<uint8_t> write_buff_;
    Tunnel *tunnel_;
    bool is_connected_;
};

class HomeCnn0 : public std::enable_shared_from_this<HomeCnn0>, public CNNBase
{   
    typedef HomeCnn0 self_type;
public:
    HomeCnn0(tcp::socket socket, Tunnel* t);
    ~HomeCnn0();
    void start();
    bool send_cmd2peer(uint8_t cmd);
    void remove_self(bool noty = true);
    void do_read();
    void on_read(boost::system::error_code ec, std::size_t length);
    void do_write(const std::vector<uint8_t>& data);    
    void on_write(boost::system::error_code ec, std::size_t length);
    bool send_by_ws(const std::vector<uint8_t>& data);
public:
    static std::map< uint32_t, std::shared_ptr<HomeCnn0> > s_id2cnn;
private:    
    Tunnel *tunnel_;
    
    uint8_t in_buf_[max_length];
    std::vector<uint8_t> out_buf_;
	tcp::socket socket_;
    uint32_t s_no_;
};


