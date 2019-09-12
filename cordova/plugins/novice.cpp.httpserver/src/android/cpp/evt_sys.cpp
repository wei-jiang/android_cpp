#include "evt_sys.h"
#include "udp.h"
#include "util.h"

using namespace std;

void EventSys::on_evt(uint16_t ch, std::function<void(uint16_t, uint32_t, const std::vector<uint8_t>&)> handler)
{
    handlers_[ch] = handler;
}
void EventSys::off_evt(uint16_t ch)
{
    auto it = handlers_.find(ch);
    if(it != handlers_.end())
    {
        handlers_.erase(it);
    }
}
void EventSys::dispatch(uint16_t ch, uint16_t cmd, uint32_t serial_no, const std::vector<uint8_t>& payload)
{
    auto it = handlers_.find(ch);
    if(it != handlers_.end())
    {
        (it->second)(cmd, serial_no, payload);
    }
}