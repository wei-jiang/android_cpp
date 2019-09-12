#pragma once
#include "common.h"

enum {
    HEART_BEAT
};

class UdpSvr;

class PeerHandler
{
public:
    PeerHandler(std::shared_ptr<UdpSvr> udp, std::string ep);
    ~PeerHandler();   
    void routine(const boost::system::error_code& ec);
    void handle_msg(uint16_t cmd, uint32_t serial_no, const std::vector<uint8_t>& payload);
    void before_destroy();
private:
    std::shared_ptr<UdpSvr> udp_;
    boost::asio::ip::udp::endpoint ep_;
    boost::asio::deadline_timer routine_timer;
    uint16_t channel_id_;
    uint32_t serial_no_;
};


