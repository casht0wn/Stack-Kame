#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MAX1704X.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Preferences.h>
#include "stackkame.h"

// I2C Slave Configuration
#define I2C_SLAVE_ADDR 0x50
#define I2C_SDA 21
#define I2C_SCL 20

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
Adafruit_MAX17048 maxlipo;
Preferences preferences;

// State variables
bool emergencyStop = false;
float batteryVoltage = 0.0;
bool lowBattery = false;

// Function declarations
void onI2CReceive(int numBytes);
void onI2CRequest();
void onESPNowReceive(const uint8_t *mac, const uint8_t *data, int len);
bool enqueueCommand(byte type, byte value);
bool dequeueCommand(Command &cmd);
void processCommand(byte type, byte value);
void checkBattery();
void loadCalibration();
void saveCalibration();

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 3000)
    ; // Wait up to 3 seconds for serial
  Serial.println("Stack Kame Quadruped Robot Initializing...");

  // Initialize preferences for calibration storage
  preferences.begin("stackkame", false);

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize MAX17048 battery monitor
  if (!maxlipo.begin(&Wire))
  {
    Serial.println("Warning: Failed to find MAX17048 battery monitor");
  }
  else
  {
    Serial.println("Battery monitor initialized");
  }

  // Initialize StackKame
  Serial.println("Initializing servo controller...");
  kame.init();

  // Load calibration from preferences
  loadCalibration();

  // Setup I2C slave mode for Stack-Chan communication
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);

  // Initialize ESP-NOW for Cardputer communication
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
  }
  else
  {
    Serial.println("ESP-NOW initialized");
    esp_now_register_recv_cb(onESPNowReceive);
  }

  Serial.println("Stack Kame Ready!");
  Serial.println("Waiting for commands...");
  Serial.println("Serial commands: h=home, z=zero, s=stop, c=calibrate");
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

  // Check battery status periodically
  static unsigned long lastBatteryCheck = 0;
  if (millis() - lastBatteryCheck > 10000)
  { // Check every 10 seconds
    checkBattery();
    lastBatteryCheck = millis();
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
    }
  }

  delay(10); // Small delay to prevent overwhelming the system
}

// I2C Receive Callback - called when Stack-Chan sends data
void onI2CReceive(int numBytes)
{
  if (numBytes >= 2)
  {
    byte cmdType = Wire.read();
    byte cmdValue = Wire.read();

    // Consume remaining bytes
    while (Wire.available())
    {
      Wire.read();
    }

    Serial.print("I2C Command received: 0x");
    Serial.print(cmdType, HEX);
    Serial.print(" Value: ");
    Serial.println(cmdValue);

    enqueueCommand(cmdType, cmdValue);
  }
}

// I2C Request Callback - called when Stack-Chan requests status
void onI2CRequest()
{
  byte status = 0x00;

  if (emergencyStop)
  {
    status |= 0x01; // Bit 0: Emergency stop
  }
  if (lowBattery)
  {
    status |= 0x02; // Bit 1: Low battery
  }
  if (queueCount > 0)
  {
    status |= 0x04; // Bit 2: Command in queue
  }

  Wire.write(status);
}

// ESP-NOW Receive Callback - called when Cardputer sends data
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

  if (lowBattery && type != 0x01)
  { // Allow stop command even on low battery
    Serial.println("Low battery - movement restricted");
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

// Check battery voltage and set low battery flag
void checkBattery()
{
  batteryVoltage = maxlipo.cellVoltage();
  float cellPercent = maxlipo.cellPercent();

  Serial.print("Battery: ");
  Serial.print(batteryVoltage);
  Serial.print("V (");
  Serial.print(cellPercent);
  Serial.println("%)");

  // Set low battery flag if below 3.4V or 20%
  if (batteryVoltage < 3.4 || cellPercent < 20.0)
  {
    if (!lowBattery)
    {
      Serial.println("WARNING: Low battery!");
      lowBattery = true;
    }
  }
  else
  {
    lowBattery = false;
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
