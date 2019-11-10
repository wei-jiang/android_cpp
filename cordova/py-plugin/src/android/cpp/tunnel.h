#pragma once
#include "common.h"

using boost::asio::ip::tcp;
using namespace std::chrono;

class Tunnel;
class SocksCnn1;
typedef std::map< uint32_t, std::shared_ptr<SocksCnn1> > ID2CNN;
class SocksCnn1 : public std::enable_shared_from_this<SocksCnn1>, public CNNBase
{
    typedef SocksCnn1 self_type;
public:
    static std::map< std::string, ID2CNN> s_peer2cnn;
    SocksCnn1(Tunnel* t);
    ~SocksCnn1();
    void start(const std::string& pid, uint32_t s_no);
    void do_read();
    void on_read(boost::system::error_code ec, std::size_t length);
    void do_write(const std::vector<uint8_t>& data);
    void on_write(boost::system::error_code ec, std::size_t length);
    bool send_cmd2peer(uint8_t cmd);
    bool send_by_ws(const std::vector<uint8_t>& data);
    void remove_self(bool noty_flag = true);
private:
    std::string pid_;
    uint32_t s_no_;
    tcp::socket sock_;
    uint8_t read_buff_[max_length];
    std::vector<uint8_t> write_buff_;
    Tunnel *tunnel_;
};

class SocksCnn0 : public std::enable_shared_from_this<SocksCnn0>, public CNNBase
{   
    typedef SocksCnn0 self_type;
public:
    SocksCnn0(tcp::socket socket, Tunnel* t);
    ~SocksCnn0();
    void start();
    bool send_cmd2peer(uint8_t cmd);
    void remove_self(bool noty = true);
    void do_read();
    void on_read(boost::system::error_code ec, std::size_t length);
    void do_write(const std::vector<uint8_t>& data);    
    void on_write(boost::system::error_code ec, std::size_t length);
    bool send_by_ws(const std::vector<uint8_t>& data);
public:
    static std::map< uint32_t, std::shared_ptr<SocksCnn0> > s_id2cnn;
private:    
    Tunnel *tunnel_;
    
    uint8_t in_buf_[max_length];
    std::vector<uint8_t> out_buf_;
	tcp::socket socket_;
    uint32_t s_no_;
};

class Tunnel : public Singleton<Tunnel>
{
    // std::map< uint16_t, std::function<void(uint16_t, uint32_t, const std::vector<uint8_t>&)> > handlers_;
public:
    void bind_handlers(WsServer::Endpoint *ws_ep);
    void start_socks(int port);
    void start_home(int port);
    void on_open(std::shared_ptr<WsServer::Connection> connection);
    void on_close(std::shared_ptr<WsServer::Connection> connection, int status, const std::string &reason);
    void on_error(std::shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec);
    void on_message(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message);
    bool send(const std::vector<uint8_t>& data);
private:
    void do_accept_socks();
    void do_accept_home();
private:
    WsServer::Endpoint *ws_ep_;
    int port_socks_, port_home_;
	std::shared_ptr<tcp::acceptor> acceptor_socks_, acceptor_home_;
	
};
extern int g_socks_port;

