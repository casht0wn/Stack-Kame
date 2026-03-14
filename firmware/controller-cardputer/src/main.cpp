#include <M5Cardputer.h>
#include <M5Unified.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <ui_widgets.h>

Button btnMenu, btnCalibration, btnDiagnostics, btnControls;
MenuScreen menuScreen;
CalibrationScreen calibrationScreen;
DiagnosticsScreen diagnosticsScreen;
MainControlScreen controlScreen;

#define ESPNOW_CHANNEL 1

// ROBOT MAC Address (update with your robot's MAC)
uint8_t robotMAC[] = {0xF0, 0x9E, 0x9E, 0x74, 0x92, 0x88}; // Update this!

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
#define CMD_HOME 0x0B
#define CMD_ZERO 0x0C
#define CMD_EMERGENCY_STOP 0xFF

esp_now_peer_info_t peerInfo;

void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);

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

  // Setup buttons
  btnMenu.label = "MENU";
  btnMenu.x = 60;
  btnMenu.y = 20;
  btnMenu.w = 120;
  btnMenu.h = 40;

  btnControls.label = "Controls";
  btnControls.x = 60;
  btnControls.y = 70;
  btnControls.w = 120;
  btnControls.h = 40;

  btnCalibration.label = "Calibration";
  btnCalibration.x = 10;
  btnCalibration.y = 45;
  btnCalibration.w = 40;
  btnCalibration.h = 40;

  btnDiagnostics.label = "Diagnostics";
  btnDiagnostics.x = 190;
  btnDiagnostics.y = 45;
  btnDiagnostics.w = 40;
  btnDiagnostics.h = 40;

  menuScreen.buttons.push_back(btnControls);
  menuScreen.buttons.push_back(btnCalibration);
  menuScreen.buttons.push_back(btnDiagnostics);

  menuScreen.draw();
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
  menuScreen.update();
  menuScreen.draw();

  Screen *currentScreen = nullptr;
  if (menuScreen.selectedIndex == 0)
  {
    currentScreen = &controlScreen;
  }
  else if (menuScreen.selectedIndex == 1)
  {
    currentScreen = &calibrationScreen;
  }
  else if (menuScreen.selectedIndex == 2)
  {
    currentScreen = &diagnosticsScreen;
  }

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

      // Arrow keys for UI movement
      if (keyStr == "UP")
      {
        menuScreen.selectPrevious();
      }
      else if (keyStr == "DOWN")
      {
        menuScreen.selectNext();
      }
      else if (keyStr == "ENTER")
      {
        // Switch to selected screen
        menuScreen.buttons[menuScreen.selectedIndex].onPress();
      }

      Serial.println("Key Pressed: " + keyStr);
    }
  }
  delay(10);
}
