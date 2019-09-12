#pragma once
#include "common.h"


class EventSys : public Singleton<EventSys>
{
    std::map< uint16_t, std::function<void(uint16_t, uint32_t, const std::vector<uint8_t>&)> > handlers_;
public:
    void on_evt(uint16_t ch, std::function<void(uint16_t, uint32_t, const std::vector<uint8_t>&)> handler);
    void off_evt(uint16_t ch);
    void dispatch(uint16_t ch, uint16_t cmd, uint32_t serial_no, const std::vector<uint8_t>& payload);
};


