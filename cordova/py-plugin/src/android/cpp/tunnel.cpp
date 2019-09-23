#include "tunnel.h"
#include "util.h"

// | cmd 2bytes | direction 1byte | cli_id(md5 string) 32bytes | uuid 16bytes |
// buffer length 2bytes | raw buffer n bytes|
using namespace std;
std::map< std::vector<uint8_t>, std::shared_ptr<RtcCnn> > RtcCnn::s_uuid2cnn;
std::map< std::string, UUID2CNN> SocksClient::s_peer2cnn;
SocksClient::SocksClient(Tunnel* t)
:sock_(*g_socks_io), tunnel_(t)
{

}
SocksClient::~SocksClient() {
    LOGI("SocksClient::~SocksClient()");
    sock_.close();
}
void SocksClient::start(const std::string& pid, const std::vector<uint8_t>& uuid)
{
    auto self(shared_from_this());
    pid_ = pid;
    uuid_ = uuid;
    SocksClient::s_peer2cnn[pid_][uuid_] = self;
    tcp::endpoint ep( boost::asio::ip::address::from_string("127.0.0.1"), g_socks_port);
    sock_.async_connect(ep, 
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
              send_cmd2peer(NEW_CONNECTION);
              do_read();
          }
          else
          {             
              remove_self();
          }
        });
}
void SocksClient::do_write(const std::vector<uint8_t>& data)
{
    write_buff_ = data;
    // LOGI("SocksClient::do_write, data=%s", Util::byte2str(write_buff_).c_str());
    boost::asio::async_write(sock_, boost::asio::buffer(write_buff_),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            send_cmd2peer(WRITE_BUFFER);
          }
          else
          {
            remove_self();
          }         
        });
}
void SocksClient::do_read()
{
    sock_.async_receive(boost::asio::buffer(read_buff_, max_length),
                            [this](boost::system::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    std::vector<uint8_t> buff(50+2+length);
                                    uint16_t len = length;
                                    buff[0] = CLI_WRITE_BUFFER;
                                    buff[1] = 1;
                                    memcpy(&buff[2], &pid_[0], 32);
                                    memcpy(&buff[34], &uuid_[0], 16);
                                    memcpy(&buff[50], &len, 2);
                                    memcpy(&buff[52], &read_buff_[0], len);
                                    // LOGI("SocksClient::do_read, data=%s", Util::byte2str(read_buff_, len).c_str());
                                    if( !tunnel_->send(buff) )
                                    {
                                        remove_self();
                                    }
                                }
                                else
                                {
                                    remove_self();
                                }                                
                            });
}
bool SocksClient::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+16 = 50
    std::vector<uint8_t> buff(50);
    buff[0] = cmd;
    buff[1] = 1;
    memcpy(&buff[2], &pid_[0], 32);
    memcpy(&buff[34], &uuid_[0], 16);
    return tunnel_->send(buff);
}
void SocksClient::remove_self(bool noty_flag)
{
    if(noty_flag) send_cmd2peer(CLOSE_CONNECTION);
    SocksClient::s_peer2cnn[pid_].erase(uuid_);
}

RtcCnn::RtcCnn(tcp::socket socket, Tunnel* t)
:tunnel_(t), socket_( std::move(socket) ), uuid_( Util::uuid() )
{}
RtcCnn::~RtcCnn()
{
    LOGI("RtcCnn::~RtcCnn()");
    socket_.close();
}
void RtcCnn::start()
{
    auto self(shared_from_this());
    RtcCnn::s_uuid2cnn[uuid_] = self;

    // via ws -> webrtc -> peer's ws -> tunnel.cpp
    if( !send_cmd2peer(NEW_CONNECTION) )
    {
        remove_self();
    }
}
bool RtcCnn::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+16 = 50
    std::vector<uint8_t> buff(50);
    buff[0] = cmd;
    buff[1] = 0;
    memcpy(&buff[34], &uuid_[0], 16);
    return tunnel_->send(buff);
}
void RtcCnn::remove_self(bool noty)
{
    if(noty) send_cmd2peer(CLOSE_CONNECTION);
    RtcCnn::s_uuid2cnn.erase(uuid_);
}
void RtcCnn::do_read()
{
    socket_.async_read_some(boost::asio::buffer(in_buf_, max_length),
                            [this](boost::system::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    std::vector<uint8_t> buff(50+2+length);
                                    uint16_t len = length;
                                    buff[0] = WRITE_BUFFER;
                                    buff[1] = 0;
                                    memcpy(&buff[34], &uuid_[0], 16);
                                    memcpy(&buff[50], &len, 2);
                                    memcpy(&buff[52], &in_buf_[0], len);
                                    // LOGI("RtcCnn::do_read, data=%s", Util::byte2str(in_buf_, len).c_str());
                                    if( !tunnel_->send(buff) )
                                    {
                                        remove_self();
                                    }
                                }
                                else
                                {
                                    remove_self();
                                }
                                
                            });
}
void RtcCnn::do_write(const std::vector<uint8_t> &data)
{
    out_buf_ = data;
    // LOGI("RtcCnn::do_write, data=%s", Util::byte2str(out_buf_).c_str());
    boost::asio::async_write(socket_, boost::asio::buffer(out_buf_),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            send_cmd2peer(CLI_WRITE_BUFFER);
          }
          else
          {
              remove_self();
          }
          
        });
}
void Tunnel::bind_handlers(WsServer::Endpoint *ws_ep)
{
    ws_ep_ = ws_ep;
    ws_ep->on_open = std::bind(&Tunnel::on_open, this, ph::_1);
    ws_ep->on_close = std::bind(&Tunnel::on_close, this, ph::_1, ph::_2, ph::_3);
    ws_ep->on_error = std::bind(&Tunnel::on_error, this, ph::_1, ph::_2);
    ws_ep->on_message = std::bind(&Tunnel::on_message, this, ph::_1, ph::_2);
    // LOGI("Tunnel::bind_handlers");
}
void Tunnel::start(int port)
{
    LOGI("Tunnel::start, port=%d", port);
    port_ = port;
    acceptor_ = make_shared<tcp::acceptor>(*g_socks_io, tcp::endpoint(tcp::v4(), port));
    do_accept();
}
bool Tunnel::send(const std::vector<uint8_t>& data)
{
    auto const &cnn_set = ws_ep_->get_connections();
    if(cnn_set.size() > 0)
    {
        std::string buff(data.begin(), data.end());
        for (auto &a_connection : cnn_set)
        {
            a_connection->send(buff, nullptr, 130);
            // LOGI( "Tunnel::send, buffer=%s", Util::hexStr(buff).c_str() );
        }            
        return true;
    }
    return false;
    
}
void Tunnel::on_open(std::shared_ptr<WsServer::Connection> connection)
{
}
void Tunnel::on_close(std::shared_ptr<WsServer::Connection> connection, int status, const std::string &reason)
{
}
void Tunnel::on_error(std::shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec)
{
}
void Tunnel::on_message(std::shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message)
{
    auto buff = in_message->string();
    // LOGI( "Tunnel::on_message, buffer=%s", Util::hexStr(buff).c_str() );
    uint8_t cmd = buff[0];
    uint8_t dir = buff[1];
    string pid(buff.begin()+2, buff.begin()+34);
    vector<uint8_t> uuid(buff.begin()+34, buff.begin()+50);
    if(dir == 0)
    {
        switch (cmd)
        {
        case NEW_CONNECTION:
            std::make_shared<SocksClient>(this)->start(pid, uuid);
            break;
        case CLI_WRITE_BUFFER:
            {
                auto psc = SocksClient::s_peer2cnn[pid][uuid];
                if(psc) psc->do_read(); 
            }             
            break;
        case WRITE_BUFFER:
            {
                auto psc = SocksClient::s_peer2cnn[pid][uuid];
                if(psc)
                {
                    uint16_t len = 0;
                    memcpy(&len, &buff[50], 2);
                    auto it = buff.begin() + 52;
                    psc->do_write( vector<uint8_t>(it, it + len) );
                }
            }           
            break;
        case CLOSE_CONNECTION:
            {
                auto psc = SocksClient::s_peer2cnn[pid][uuid];
                if(psc)
                {
                    psc->remove_self( false );
                } 
            }                     
            break;
        case CLOSE_PEER_CNNS:
            {
                auto it = SocksClient::s_peer2cnn.find(pid);
                if(it != SocksClient::s_peer2cnn.end() )
                {
                    auto &peer_cnns = SocksClient::s_peer2cnn[pid];
                    peer_cnns.clear();
                    // for( auto const& [k, v] : symbolTable )
                    // {
                    //     if(v)
                    //     {
                    //         v->remove_self( false );
                    //     } 
                    // }
                    SocksClient::s_peer2cnn.erase(pid); 
                }                  
            }    
            break;
        default:
            break;
        }
    }
    else
    {
        // dir == 1
        switch (cmd)
        {
        case NEW_CONNECTION:
        case WRITE_BUFFER:
            {
                auto pcnn = RtcCnn::s_uuid2cnn[uuid];
                if(pcnn) pcnn->do_read();
            }             
            break;
        case CLI_WRITE_BUFFER:
            {
                auto pcnn = RtcCnn::s_uuid2cnn[uuid];
                if(pcnn) 
                {
                    uint16_t len = 0;
                    memcpy(&len, &buff[50], 2);
                    auto it = buff.begin() + 52;
                    pcnn->do_write( vector<uint8_t>(it, it + len) );
                } 
            }           
            break;
        case CLOSE_CONNECTION:
            {
                auto pcnn = RtcCnn::s_uuid2cnn[uuid];
                if(pcnn)
                {
                    pcnn->remove_self( false );
                } 
            }                     
            break;
        case CLOSE_PEER_CNNS:
            {
                RtcCnn::s_uuid2cnn.clear();   
            }    
            break;
        default:
            break;
        }
    }
    
}
void Tunnel::do_accept()
{
    acceptor_->async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
            {                
                std::make_shared<RtcCnn>(std::move(socket), this)->start();
            }
            do_accept();
        });
}