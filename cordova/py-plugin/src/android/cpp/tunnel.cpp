#include "tunnel.h"
#include "util.h"
#include "home_cnn.h"
// | cmd 1bytes | direction 1byte | cli_id(md5 string) 32bytes | serial_no 4bytes |
// buffer length 2bytes | raw buffer n bytes|
using namespace std;
std::map< uint32_t, std::shared_ptr<SocksCnn0> > SocksCnn0::s_id2cnn;
std::map< std::string, ID2CNN> SocksCnn1::s_peer2cnn;
SocksCnn1::SocksCnn1(Tunnel* t)
:sock_(*g_io), tunnel_(t)
{

}
SocksCnn1::~SocksCnn1() {
    // LOGI("SocksCnn1::~SocksCnn1()");    
}
void SocksCnn1::start(const std::string& pid, uint32_t s_no)
{
    auto self(shared_from_this());
    pid_ = pid;
    s_no_ = s_no;
    SocksCnn1::s_peer2cnn[pid_][s_no_] = self;
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
          if(ec != boost::asio::error::operation_aborted)
          {             
              remove_self();
          }
        });
}
void SocksCnn1::do_write(const std::vector<uint8_t>& data)
{
    write_buff_ = data;
    // LOGI("SocksCnn1::do_write, data=%s", Util::byte2str(write_buff_).c_str());
    boost::asio::async_write(sock_, boost::asio::buffer(write_buff_), MEM_FN2(on_write, _1, _2));
}
void SocksCnn1::on_write(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        send_cmd2peer(WRITE_BUFFER);
    }
    else
    if(ec != boost::asio::error::operation_aborted)
    {
        remove_self();
    }  
}
void SocksCnn1::on_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        std::vector<uint8_t> buff(38+2+length);
        uint16_t len = length;
        buff[0] = CLI_WRITE_BUFFER;
        buff[1] = 1;
        memcpy(&buff[2], &pid_[0], 32);
        memcpy(&buff[34], &s_no_, 4);
        memcpy(&buff[38], &len, 2);
        memcpy(&buff[40], &read_buff_[0], len);
        // LOGI("SocksCnn1::do_read, data=%s", Util::byte2str(read_buff_, len).c_str());
        if( !send_by_ws(buff) )
        {
            remove_self();
        }
    }
    else
    if(ec != boost::asio::error::operation_aborted)
    {
        remove_self();
    }     
}
void SocksCnn1::do_read()
{
    sock_.async_receive(boost::asio::buffer(read_buff_, max_length), MEM_FN2(on_read, _1, _2));
}
bool SocksCnn1::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+4 = 38
    std::vector<uint8_t> buff(38);
    buff[0] = cmd;
    buff[1] = 1;
    memcpy(&buff[2], &pid_[0], 32);
    memcpy(&buff[34], &s_no_, 4);
    return send_by_ws(buff);
}
void SocksCnn1::remove_self(bool noty_flag)
{
    if(noty_flag) send_cmd2peer(CLOSE_CONNECTION);
    auto i = SocksCnn1::s_peer2cnn.find(pid_);
    if(SocksCnn1::s_peer2cnn.end() != i)
    {
        i->second.erase(s_no_);
    }
    boost::system::error_code ec;
    sock_.close(ec);
}
bool SocksCnn1::send_by_ws(const std::vector<uint8_t>& data)
{
    reset();
    return tunnel_->send(data);
}
SocksCnn0::SocksCnn0(tcp::socket socket, Tunnel* t)
:tunnel_(t), socket_( std::move(socket) ), s_no_( Util::serial_no() )
{}
SocksCnn0::~SocksCnn0()
{
    // LOGI("SocksCnn0::~SocksCnn0()");
}
void SocksCnn0::start()
{
    auto self(shared_from_this());
    SocksCnn0::s_id2cnn[s_no_] = self;

    // via ws -> webrtc -> peer's ws -> tunnel.cpp
    if( !send_cmd2peer(NEW_CONNECTION) )
    {
        remove_self(false);
    }
}
bool SocksCnn0::send_by_ws(const std::vector<uint8_t>& data)
{
    reset();
    return tunnel_->send(data);
}
bool SocksCnn0::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+4 = 38
    std::vector<uint8_t> buff(38);
    buff[0] = cmd;
    buff[1] = 0;
    memcpy(&buff[34], &s_no_, 4);
    return send_by_ws(buff);
}
void SocksCnn0::remove_self(bool noty)
{
    if(noty) send_cmd2peer(CLOSE_CONNECTION);
    SocksCnn0::s_id2cnn.erase(s_no_);
    boost::system::error_code ec;
    socket_.close(ec);
}
void SocksCnn0::on_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        std::vector<uint8_t> buff(38+2+length);
        uint16_t len = length;
        buff[0] = WRITE_BUFFER;
        buff[1] = 0;
        memcpy(&buff[34], &s_no_, 4);
        memcpy(&buff[38], &len, 2);
        memcpy(&buff[40], &in_buf_[0], len);
        // LOGI("SocksCnn0::do_read, data=%s", Util::byte2str(in_buf_, len).c_str());
        if( !send_by_ws(buff) )
        {
            remove_self();
        }
    }
    else
    if(ec != boost::asio::error::operation_aborted)
    {
        remove_self();
    }
}
void SocksCnn0::do_read()
{
    socket_.async_read_some(boost::asio::buffer(in_buf_, max_length), MEM_FN2(on_read, _1, _2));
}
void SocksCnn0::on_write(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        send_cmd2peer(CLI_WRITE_BUFFER);
    }
    else
    if(ec != boost::asio::error::operation_aborted)
    {
        remove_self();
    }
}
void SocksCnn0::do_write(const std::vector<uint8_t> &data)
{
    out_buf_ = data;
    // LOGI("SocksCnn0::do_write, data=%s", Util::byte2str(out_buf_).c_str());
    boost::asio::async_write(socket_, boost::asio::buffer(out_buf_), MEM_FN2(on_write, _1, _2));
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
void Tunnel::start_socks(int port)
{    
    port_socks_ = port + 100;
    LOGI("Tunnel::start_socks, port=%d", port_socks_);
    acceptor_socks_ = make_shared<tcp::acceptor>(*g_io, tcp::endpoint(tcp::v4(), port_socks_));
    do_accept_socks();
}
void Tunnel::start_home(int port)
{    
    HomeCnn1::s_home_port = port + 1;
    port_home_ = port + 100;
    LOGI("Tunnel::start_home, port=%d", port_home_);
    acceptor_home_ = make_shared<tcp::acceptor>(*g_io, tcp::endpoint(tcp::v4(), port_home_));
    do_accept_home();
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
    uint32_t s_no;
    memcpy(&s_no, &buff[34], 4);
    if(dir == 0)
    {
        switch (cmd)
        {
        case NEW_CONNECTION:
            std::make_shared<SocksCnn1>(this)->start(pid, s_no);
            break;
        case NEW_CONNECTION_HOME:
            // LOGI( "Tunnel::on_message, NEW_CONNECTION_HOME, buffer=%s", Util::hexStr(buff).c_str() );
            std::make_shared<HomeCnn1>(this)->start(pid, s_no);
            break;
        case CLI_WRITE_BUFFER:
            {
                auto i = SocksCnn1::s_peer2cnn.find(pid);
                if(i != SocksCnn1::s_peer2cnn.end() )
                {
                    auto &id_map = i->second;
                    auto ii = id_map.find(s_no);
                    if(ii != id_map.end())
                    {
                        auto psc = ii->second;
                        psc->do_read(); 
                    }
                    else 
                    {
                        LOGI( "CLI_WRITE_BUFFER,dir==0, pid=%s, s_no=%u no found", pid.c_str(), s_no );
                    }
                }
                else 
                {
                    LOGI( "CLI_WRITE_BUFFER,dir==0, pid=%s no found", pid.c_str() );
                }
            }             
            break;
        case WRITE_BUFFER:
            {
                auto i = SocksCnn1::s_peer2cnn.find(pid);
                if(SocksCnn1::s_peer2cnn.end() != i)
                {
                    auto &id_map = i->second;
                    auto ii = id_map.find(s_no);
                    if(ii != id_map.end())
                    {
                        auto psc = ii->second;
                        uint16_t len = 0;
                        memcpy(&len, &buff[38], 2);
                        auto it = buff.begin() + 40;
                        psc->do_write( vector<uint8_t>(it, it + len) );
                    }
                    else 
                    {
                        LOGI( "WRITE_BUFFER, dir==0, pid=%s, s_no=%u no found", pid.c_str(), s_no );
                    }
                }
                else
                {
                    LOGI( "WRITE_BUFFER, dir==0, pid=%s no found", pid.c_str() );
                }
 
            }           
            break;
        case WRITE_BUFFER_HOME:
            {
                auto i = HomeCnn1::s_peer2cnn.find(pid);
                if(HomeCnn1::s_peer2cnn.end() != i)
                {
                    auto &id_map = i->second;
                    auto ii = id_map.find(s_no);
                    if(ii != id_map.end())
                    {
                        auto psc = ii->second;
                        uint16_t len = 0;
                        memcpy(&len, &buff[38], 2);
                        auto it = buff.begin() + 40;
                        psc->do_write( vector<uint8_t>(it, it + len) );
                    }
                    else 
                    {
                        LOGI( "WRITE_BUFFER_HOME, dir==0, pid=%s, s_no=%u no found", pid.c_str(), s_no );
                    }
                }
                else
                {
                    LOGI( "WRITE_BUFFER_HOME, dir==0, pid=%s no found", pid.c_str() );
                }
 
            }           
            break;
        case CLOSE_CONNECTION:
            {
                auto i = SocksCnn1::s_peer2cnn.find(pid);
                if(i != SocksCnn1::s_peer2cnn.end() )
                {
                    auto &id_map = i->second;
                    auto ii = id_map.find(s_no);
                    if(ii != id_map.end())
                    {
                        auto psc = ii->second;
                        psc->remove_self( false ); 
                    }
                    else 
                    {
                        LOGI( "CLOSE_CONNECTION, dir==0, pid=%s, s_no=%u no found", pid.c_str(), s_no );
                    }
                }
                else 
                {
                    LOGI( "CLOSE_CONNECTION, dir==0, pid=%s no found", pid.c_str() );
                }
            }                     
            break;
        case CLOSE_CONNECTION_HOME:
            {
                auto i = HomeCnn1::s_peer2cnn.find(pid);
                if(i != HomeCnn1::s_peer2cnn.end() )
                {
                    auto &id_map = i->second;
                    auto ii = id_map.find(s_no);
                    if(ii != id_map.end())
                    {
                        auto psc = ii->second;
                        psc->remove_self(false); 
                        LOGI( "CLOSE_CONNECTION_HOME, dir==0, pid=%s, s_no=%u close HomeCnn1", pid.c_str(), s_no );
                    }
                    else 
                    {
                        LOGI( "CLOSE_CONNECTION_HOME, dir==0, pid=%s, s_no=%u no found", pid.c_str(), s_no );
                    }
                }
                else 
                {
                    LOGI( "CLOSE_CONNECTION_HOME, dir==0, pid=%s no found", pid.c_str() );
                }
            }                     
            break;
        case CLOSE_PEER_CNNS:
            {
                // LOGI( "Tunnel::on_message, CLOSE_PEER_CNNS, pid=%s", pid.c_str() );
                auto it = SocksCnn1::s_peer2cnn.find(pid);
                if(it != SocksCnn1::s_peer2cnn.end() )
                {
                    auto &peer_cnns = it->second;
                    // peer_cnns.clear();
                    for( auto const& [k, v] : peer_cnns )
                    {
                        if(v)
                        {
                            v->remove_self(false);
                        } 
                    }
                    SocksCnn1::s_peer2cnn.erase(pid); 
                }                  
            }    
            break;
        case CLOSE_PEER_CNNS_HOME:
            {
                // LOGI( "Tunnel::on_message, CLOSE_PEER_CNNS, pid=%s", pid.c_str() );
                auto it = HomeCnn1::s_peer2cnn.find(pid);
                if(it != HomeCnn1::s_peer2cnn.end() )
                {
                    auto &peer_cnns = it->second;
                    for( auto const& [k, v] : peer_cnns )
                    {
                        if(v)
                        {
                            v->remove_self(false);
                        } 
                    }
                    HomeCnn1::s_peer2cnn.erase(pid); 
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
                auto i = SocksCnn0::s_id2cnn.find(s_no);
                if(i != SocksCnn0::s_id2cnn.end() )
                {
                    auto &pcnn = i->second;
                    pcnn->do_read();
                }
                else 
                {
                    LOGI( "WRITE_BUFFER/NEW_CONNECTION, dir==1, s_no=%u no found", s_no );
                }
            }             
            break;
        case CLI_WRITE_BUFFER:
            {
                auto i = SocksCnn0::s_id2cnn.find(s_no);
                if(i != SocksCnn0::s_id2cnn.end() )
                {
                    auto &pcnn = i->second;
                    uint16_t len = 0;
                    memcpy(&len, &buff[38], 2);
                    auto it = buff.begin() + 40;
                    pcnn->do_write( vector<uint8_t>(it, it + len) );
                }
                else 
                {
                    LOGI( "CLI_WRITE_BUFFER, dir==1, s_no=%u no found", s_no );
                }
            }           
            break;
        case WRITE_BUFFER_HOME:
            {
                auto i = HomeCnn0::s_id2cnn.find(s_no);
                if(i != HomeCnn0::s_id2cnn.end() )
                {
                    auto &pcnn = i->second;
                    uint16_t len = 0;
                    memcpy(&len, &buff[38], 2);
                    auto it = buff.begin() + 40;
                    pcnn->do_write( vector<uint8_t>(it, it + len) );
                }
                else 
                {
                    LOGI( "WRITE_BUFFER_HOME, dir==1, s_no=%u no found", s_no );
                }
            }           
            break;
        case CLOSE_CONNECTION:
            {
                auto i = SocksCnn0::s_id2cnn.find(s_no);
                if(i != SocksCnn0::s_id2cnn.end() )
                {
                    auto &pcnn = i->second;
                    pcnn->remove_self( false );
                }
                else 
                {
                    LOGI( "CLOSE_CONNECTION, dir==1, s_no=%u no found", s_no );
                }
            }                     
            break;
        case CLOSE_CONNECTION_HOME:
            {
                auto i = HomeCnn0::s_id2cnn.find(s_no);
                if(i != HomeCnn0::s_id2cnn.end() )
                {
                    auto &pcnn = i->second;
                    pcnn->remove_self( false );
                }
                else 
                {
                    LOGI( "CLOSE_CONNECTION_HOME, dir==1, s_no=%u no found", s_no );
                }
            }                     
            break;
        case CLOSE_PEER_CNNS:
            {
                for( auto const& [k, v] : SocksCnn0::s_id2cnn )
                {
                    if(v)
                    {
                        v->remove_self(false);
                    } 
                }
            }    
            break;
        case CLOSE_PEER_CNNS_HOME:
            {
                for( auto const& [k, v] : HomeCnn0::s_id2cnn )
                {
                    if(v)
                    {
                        v->remove_self(false);
                    } 
                }
            }    
            break;
        default:
            break;
        }
    }
    
}
void Tunnel::do_accept_socks()
{
    acceptor_socks_->async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
            {                
                std::make_shared<SocksCnn0>(std::move(socket), this)->start();
            }
            do_accept_socks();
        });
}
void Tunnel::do_accept_home()
{
    acceptor_home_->async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
            {                
                std::make_shared<HomeCnn0>(std::move(socket), this)->start();
            }
            do_accept_home();
        });
}