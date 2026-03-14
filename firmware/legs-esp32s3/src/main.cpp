#include <Arduino.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Preferences.h>
#include "stackkame.h"

#define ESPNOW_CHANNEL 1

// Command Queue
#define QUEUE_SIZE 16
struct Command
{
  byte type;
  byte value;
  unsigned long timestamp;
};

Command commandQueue[QUEUE_SIZE];
int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// Global objects
StackKame kame;
Preferences preferences;

// State variables
bool emergencyStop = false;
int selectedServo = 0;
int selectedAngle = 90;

// Function declarations
void onESPNowReceive(const uint8_t *mac, const uint8_t *data, int len);
bool enqueueCommand(byte type, byte value);
bool dequeueCommand(Command &cmd);
void processCommand(byte type, byte value);
void loadCalibration();
void saveCalibration();

void setup()
{
  Serial.begin(115200);
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart < 5000))
  {
    delay(10);
  }
  Serial.println("\n\nStack Kame Boot Starting...");
  delay(500);

  // Initialize preferences for calibration storage
  Serial.println("[1/5] Initializing preferences...");
  preferences.begin("stackkame", false);
  delay(100);

  // Initialize StackKame servo controller
  Serial.println("[2/5] Initializing servo controller...");
  kame.init();
  delay(100);

  // Set servos to home position
  Serial.println("[3/5] Moving servos to home position...");
  kame.home();
  delay(500);

  // Load calibration from preferences
  Serial.println("[4/5] Loading calibration...");
  loadCalibration();
  delay(100);

  // Initialize ESP-NOW for wireless control (Cardputer + Stack-Chan)
  Serial.println("[5/5] Initializing ESP-NOW...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);

  uint8_t localMac[6];
  WiFi.macAddress(localMac);
  Serial.printf("Robot STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                localMac[0], localMac[1], localMac[2], localMac[3], localMac[4], localMac[5]);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  Serial.printf("ESP-NOW channel locked to: %d\n", ESPNOW_CHANNEL);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
  }
  else
  {
    Serial.println("ESP-NOW initialized");
    esp_err_t cbResult = esp_now_register_recv_cb(onESPNowReceive);
    if (cbResult != ESP_OK)
    {
      Serial.printf("ESP-NOW receive callback registration failed: %d\n", cbResult);
    }
    else
    {
      Serial.println("ESP-NOW receive callback registered");
    }
  }
  delay(100);

  Serial.println("\n=== Stack Kame Ready ===");
  Serial.println("Serial commands: h=home, z=zero, w=walk, t=turn, s=stop, r=resume, q=queue");
  Serial.println("Calibration: 0-7=select servo, +=jog +5, -=jog -5, p=print selected");
  Serial.println("Waiting for commands...\n");
}

void loop()
{
  // Update oscillators for smooth movements
  kame.update();

  // Process command queue
  if (queueCount > 0 && !emergencyStop)
  {
    Command cmd;
    if (dequeueCommand(cmd))
    {
      processCommand(cmd.type, cmd.value);
    }
  }

  // Handle serial commands for debugging
  if (Serial.available() > 0)
  {
    char cmd = Serial.read();
    switch (cmd)
    {
    case 'h':
      Serial.println("Moving to home position");
      kame.home();
      break;
    case 'z':
      Serial.println("Zeroing servos");
      kame.zero();
      break;
    case 's':
      Serial.println("Emergency stop!");
      emergencyStop = true;
      kame.stopMovement();
      break;
    case 'r':
      Serial.println("Resume operation");
      emergencyStop = false;
      break;
    case 'c':
      Serial.println("Entering calibration mode - send trim values");
      // TODO: Implement interactive calibration
      break;
    case 'w':
      Serial.println("Walk forward 3 steps");
      enqueueCommand(0x02, 3);
      break;
    case 'b':
      Serial.println("Walk backward 3 steps");
      enqueueCommand(0x03, 3);
      break;
    case 't':
      Serial.println("Turn left");
      enqueueCommand(0x04, 1);
      break;
    case 'q':
      Serial.print("Queue count: ");
      Serial.println(queueCount);
      break;
    case '+':
      selectedAngle = constrain(selectedAngle + 5, 0, 180);
      Serial.print("Jog servo ");
      Serial.print(selectedServo);
      Serial.print(" to ");
      Serial.println(selectedAngle);
      kame.setServo(selectedServo, selectedAngle);
      break;
    case '-':
      selectedAngle = constrain(selectedAngle - 5, 0, 180);
      Serial.print("Jog servo ");
      Serial.print(selectedServo);
      Serial.print(" to ");
      Serial.println(selectedAngle);
      kame.setServo(selectedServo, selectedAngle);
      break;
    case 'p':
      Serial.print("Selected servo: ");
      Serial.print(selectedServo);
      Serial.print(", angle: ");
      Serial.println(selectedAngle);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      selectedServo = cmd - '0';
      selectedAngle = 90;
      Serial.print("Selected servo ");
      Serial.print(selectedServo);
      Serial.println(" (set to 90)");
      kame.setServo(selectedServo, selectedAngle);
      break;
    }
  }

  delay(10); // Small delay to prevent overwhelming the system
}

// ESP-NOW Receive Callback - receives commands from Cardputer or Stack-Chan
void onESPNowReceive(const uint8_t *mac, const uint8_t *data, int len)
{
  if (len >= 2)
  {
    byte cmdType = data[0];
    byte cmdValue = data[1];

    Serial.print("ESP-NOW Command from: ");
    for (int i = 0; i < 6; i++)
    {
      Serial.printf("%02X", mac[i]);
      if (i < 5)
        Serial.print(":");
    }
    Serial.print(" - Type: 0x");
    Serial.print(cmdType, HEX);
    Serial.print(" Value: ");
    Serial.println(cmdValue);

    enqueueCommand(cmdType, cmdValue);
  }
}

// Add command to queue
bool enqueueCommand(byte type, byte value)
{
  if (queueCount >= QUEUE_SIZE)
  {
    Serial.println("Warning: Command queue full!");
    return false;
  }

  commandQueue[queueTail].type = type;
  commandQueue[queueTail].value = value;
  commandQueue[queueTail].timestamp = millis();

  queueTail = (queueTail + 1) % QUEUE_SIZE;
  queueCount++;

  return true;
}

// Remove command from queue
bool dequeueCommand(Command &cmd)
{
  if (queueCount == 0)
  {
    return false;
  }

  cmd = commandQueue[queueHead];
  queueHead = (queueHead + 1) % QUEUE_SIZE;
  queueCount--;

  return true;
}

// Process a movement command
void processCommand(byte type, byte value)
{
  if (emergencyStop)
  {
    Serial.println("Emergency stop active - command ignored");
    return;
  }

  Serial.print("Processing command: 0x");
  Serial.print(type, HEX);
  Serial.print(" Value: ");
  Serial.println(value);

  switch (type)
  {
  case 0x01: // Stop
    kame.stopMovement();
    kame.home();
    Serial.println("Stop & Home");
    break;

  case 0x02: // Walk forward
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Walk forward ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.walk(steps, 900, false);
  }
  break;

  case 0x03: // Walk backward
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Walk backward ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.walk(steps, 900, true);
  }
  break;

  case 0x04: // Turn left
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Turn left ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.turn(steps, 900, true);
  }
  break;

  case 0x05: // Turn right
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Turn right ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.turn(steps, 900, false);
  }
  break;

  case 0x06: // Moonwalk forward
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Moonwalk forward ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.moonwalk(steps, 1000, false);
  }
  break;

  case 0x07: // Moonwalk backward
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Moonwalk backward ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.moonwalk(steps, 1000, true);
  }
  break;

  case 0x08: // Jump
    Serial.println("Jump!");
    kame.jump();
    break;

  case 0x09: // Lateral shuffle left
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Lateral shuffle left ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.lateral_fuerte(true, steps, 900);
  }
  break;

  case 0x0A: // Lateral shuffle right
  {
    int steps = (value > 0) ? value : 1;
    Serial.print("Lateral shuffle right ");
    Serial.print(steps);
    Serial.println(" steps");
    kame.lateral_fuerte(false, steps, 900);
  }
  break;

  case 0x0B: // home position
  {
    Serial.println("Moving to home position");
    kame.home();
  }
  break;

  case 0x0C: // zero position
  {
    Serial.println("Zeroing servos");
    kame.zero();
  }
  break;

  case 0xFF: // Emergency stop
    Serial.println("Emergency Stop Command!");
    emergencyStop = true;
    kame.stopMovement();
    break;

  default:
    Serial.print("Unknown command: 0x");
    Serial.println(type, HEX);
    break;
  }
}

// Load servo trim calibration from preferences
void loadCalibration()
{
  int trim0 = preferences.getInt("trim0", 0);
  int trim1 = preferences.getInt("trim1", 0);
  int trim2 = preferences.getInt("trim2", 0);
  int trim3 = preferences.getInt("trim3", 0);
  int trim4 = preferences.getInt("trim4", 0);
  int trim5 = preferences.getInt("trim5", 0);
  int trim6 = preferences.getInt("trim6", 0);
  int trim7 = preferences.getInt("trim7", 0);

  kame.setTrims(trim0, trim1, trim2, trim3, trim4, trim5, trim6, trim7);

  Serial.println("Calibration loaded from preferences");
  Serial.print("Trims: ");
  Serial.print(trim0);
  Serial.print(", ");
  Serial.print(trim1);
  Serial.print(", ");
  Serial.print(trim2);
  Serial.print(", ");
  Serial.print(trim3);
  Serial.print(", ");
  Serial.print(trim4);
  Serial.print(", ");
  Serial.print(trim5);
  Serial.print(", ");
  Serial.print(trim6);
  Serial.print(", ");
  Serial.println(trim7);
}

// Save servo trim calibration to preferences
void saveCalibration()
{
  // TODO: Implement interactive calibration and save
  Serial.println("Calibration saved to preferences");
}
