
#include <thread>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "util.h"
#include "udp.h"

using namespace std;
using boost::asio::ip::udp;
using boost::asio::ip::address;
UdpSvr::SsEp::SsEp(const std::string& a, int32_t i, uint32_t t)
:addr(a), id(i), token(t)
{
 auto v = Util::split(a, ":");
 if(v.size() == 2)
 {
   ep = udp::endpoint(address::from_string(v[0]), stoi(v[1]) );
 }
 
}
UdpSvr::UdpSvr(short port)
    : socket_(*g_io, udp::endpoint(udp::v4(), port))
{
  do_receive();
}
void UdpSvr::on_svr(const std::string& svr_addr, int32_t id, uint32_t token)
{
  svrs_.push_back( make_shared<SsEp>( svr_addr, id, token ) );
}
void UdpSvr::off_svr(const std::string& svr_addr)
{
  auto i = std::find_if( svrs_.begin(), svrs_.end(), [svr_addr](const auto& s){ return s->addr == svr_addr; } );
  svrs_.erase(i);
}
void UdpSvr::do_receive()
{
  socket_.async_receive_from(
      boost::asio::buffer(data_, max_length), sender_endpoint_,
      [this](boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0)
        {
          do_send( bytes_recvd );
          cout<<"received: " << string(data_, bytes_recvd);
        }
        else
        {
          do_receive();
        }
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