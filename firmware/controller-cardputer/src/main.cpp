#include <M5Cardputer.h>
#include <M5Unified.h>
#include <WiFi.h>

#include <app_state.h>
#include <communication.h>
#include <screens.h>

namespace
{
constexpr uint8_t ESPNOW_CHANNEL = 1;
constexpr unsigned long INPUT_REPEAT_DELAY_MS = 260;
constexpr unsigned long INPUT_REPEAT_MS = 90;

const uint8_t ROBOT_MAC[6] = {0xF0, 0x9E, 0x9E, 0x74, 0x92, 0x88};

AppState appState;
CommandSender sender;

KeyInput decodeKeyInput()
{
  KeyInput input;

  if (!M5Cardputer.Keyboard.isPressed())
  {
    return input;
  }

  input.pressed = true;
  const Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

  String keyStr;
  for (const auto token : status.word)
  {
    if (!keyStr.isEmpty())
    {
      keyStr += "+";
    }
    keyStr += token;
  }

  /**
   * UI Navigation: uses the "arrow keys" but ignoring the fn modifier since it's hard to press together. 
   * The number keys set the speed level in the controls screen.
   */
  if (keyStr == ";" || keyStr == "w" || keyStr == "W") // ";" = Up arrow key on M5Stack Cardputer-ADV keyboard
  {
    input.action = Action::Up;
  }
  else if (keyStr == "." || keyStr == "s" || keyStr == "S") // "." = Down arrow key on M5Stack Cardputer-ADV keyboard
  {
    input.action = Action::Down;
  }
  else if (keyStr == "," || keyStr == "a" || keyStr == "A") // "," = Left arrow key on M5Stack Cardputer-ADV keyboard
  {
    input.action = Action::Left;
  }
  else if (keyStr == "/" || keyStr == "d" || keyStr == "D") // "/" = Right arrow key on M5Stack Cardputer-ADV keyboard
  {
    input.action = Action::Right;
  }
  else if (keyStr == "h" || keyStr == "H" ) // (On Controls screen) Send robot to "home" position
  {
    input.action = Action::Home;
  }
  else if (keyStr == "`" || keyStr == "x" || keyStr == "X") // "`" = Escape key on M5Stack Cardputer-ADV keyboard
  {
    input.action = Action::Emergency;
  }
  else if (keyStr.length() == 1 && keyStr[0] >= '0' && keyStr[0] <= '9')
  {
    input.number = keyStr[0] - '0';
  }

  if (status.enter)
  {
    input.action = Action::Select;
  }
  else if (status.del)
  {
    input.action = Action::Back;
  }

  return input;
}

bool isCalibrationJogInput(const KeyInput &input, const AppState &state)
{
  if (state.activeScreen != ScreenId::Calibration)
  {
    return false;
  }

  return input.action == Action::Left ||
         input.action == Action::Right ||
         input.action == Action::Up ||
         input.action == Action::Down;
}

void resetRepeatState(AppState &state)
{
  state.heldAction = Action::None;
  state.heldNumber = -1;
  state.lastRepeatMs = 0;
  state.repeatActive = false;
}

KeyInput readKeyInput(AppState &state, unsigned long now)
{
  KeyInput input = decodeKeyInput();
  if (!input.pressed)
  {
    resetRepeatState(state);
    return input;
  }

  const bool changed = M5Cardputer.Keyboard.isChange();
  const bool sameInput = (input.action == state.heldAction && input.number == state.heldNumber);
  if (changed || !sameInput)
  {
    state.heldAction = input.action;
    state.heldNumber = input.number;
    state.lastRepeatMs = now;
    state.repeatActive = false;
    return input;
  }

  if (isCalibrationJogInput(input, state))
  {
    const unsigned long interval = state.repeatActive ? INPUT_REPEAT_MS : INPUT_REPEAT_DELAY_MS;
    if (now - state.lastRepeatMs >= interval)
    {
      state.lastRepeatMs = now;
      state.repeatActive = true;
      return input;
    }
  }

  return KeyInput{};
}

bool macIsUnset(const uint8_t mac[6])
{
  for (size_t i = 0; i < 6; ++i)
  {
    if (mac[i] != 0)
    {
      return false;
    }
  }

  return true;
}
} // namespace

void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);

  Serial.begin(115200);
  delay(200);

  if (macIsUnset(ROBOT_MAC))
  {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(8, 12);
    M5Cardputer.Display.println("Set ROBOT_MAC in main.cpp");
    Serial.println("ERROR: ROBOT_MAC is unset.");
    return;
  }

  uint8_t localMac[6];
  WiFi.macAddress(localMac);
  Serial.printf("Cardputer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                localMac[0], localMac[1], localMac[2], localMac[3], localMac[4], localMac[5]);

  if (!sender.begin(ROBOT_MAC, ESPNOW_CHANNEL))
  {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(8, 12);
    M5Cardputer.Display.println("ESP-NOW setup failed");
    return;
  }

  appState.diagnostics.addLog("Controller booted");
  appState.diagnostics.addLog("ESP-NOW ready");
  appState.needsRedraw = true;
}

void loop()
{
  M5Cardputer.update();

  refreshDiagnostics(appState);

  const unsigned long now = millis();
  const KeyInput input = readKeyInput(appState, now);
  if (input.pressed)
  {
    appState.lastInputMs = now;
    handleInput(input, appState, sender);
  }

  if (appState.needsRedraw)
  {
    drawActiveScreen(appState);
    appState.needsRedraw = false;
  }

  delay(10);
}
