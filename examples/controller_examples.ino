// ========================================
// CARDPUTER EXAMPLE CODE
// ========================================
// Send commands to Stack Kame via ESP-NOW

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <M5Cardputer.h>

#define ESPNOW_CHANNEL 1

// Stack Kame MAC Address (update with your robot's MAC)
uint8_t robotMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Update this!

// Command definitions
#define CMD_STOP 0x01
#define CMD_WALK_FORWARD 0x02
#define CMD_WALK_BACKWARD 0x03
#define CMD_TURN_LEFT 0x04
#define CMD_TURN_RIGHT 0x05
#define CMD_MOONWALK_FWD 0x06
#define CMD_MOONWALK_BACK 0x07
#define CMD_JUMP 0x08
#define CMD_SHUFFLE_LEFT 0x09
#define CMD_SHUFFLE_RIGHT 0x0A
#define CMD_EMERGENCY_STOP 0xFF

esp_now_peer_info_t peerInfo;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.println("Stack Kame Control");

    Serial.begin(115200);
    delay(200);

    if (robotMAC[0] == 0x00 && robotMAC[1] == 0x00 && robotMAC[2] == 0x00 &&
        robotMAC[3] == 0x00 && robotMAC[4] == 0x00 && robotMAC[5] == 0x00)
    {
        Serial.println("ERROR: robotMAC is still 00:00:00:00:00:00. Update robotMAC first.");
        M5Cardputer.Display.println("Set robotMAC!");
        return;
    }

    // Initialize WiFi and ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setSleep(false);

    uint8_t localMac[6];
    WiFi.macAddress(localMac);
    Serial.printf("Cardputer STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  localMac[0], localMac[1], localMac[2], localMac[3], localMac[4], localMac[5]);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    Serial.printf("ESP-NOW channel locked to: %d\n", ESPNOW_CHANNEL);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_send_cb(onDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, robotMAC, 6);
    peerInfo.channel = ESPNOW_CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

    Serial.printf("Target robot MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  robotMAC[0], robotMAC[1], robotMAC[2], robotMAC[3], robotMAC[4], robotMAC[5]);
    Serial.println("Ready to send commands!");
}

void sendCommand(uint8_t type, uint8_t value)
{
    uint8_t data[2] = {type, value};
    esp_err_t result = esp_now_send(robotMAC, data, 2);

    if (result == ESP_OK)
    {
        Serial.print("Sent: 0x");
        Serial.print(type, HEX);
        Serial.print(" Value: ");
        Serial.println(value);
    }
    else
    {
        Serial.println("Send error");
    }
}

void loop()
{
    M5Cardputer.update();

    // Check keyboard input
    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            String keyStr = "";
            for (auto i : status.word)
            {
                if (keyStr != "")
                {
                    keyStr = keyStr + "+" + i;
                }
                else
                {
                    keyStr = i;
                }
            }

            // Arrow keys for movement
            if (keyStr == "w" || keyStr == "up")
            {
                sendCommand(CMD_WALK_FORWARD, 3);
                M5Cardputer.Display.println("Walk Forward");
            }
            else if (keyStr == "s" || keyStr == "down")
            {
                sendCommand(CMD_WALK_BACKWARD, 3);
                M5Cardputer.Display.println("Walk Back");
            }
            else if (keyStr == "a" || keyStr == "left")
            {
                sendCommand(CMD_TURN_LEFT, 2);
                M5Cardputer.Display.println("Turn Left");
            }
            else if (keyStr == "d" || keyStr == "right")
            {
                sendCommand(CMD_TURN_RIGHT, 2);
                M5Cardputer.Display.println("Turn Right");
            }
            // Special moves
            else if (keyStr == "j")
            {
                sendCommand(CMD_JUMP, 0);
                M5Cardputer.Display.println("Jump!");
            }
            else if (keyStr == "m")
            {
                sendCommand(CMD_MOONWALK_FWD, 2);
                M5Cardputer.Display.println("Moonwalk");
            }
            else if (keyStr == "q")
            {
                sendCommand(CMD_SHUFFLE_LEFT, 2);
                M5Cardputer.Display.println("Shuffle Left");
            }
            else if (keyStr == "e")
            {
                sendCommand(CMD_SHUFFLE_RIGHT, 2);
                M5Cardputer.Display.println("Shuffle Right");
            }
            // Stop commands
            else if (keyStr == " " || keyStr == "space")
            {
                sendCommand(CMD_STOP, 0);
                M5Cardputer.Display.println("Stop");
            }
            else if (keyStr == "x")
            {
                sendCommand(CMD_EMERGENCY_STOP, 0);
                M5Cardputer.Display.println("EMERGENCY!");
            }
        }
    }

    delay(10);
}

// ========================================
// STACK-CHAN EXAMPLE CODE
// ========================================
// Send commands to Stack Kame via ESP-NOW
// (Both Stack-Chan and robot mounted on same chassis)

/*
#include <esp_now.h>
#include <WiFi.h>

// Stack Kame MAC Address (update with your robot's MAC)
uint8_t robotMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Update this!

// Command definitions
#define CMD_STOP 0x01
#define CMD_WALK_FORWARD 0x02
#define CMD_WALK_BACKWARD 0x03
#define CMD_TURN_LEFT 0x04
#define CMD_TURN_RIGHT 0x05
#define CMD_MOONWALK_FWD 0x06
#define CMD_MOONWALK_BACK 0x07
#define CMD_JUMP 0x08
#define CMD_SHUFFLE_LEFT 0x09
#define CMD_SHUFFLE_RIGHT 0x0A
#define CMD_EMERGENCY_STOP 0xFF

esp_now_peer_info_t peerInfo;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Command sent: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Failed");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Stack-Chan -> Stack Kame Control (ESP-NOW)");

  // Initialize WiFi and ESP-NOW
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  // Register peer (Stack Kame)
  memcpy(peerInfo.peer_addr, robotMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Ready!");
}

void sendKameCommand(uint8_t type, uint8_t value) {
  uint8_t data[2] = {type, value};
  esp_err_t result = esp_now_send(robotMAC, data, 2);

  Serial.print("Sent: 0x");
  Serial.print(type, HEX);
  Serial.print(" Value: ");
  Serial.println(value);
}

void loop() {
  // Example: Make the robot walk in a square

  // Walk forward
  sendKameCommand(CMD_WALK_FORWARD, 5); // Walk forward 5 steps
  delay(6000); // Wait for movement to complete

  // Turn left
  sendKameCommand(CMD_TURN_LEFT, 2); // Turn left 2 steps
  delay(2500);

  delay(1000);
}
*/
