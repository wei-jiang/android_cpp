#include "peer_handler.h"
#include "udp.h"
#include "util.h"
#include "evt_sys.h"

using namespace std;
using boost::asio::ip::address;
using boost::asio::ip::udp;
PeerHandler::PeerHandler(std::shared_ptr<UdpSvr> udp, std::string ep)
:udp_(udp),routine_timer(*g_io, boost::posix_time::seconds(0)),channel_id_(g_channel_id++),serial_no_(0)
{
    auto v = Util::split(ep, ":");
    if (v.size() == 2)
    {
        ep_ = udp::endpoint(address::from_string(v[0]), stoi(v[1]));
    }
    routine_timer.async_wait( boost::bind(&PeerHandler::routine, this, boost::asio::placeholders::error) );
    EventSys::instance().on_evt(channel_id_, std::bind(&PeerHandler::handle_msg, this, ph::_1, ph::_2, ph::_3) );
}
PeerHandler::~PeerHandler()
{
    LOGI("in PeerHandler::~PeerHandler()");     
}
void PeerHandler::before_destroy()
{
    EventSys::instance().off_evt(channel_id_);
    routine_timer.cancel();
    LOGI("in PeerHandler::before_destroy()"); 
}
void PeerHandler::handle_msg(uint16_t cmd, uint32_t serial_no, const std::vector<uint8_t>& payload)
{
    LOGI("handle_msg, cmd=%d; serial_no=%d", cmd, serial_no); 
}
void PeerHandler::routine(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted) 
    {
        LOGI("PeerHandler::routine(), routine_timer cancelled"); 
        return; //cancelled
    }
    auto tid = Util::get_tid();
    // LOGI("thread[%s] in routine...", tid.c_str()); 
    // channel_id(2 bytes), cmd_code(2 bytes), serial_no(4 bytes), payload
    std::vector<uint8_t> data(8);
    memcpy(&data[0], &channel_id_, 2);
    uint16_t cmd = HEART_BEAT;
    memcpy(&data[2], &cmd, 2);
    uint32_t sn = serial_no_++;
    memcpy(&data[4], &sn, 4);
    udp_->do_send(data, ep_);
    LOGI("PeerHandler::routine send heartbeat=%s to %s", Util::byte2str(data).c_str(), Util::ep_to_string(ep_).c_str() ); 
    routine_timer.expires_at(routine_timer.expires_at() + boost::posix_time::seconds(5));
    routine_timer.async_wait(boost::bind(&PeerHandler::routine, this, boost::asio::placeholders::error));
}
