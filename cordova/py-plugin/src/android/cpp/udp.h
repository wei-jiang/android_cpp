#pragma once

#include <boost/asio.hpp>

// mtu 1500 bytes (1472 bytes for payload )?
// ping -M do -s 1472 baidu.com -c 1 [996 pass]
// ping -s $((1500 - 28)) -M do 8.8.8.8 -c 1
// ping -s 992 -D 8.8.8.8 -c 1  [pass, and 993 failed]
class UdpSvr
{    
    struct SsEp
    {
        SsEp(const std::string& a, const std::string& i, uint32_t t);
        std::string addr;
        std::string id; 
        uint32_t token;
        boost::asio::ip::udp::endpoint ep;
        std::vector<uint8_t> ping_data;
    };
  public:
    UdpSvr(short port);
    void do_receive();
    void do_send(std::size_t length);
    void do_send(std::vector<uint8_t> buff, boost::asio::ip::udp::endpoint& ep);
    void on_svr(const std::string& svr_addr, const std::string& id, uint32_t token);
    void off_svr(const std::string& svr_addr);
  private:
    void routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t);
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    enum
    {
        max_length = 4 * 1024
    };
    uint8_t data_[max_length];
    std::vector<std::shared_ptr<SsEp>> svrs_;
};
