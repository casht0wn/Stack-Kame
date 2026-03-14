#pragma once

#include <Arduino.h>
#include <esp_now.h>

class CommandSender
{
public:
    bool begin(const uint8_t *peerMac, uint8_t channel);
    bool send(uint8_t type, uint8_t value) const;
    bool ready() const;

private:
    uint8_t peerMac_[6] = {0};
    uint8_t channel_ = 1;
    bool isReady_ = false;
};
