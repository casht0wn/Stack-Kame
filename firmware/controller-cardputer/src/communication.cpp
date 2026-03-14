#include <communication.h>

#include <WiFi.h>
#include <esp_wifi.h>

bool CommandSender::begin(const uint8_t *peerMac, uint8_t channel)
{
    channel_ = channel;
    memcpy(peerMac_, peerMac, sizeof(peerMac_));

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setSleep(false);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel_, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed");
        isReady_ = false;
        return false;
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerMac_, sizeof(peerMac_));
    peerInfo.channel = channel_;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add ESP-NOW peer");
        isReady_ = false;
        return false;
    }

    Serial.printf("ESP-NOW ready on channel %d\n", channel_);
    Serial.printf("Robot peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  peerMac_[0], peerMac_[1], peerMac_[2], peerMac_[3], peerMac_[4], peerMac_[5]);

    isReady_ = true;
    return true;
}

bool CommandSender::send(uint8_t type, uint8_t value) const
{
    if (!isReady_)
    {
        return false;
    }

    const uint8_t packet[2] = {type, value};
    const esp_err_t result = esp_now_send(peerMac_, packet, sizeof(packet));
    if (result != ESP_OK)
    {
        Serial.printf("Send failed: cmd=0x%02X value=%u err=%d\n", type, value, static_cast<int>(result));
        return false;
    }

    Serial.printf("Sent cmd=0x%02X value=%u\n", type, value);
    return true;
}

bool CommandSender::ready() const
{
    return isReady_;
}
