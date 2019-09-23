#pragma once
#include "common.h"

using boost::asio::ip::tcp;
enum {
    NEW_CONNECTION,
    WRITE_BUFFER,
    CLI_WRITE_BUFFER,
    CLOSE_CONNECTION,
    CLOSE_PEER_CNNS
};
enum
{
    max_length = 16 * 1024
};
class Tunnel;
class SocksClient;
typedef std::map< std::vector<uint8_t>, std::shared_ptr<SocksClient> > UUID2CNN;
class SocksClient : public std::enable_shared_from_this<SocksClient>
{
public:
    static std::map< std::string, UUID2CNN> s_peer2cnn;
    SocksClient(Tunnel* t);
    ~SocksClient();
    void start(const std::string& pid, const std::vector<uint8_t>& uuid);
    void do_read();
    void do_write(const std::vector<uint8_t>& data);
    bool send_cmd2peer(uint8_t cmd);
    void remove_self(bool noty_flag = true);
private:
    std::string pid_;
    std::vector<uint8_t> uuid_;
    tcp::socket sock_;
    uint8_t read_buff_[max_length];
    std::vector<uint8_t> write_buff_;
    Tunnel *tunnel_;
};

class RtcCnn : public std::enable_shared_from_this<RtcCnn>
{   
public:
    RtcCnn(tcp::socket socket, Tunnel* t);
    ~RtcCnn();
    void start();
    bool send_cmd2peer(uint8_t cmd);
    void remove_self(bool noty = true);
    void do_read();
    void do_write(const std::vector<uint8_t>& data);    
public:
    static std::map< std::vector<uint8_t>, std::shared_ptr<RtcCnn> > s_uuid2cnn;
private:    
    Tunnel *tunnel_;
    
    uint8_t in_buf_[max_length];
    std::vector<uint8_t> out_buf_;
	tcp::socket socket_;
    std::vector<uint8_t> uuid_;
};

class Tunnel : public Singleton<Tunnel>
{
    // std::map< uint16_t, std::function<void(uint16_t, uint32_t, const std::vector<uint8_t>&)> > handlers_;
public:
    void bind_handlers(WsServer::Endpoint *ws_ep);
    void start(int port);
    void on_open(std::shared_ptr<WsServer::Connection> connection);
    void on_close(std::shared_ptr<WsServer::Connection> connection, int status, const std::string &reason);
    void on_error(std::shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec);
    void on_message(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message);
    bool send(const std::vector<uint8_t>& data);
private:
    void do_accept();
private:
    WsServer::Endpoint *ws_ep_;
    int port_;
	std::shared_ptr<tcp::acceptor> acceptor_;
	
};
extern int g_socks_port;

