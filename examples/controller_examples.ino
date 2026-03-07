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
#define CMD_STOP           0x01
#define CMD_WALK_FORWARD   0x02
#define CMD_WALK_BACKWARD  0x03
#define CMD_TURN_LEFT      0x04
#define CMD_TURN_RIGHT     0x05
#define CMD_MOONWALK_FWD   0x06
#define CMD_MOONWALK_BACK  0x07
#define CMD_JUMP           0x08
#define CMD_SHUFFLE_LEFT   0x09
#define CMD_SHUFFLE_RIGHT  0x0A
#define CMD_EMERGENCY_STOP 0xFF

esp_now_peer_info_t peerInfo;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.print("Stack Kame Control");
  
  Serial.begin(115200);
  
  // Initialize WiFi and ESP-NOW
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  
  esp_now_register_send_cb(onDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, robotMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("Ready to send commands!");
}

void sendCommand(uint8_t type, uint8_t value) {
  uint8_t data[2] = {type, value};
  esp_err_t result = esp_now_send(robotMAC, data, 2);
  
  if (result == ESP_OK) {
    Serial.print("Sent: 0x");
    Serial.print(type, HEX);
    Serial.print(" Value: ");
    Serial.println(value);
  } else {
    Serial.println("Send error");
  }
}

void loop() {
  M5Cardputer.update();
  
  // Check keyboard input
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      
      // Arrow keys for movement
      if (status.word == "w" || status.word == "up") {
        sendCommand(CMD_WALK_FORWARD, 3);
        M5Cardputer.Display.println("Walk Forward");
      }
      else if (status.word == "s" || status.word == "down") {
        sendCommand(CMD_WALK_BACKWARD, 3);
        M5Cardputer.Display.println("Walk Back");
      }
      else if (status.word == "a" || status.word == "left") {
        sendCommand(CMD_TURN_LEFT, 2);
        M5Cardputer.Display.println("Turn Left");
      }
      else if (status.word == "d" || status.word == "right") {
        sendCommand(CMD_TURN_RIGHT, 2);
        M5Cardputer.Display.println("Turn Right");
      }
      // Special moves
      else if (status.word == "j") {
        sendCommand(CMD_JUMP, 0);
        M5Cardputer.Display.println("Jump!");
      }
      else if (status.word == "m") {
        sendCommand(CMD_MOONWALK_FWD, 2);
        M5Cardputer.Display.println("Moonwalk");
      }
      else if (status.word == "q") {
        sendCommand(CMD_SHUFFLE_LEFT, 2);
        M5Cardputer.Display.println("Shuffle Left");
      }
      else if (status.word == "e") {
        sendCommand(CMD_SHUFFLE_RIGHT, 2);
        M5Cardputer.Display.println("Shuffle Right");
      }
      // Stop commands
      else if (status.word == " " || status.word == "space") {
        sendCommand(CMD_STOP, 0);
        M5Cardputer.Display.println("Stop");
      }
      else if (status.word == "x") {
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
// Send commands to Stack Kame via I2C

/*
#include <Wire.h>

#define STACKKAME_ADDR 0x50

void setup() {
  Wire.begin(); // Initialize I2C as master
  Serial.begin(115200);
  Serial.println("Stack-Chan -> Stack Kame Control");
}

void sendKameCommand(uint8_t type, uint8_t value) {
  Wire.beginTransmission(STACKKAME_ADDR);
  Wire.write(type);
  Wire.write(value);
  Wire.endTransmission();
  
  Serial.print("Sent command: 0x");
  Serial.print(type, HEX);
  Serial.print(" Value: ");
  Serial.println(value);
}

uint8_t getKameStatus() {
  Wire.requestFrom(STACKKAME_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF; // Error
}

void loop() {
  // Example: Make the robot walk in a square
  
  // Walk forward
  sendKameCommand(0x02, 5); // Walk forward 5 steps
  delay(6000); // Wait for movement to complete
  
  // Turn left
  sendKameCommand(0x04, 2); // Turn left 2 steps
  delay(2500);
  
  // Check status
  uint8_t status = getKameStatus();
  Serial.print("Status: 0x");
  Serial.println(status, HEX);
  
  if (status & 0x02) {
    Serial.println("Warning: Low battery!");
  }
  
  delay(1000);
}
*/
