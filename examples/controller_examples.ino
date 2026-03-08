// ========================================
// CARDPUTER EXAMPLE CODE
// ========================================
// Send commands to Stack Kame via ESP-NOW

#include <esp_now.h>
#include <WiFi.h>
#include <M5Cardputer.h>

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

    // Initialize WiFi and ESP-NOW
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_send_cb(onDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, robotMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

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
            for (auto i : status.word) {
                if (keyStr != "") {
                    keyStr = keyStr + "+" + i;
                } else {
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
