
#include <thread>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "util.h"
#include "udp.h"
#include "evt_sys.h"

using namespace std;
using boost::asio::ip::address;
using boost::asio::ip::udp;
UdpSvr::SsEp::SsEp(const std::string &a, const std::string& i, uint32_t t)
    : addr(a), id(i), token(t)
{
  auto v = Util::split(a, ":");
  if (v.size() == 2)
  {
    ep = udp::endpoint(address::from_string(v[0]), stoi(v[1]));
  }
  ping_data.resize(37);
  ping_data[0] = 0x18;
  t = htonl(token);
  memcpy(&ping_data[1], i.c_str(), 32);
  memcpy(&ping_data[33], &t, 4);
}
UdpSvr::UdpSvr(short port)
    : socket_(*g_io, udp::endpoint(udp::v4(), port))
{
  static boost::asio::deadline_timer routine_timer(*g_io, boost::posix_time::seconds(1));
  // can not use std::bind
  routine_timer.async_wait(boost::bind(&UdpSvr::routine, this, boost::asio::placeholders::error, &routine_timer));
  do_receive();
  std::string s = boost::lexical_cast<std::string>(socket_.local_endpoint() );
  LOGI("UdpSvr::UdpSvr(), local_endpoint=%s", s.c_str());
}
void UdpSvr::routine(const boost::system::error_code & /*e*/, boost::asio::deadline_timer *t)
{
  auto tid = Util::get_tid();
  
  for (auto&& s : svrs_)
  {
    do_send(s->ping_data, s->ep);
    LOGI("thread[%s] in udp routine, ping %s", tid.c_str(), s->addr.c_str());
  }
  t->expires_at(t->expires_at() + boost::posix_time::seconds(20));
  t->async_wait(boost::bind(&UdpSvr::routine, this,
                            boost::asio::placeholders::error, t));
}
void UdpSvr::on_svr(const std::string &svr_addr, const std::string& id, uint32_t token)
{
  off_svr(svr_addr);
  auto s = make_shared<SsEp>(svr_addr, id, token);
  svrs_.push_back(s);
  do_send(s->ping_data, s->ep);

}
void UdpSvr::off_svr(const std::string &svr_addr)
{
  auto i = std::find_if(svrs_.begin(), svrs_.end(), [svr_addr](const auto &s) { return s->addr == svr_addr; });
  if(i != svrs_.end())
  {
    svrs_.erase(i);
  }  
}
// channel_id(2 bytes), cmd_code(2 bytes), serial_no(4 bytes), payload
void UdpSvr::do_receive()
{
  socket_.async_receive_from(
      boost::asio::buffer(data_, max_length), sender_endpoint_,
      [this](boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0)
        {
          LOGI("do_receive, data=%s; from:%s", Util::byte2str(data_, bytes_recvd).c_str(), Util::ep_to_string(sender_endpoint_).c_str() );
          if(bytes_recvd < 8)
          {
            do_send(bytes_recvd);
          }
          else 
          {
            auto payload_len = bytes_recvd - 8;
            vector<uint8_t> payload(payload_len), echo;
            uint16_t cmd = 0, channel_id = 0;
            uint32_t serial_no = 0;
            memcpy(&channel_id, &data_[0], 2);
            memcpy(&cmd, &data_[2], 2);
            memcpy(&serial_no, &data_[4], 4);
            memcpy(&payload[0], &data_[8], payload_len);
            EventSys::instance().dispatch(channel_id, cmd, serial_no, payload);
            // echo.assign(data_, data_ + 8);
            // do_send(echo, sender_endpoint_);
          }
          
          // cout << "received: " << string(data_, bytes_recvd);
        }
        do_receive();
      });
}
void UdpSvr::do_send(std::vector<uint8_t> buff, boost::asio::ip::udp::endpoint &ep)
{
  socket_.async_send_to(
      boost::asio::buffer(buff), ep,
      [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {

      });
}
void UdpSvr::do_send(std::size_t length)
{
  socket_.async_send_to(
      boost::asio::buffer(data_, length), sender_endpoint_,
      [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
        do_receive();
      });
}

// echo 'hello world' | nc -u 192.168.1.96 57100