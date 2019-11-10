#include "tunnel.h"
#include "home_cnn.h"
#include "util.h"

// | cmd 1bytes | direction 1byte | cli_id(md5 string) 32bytes | serial_no 4bytes |
// buffer length 2bytes | raw buffer n bytes|
using namespace std;
std::map< uint32_t, std::shared_ptr<HomeCnn0> > HomeCnn0::s_id2cnn;
std::map< std::string, HomeCnn1::ID2CNN> HomeCnn1::s_peer2cnn;
int HomeCnn1::s_home_port = 57100;
HomeCnn1::HomeCnn1(Tunnel* t)
:tunnel_(t), is_connected_(false)
{
    
}
HomeCnn1::~HomeCnn1() {
    // LOGI("HomeCnn1::~HomeCnn1()");
}
void HomeCnn1::do_connect(std::function<void()> ok_handler)
{
    sock_.reset(new tcp::socket(*g_io));
    tcp::endpoint ep( boost::asio::ip::address::from_string("127.0.0.1"), s_home_port);
    sock_->async_connect(ep, 
        [this, ok_handler](boost::system::error_code ec)
        {
            if (!ec)
            {
                boost::asio::socket_base::keep_alive option(true);
                sock_->set_option(option);
                LOGI("HomeCnn1::start, async_connect success");
                //   send_cmd2peer(NEW_CONNECTION_HOME);
                is_connected_ = true;
                if(ok_handler) ok_handler();
                do_read();
            }
            else
            {             
                LOGI("HomeCnn1::do_connect, async_connect failed, send_cmd2peer(CLOSE_CONNECTION_HOME)");
                send_cmd2peer(CLOSE_CONNECTION_HOME);
            }
        });
}
void HomeCnn1::start(const std::string& pid, uint32_t s_no)
{
    auto self(shared_from_this());
    pid_ = pid;
    s_no_ = s_no;
    // LOGI("HomeCnn1::start, pid=%s, s_no_=%u", pid_.c_str(), s_no_);
    HomeCnn1::s_peer2cnn[pid_][s_no_] = self;
    do_connect();
}
void HomeCnn1::do_write(const std::vector<uint8_t>& data)
{
    write_buff_ = data;
    // LOGI("HomeCnn1::do_write, data=%s", Util::byte2str(write_buff_).c_str());
    if(is_connected_)
    {
        boost::asio::async_write(*sock_, boost::asio::buffer(write_buff_), MEM_FN2(on_write, _1, _2));
    }
    else 
    {
        do_connect([this]{
            boost::asio::async_write(*sock_, boost::asio::buffer(write_buff_), MEM_FN2(on_write, _1, _2));
        });
    }   
}
void HomeCnn1::on_write(boost::system::error_code ec, std::size_t length)
{
    if (ec && ec != boost::asio::error::operation_aborted)
    {
        is_connected_ = false;
    }  
}
void HomeCnn1::on_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        std::vector<uint8_t> buff(38+2+length);
        uint16_t len = length;
        buff[0] = WRITE_BUFFER_HOME;
        buff[1] = 1;
        memcpy(&buff[2], &pid_[0], 32);
        memcpy(&buff[34], &s_no_, 4);
        memcpy(&buff[38], &len, 2);
        memcpy(&buff[40], &read_buff_[0], len);
        // LOGI("HomeCnn1::do_read, data=%s", Util::byte2str(read_buff_, len).c_str());
        if( !send_by_ws(buff) )
        {
            LOGI("HomeCnn1::on_read, send_by_ws(buff) failed");
            remove_self();
        }
        else 
        {
            // LOGI("HomeCnn1::on_read, send_by_ws(buff) success, read next");
            do_read();
        }
    }
    else
    {
        LOGI("HomeCnn1::on_read, failed, ec=%s, set is_connected_ = false", ec.message().c_str());
        is_connected_ = false;
    }     
}
void HomeCnn1::do_read()
{
    sock_->async_receive(boost::asio::buffer(read_buff_, max_length), MEM_FN2(on_read, _1, _2));
}
bool HomeCnn1::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+4 = 38
    std::vector<uint8_t> buff(38);
    buff[0] = cmd;
    buff[1] = 1;
    memcpy(&buff[2], &pid_[0], 32);
    memcpy(&buff[34], &s_no_, 4);
    return send_by_ws(buff);
}
void HomeCnn1::remove_self(bool noty_flag)
{   
    if(noty_flag) send_cmd2peer(CLOSE_CONNECTION_HOME);
    auto i = HomeCnn1::s_peer2cnn.find(pid_);
    if(HomeCnn1::s_peer2cnn.end() != i)
    {
        i->second.erase(s_no_);
    }
    LOGI("HomeCnn1::remove_self, pid=%s, s_no_=%u", pid_.c_str(), s_no_);
    boost::system::error_code ec;
    sock_->close(ec);
}
bool HomeCnn1::send_by_ws(const std::vector<uint8_t>& data)
{
    reset();
    return tunnel_->send(data);
}
HomeCnn0::HomeCnn0(tcp::socket socket, Tunnel* t)
:tunnel_(t), socket_( std::move(socket) ), s_no_( Util::serial_no() )
{}
HomeCnn0::~HomeCnn0()
{
    // LOGI("HomeCnn0::~HomeCnn0()");
}
void HomeCnn0::start()
{
    auto self(shared_from_this());
    HomeCnn0::s_id2cnn[s_no_] = self;

    // via ws -> webrtc -> peer's ws -> tunnel.cpp
    if( !send_cmd2peer(NEW_CONNECTION_HOME) )
    {
        LOGI("HomeCnn0::start, send_cmd2peer failed");
        remove_self(false);
    }
    else 
    {
        LOGI("HomeCnn0::start, send_cmd2peer success, do_read()");
        do_read();
    }
}
bool HomeCnn0::send_by_ws(const std::vector<uint8_t>& data)
{
    reset();
    return tunnel_->send(data);
}
bool HomeCnn0::send_cmd2peer(uint8_t cmd)
{
    // 1+1+32+4 = 38
    std::vector<uint8_t> buff(38);
    buff[0] = cmd;
    buff[1] = 0;
    memcpy(&buff[34], &s_no_, 4);
    return send_by_ws(buff);
}
void HomeCnn0::remove_self(bool noty)
{
    if(noty) send_cmd2peer(CLOSE_CONNECTION_HOME);
    HomeCnn0::s_id2cnn.erase(s_no_);
    boost::system::error_code ec;
    socket_.close(ec);
    LOGI("HomeCnn0::remove_self, s_no_=%u", s_no_);
}
void HomeCnn0::on_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        std::vector<uint8_t> buff(38+2+length);
        uint16_t len = length;
        buff[0] = WRITE_BUFFER_HOME;
        buff[1] = 0;
        memcpy(&buff[34], &s_no_, 4);
        memcpy(&buff[38], &len, 2);
        memcpy(&buff[40], &in_buf_[0], len);
        // LOGI("HomeCnn0::do_read, data=%s", Util::byte2str(in_buf_, len).c_str());
        if( !send_by_ws(buff) )
        {
            LOGI("HomeCnn0::on_read, send_by_ws(buff) failed");
            remove_self();
        }
        else
        {
            // LOGI("HomeCnn0::on_read, send_by_ws(buff) success, read next");
            do_read();
        }
        
    }
    else
    if(ec != boost::asio::error::operation_aborted)
    {
        LOGI("HomeCnn0::on_read, failed, remove_self(), ec=%s", ec.message().c_str());
        remove_self();
    }
}
void HomeCnn0::do_read()
{
    socket_.async_read_some(boost::asio::buffer(in_buf_, max_length), MEM_FN2(on_read, _1, _2));
}
void HomeCnn0::on_write(boost::system::error_code ec, std::size_t length)
{
    if (ec && ec != boost::asio::error::operation_aborted)
    {
        remove_self();
    }
}
void HomeCnn0::do_write(const std::vector<uint8_t> &data)
{
    out_buf_ = data;
    // LOGI("HomeCnn0::do_write, data=%s", Util::byte2str(out_buf_).c_str());
    boost::asio::async_write(socket_, boost::asio::buffer(out_buf_), MEM_FN2(on_write, _1, _2));
}
