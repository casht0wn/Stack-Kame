#include <screens.h>

#include <M5Cardputer.h>
#include <M5Unified.h>
#include <WiFi.h>

#include <commands.h>

namespace
{
constexpr int MENU_COUNT = 3;
constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 135;
constexpr int PAD = 8;
constexpr int HEADER_Y = 6;
constexpr int HEADER_H = 20;
constexpr int FOOTER_Y = 122;
constexpr uint16_t PANEL_BG = 0x10A2;
constexpr uint16_t PANEL_ALT_BG = 0x18E3;
constexpr uint16_t PANEL_BORDER = 0x6D7A;
constexpr uint16_t ACCENT = 0x2DF7;
constexpr uint16_t ACCENT_OK = 0x87F0;
constexpr uint16_t ACCENT_FAIL = 0xFD20;

String clipText(const String &value, size_t maxLen)
{
    if (value.length() <= maxLen)
    {
        return value;
    }

    if (maxLen <= 3)
    {
        return value.substring(0, maxLen);
    }

    return value.substring(0, maxLen - 3) + "...";
}

String formatUptime(uint32_t uptimeSec)
{
    const uint32_t hours = uptimeSec / 3600UL;
    const uint32_t minutes = (uptimeSec % 3600UL) / 60UL;
    const uint32_t seconds = uptimeSec % 60UL;

    char buffer[16];
    if (hours > 0)
    {
        snprintf(buffer, sizeof(buffer), "%luh %02lum", static_cast<unsigned long>(hours), static_cast<unsigned long>(minutes));
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu", static_cast<unsigned long>(minutes), static_cast<unsigned long>(seconds));
    }
    return String(buffer);
}

String feedbackText(const CommandFeedbackState &feedback)
{
    if (!feedback.hasValue)
    {
        return "Waiting for input";
    }

    return String(feedback.success ? "OK " : "FAIL ") + clipText(feedback.label, 18);
}

void drawPanel(int x, int y, int w, int h, uint16_t border = PANEL_BORDER, uint16_t fill = PANEL_BG)
{
    M5Cardputer.Display.fillRoundRect(x, y, w, h, 5, fill);
    M5Cardputer.Display.drawRoundRect(x, y, w, h, 5, border);
}

void drawKeyValuePanel(int x, int y, int w, int h, const String &label, const String &value, uint16_t valueColor = WHITE)
{
    drawPanel(x, y, w, h);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(LIGHTGREY, PANEL_BG);
    M5Cardputer.Display.setCursor(x + 6, y + 4);
    M5Cardputer.Display.println(label);
    M5Cardputer.Display.setTextColor(valueColor, PANEL_BG);
    M5Cardputer.Display.setCursor(x + 6, y + 14);
    M5Cardputer.Display.println(clipText(value, 21));
}

void drawHeader(const String &title)
{
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.fillRoundRect(PAD, HEADER_Y, SCREEN_W - (PAD * 2), HEADER_H, 6, PANEL_ALT_BG);
    M5Cardputer.Display.drawRoundRect(PAD, HEADER_Y, SCREEN_W - (PAD * 2), HEADER_H, 6, ACCENT);
    M5Cardputer.Display.setTextColor(WHITE, PANEL_ALT_BG);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(14, 10);
    M5Cardputer.Display.println(title);
    M5Cardputer.Display.setTextSize(1);
}

void drawFooter(const String &text)
{
    M5Cardputer.Display.setTextColor(LIGHTGREY, BLACK);
    M5Cardputer.Display.setCursor(PAD, FOOTER_Y);
    M5Cardputer.Display.println(clipText(text, 37));
}

void drawMenu(const AppState &state)
{
    static const char *items[MENU_COUNT] = {"Controls", "Diagnostics", "Calibration"};
    static const char *detail[MENU_COUNT] = {"Drive, jump, emergency", "Battery, link, event log", "Servo select and jog"};

    drawHeader("Stack Kame");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(LIGHTGREY, BLACK);
    M5Cardputer.Display.setCursor(PAD, 32);
    M5Cardputer.Display.println("W/S move   Enter open   X stop");

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        const int y = 46 + (i * 22);
        const bool selected = (state.menu.selected == i);
        const uint16_t bg = selected ? ACCENT : PANEL_BG;
        const uint16_t fg = selected ? BLACK : WHITE;

        M5Cardputer.Display.fillRoundRect(PAD, y, SCREEN_W - (PAD * 2), 18, 5, bg);
        M5Cardputer.Display.drawRoundRect(PAD, y, SCREEN_W - (PAD * 2), 18, 5, selected ? ACCENT : PANEL_BORDER);
        M5Cardputer.Display.setTextColor(fg, bg);
        M5Cardputer.Display.setCursor(16, y + 5);
        M5Cardputer.Display.println(items[i]);
    }

    drawKeyValuePanel(PAD, 114 - 16, SCREEN_W - (PAD * 2), 20, "Selected", detail[state.menu.selected]);
    drawFooter("Del back on sub-screens");
}

void drawControls(const AppState &state)
{
    drawHeader("Controls");
    drawKeyValuePanel(PAD, 32, 68, 28, "Steps", String(state.controls.speed));
    drawKeyValuePanel(84, 32, SCREEN_W - 92, 28, "Last TX", feedbackText(state.commandFeedback), state.commandFeedback.success ? ACCENT_OK : ACCENT_FAIL);

    drawPanel(PAD, 68, SCREEN_W - (PAD * 2), 42);
    M5Cardputer.Display.setTextColor(WHITE, PANEL_BG);
    M5Cardputer.Display.setCursor(16, 76);
    M5Cardputer.Display.println("W forward      S backward");
    M5Cardputer.Display.setCursor(16, 88);
    M5Cardputer.Display.println("A turn left    D turn right");
    M5Cardputer.Display.setCursor(16, 100);
    M5Cardputer.Display.println("Enter jump     1-3 steps");

    drawFooter("Del menu   X emergency stop");
}

void drawDiagnostics(const AppState &state)
{
    drawHeader("Diagnostics");
    drawKeyValuePanel(PAD, 32, 70, 24, "Battery", String(state.diagnostics.batteryPercent) + "%");
    drawKeyValuePanel(85, 32, 70, 24, "RSSI", String(state.diagnostics.rssiDbm) + " dBm");
    drawKeyValuePanel(162, 32, 70, 24, "Uptime", formatUptime(state.diagnostics.uptimeSec));

    drawPanel(PAD, 62, SCREEN_W - (PAD * 2), 50);
    M5Cardputer.Display.setTextColor(LIGHTGREY, PANEL_BG);
    M5Cardputer.Display.setCursor(16, 70);
    M5Cardputer.Display.println("Recent events");

    int y = 82;
    for (int i = static_cast<int>(state.diagnostics.logCount) - 1; i >= 0 && y <= 106; --i)
    {
        M5Cardputer.Display.setTextColor(WHITE, PANEL_BG);
        M5Cardputer.Display.setCursor(16, y);
        M5Cardputer.Display.println(clipText(state.diagnostics.logs[i], 31));
        y += 12;
    }

    drawFooter("Del menu");
}

const char *servoName(uint8_t id)
{
    static const char *names[8] = {"FL Hip", "FR Hip", "FL Foot", "FR Foot", "BL Hip", "BR Hip", "BL Foot", "BR Foot"};
    return (id < 8) ? names[id] : "?";
}

String formatTrim(int trim)
{
    if (trim == 0) return "0";
    return String(trim > 0 ? "+" : "") + String(trim);
}

void drawCalibration(const AppState &state)
{
    const int trim = state.calibration.selectedAngle - 90;
    const int savedTrim = state.calibration.trims[state.calibration.selectedServo];

    drawHeader("Calibration");

    // Row 1: servo name | current jog angle | trim that would be saved
    drawKeyValuePanel(PAD, 32, 68, 24, "Servo", String(state.calibration.selectedServo) + " " + servoName(state.calibration.selectedServo));
    drawKeyValuePanel(82, 32, 72, 24, "Angle", String(state.calibration.selectedAngle) + " deg");
    drawKeyValuePanel(160, 32, 72, 24, "Trim", formatTrim(trim) + " (sv:" + formatTrim(savedTrim) + ")");

    // Row 2: instructions + last TX
    drawPanel(PAD, 62, SCREEN_W - (PAD * 2), 38);
    M5Cardputer.Display.setTextColor(WHITE, PANEL_BG);
    M5Cardputer.Display.setCursor(16, 70);
    M5Cardputer.Display.println("0-7 select   Arrows jog +/-5");
    M5Cardputer.Display.setCursor(16, 82);
    M5Cardputer.Display.println("Enter saves trim for servo");

    drawKeyValuePanel(PAD, 106, SCREEN_W - (PAD * 2), 20, "Last TX", feedbackText(state.commandFeedback), state.commandFeedback.success ? ACCENT_OK : ACCENT_FAIL);

    drawFooter("Del menu   Enter saves trim");
}

void changeScreen(AppState &state, ScreenId next)
{
    if (state.activeScreen != next)
    {
        state.activeScreen = next;
        state.needsRedraw = true;
    }
}

void recordCommandFeedback(AppState &state, const String &label, bool success)
{
    state.commandFeedback.label = label;
    state.commandFeedback.success = success;
    state.commandFeedback.hasValue = true;
    state.commandFeedback.updatedMs = millis();
    state.diagnostics.addLog(String(success ? "TX ok " : "TX fail ") + clipText(label, 18));
    state.needsRedraw = true;
}

bool sendCommand(AppState &state, CommandSender &sender, uint8_t cmd, uint8_t value, const String &label)
{
    const bool success = sender.send(cmd, value);
    recordCommandFeedback(state, label, success);
    return success;
}

void dispatchControlCommand(Action action, AppState &state, CommandSender &sender)
{
    uint8_t cmd = robotcmd::STOP;
    bool shouldSend = true;
    String label;

    switch (action)
    {
    case Action::Up:
        cmd = robotcmd::WALK_FORWARD;
        label = "Walk fwd x" + String(state.controls.speed);
        break;
    case Action::Down:
        cmd = robotcmd::WALK_BACKWARD;
        label = "Walk back x" + String(state.controls.speed);
        break;
    case Action::Left:
        cmd = robotcmd::TURN_LEFT;
        label = "Turn left x" + String(state.controls.speed);
        break;
    case Action::Right:
        cmd = robotcmd::TURN_RIGHT;
        label = "Turn right x" + String(state.controls.speed);
        break;
    case Action::Select:
        cmd = robotcmd::JUMP;
        label = "Jump";
        break;
    case Action::Emergency:
        cmd = robotcmd::EMERGENCY_STOP;
        label = "Emergency stop";
        break;
    case Action::Home:
        cmd = robotcmd::HOME;
        label = "Go to home position";
        break;
    case Action::Zero:
        cmd = robotcmd::ZERO;
        label = "Go to zero position";
        break;
    default:
        shouldSend = false;
        break;
    }

    if (!shouldSend)
    {
        return;
    }

    const uint8_t value = (cmd == robotcmd::JUMP || cmd == robotcmd::EMERGENCY_STOP) ? 0 : state.controls.speed;
    sendCommand(state, sender, cmd, value, label);
}

void handleMenuInput(const KeyInput &input, AppState &state)
{
    if (input.action == Action::Up)
    {
        state.menu.selected = (state.menu.selected - 1 + MENU_COUNT) % MENU_COUNT;
        state.needsRedraw = true;
    }
    else if (input.action == Action::Down)
    {
        state.menu.selected = (state.menu.selected + 1) % MENU_COUNT;
        state.needsRedraw = true;
    }
    else if (input.action == Action::Select)
    {
        if (state.menu.selected == 0)
        {
            changeScreen(state, ScreenId::Controls);
            state.diagnostics.addLog("Open controls");
        }
        else if (state.menu.selected == 1)
        {
            changeScreen(state, ScreenId::Diagnostics);
            state.diagnostics.addLog("Open diagnostics");
        }
        else if (state.menu.selected == 2)
        {
            changeScreen(state, ScreenId::Calibration);
            state.diagnostics.addLog("Open calibration");
        }
    }
}

void handleControlInput(const KeyInput &input, AppState &state, CommandSender &sender)
{
    if (input.action == Action::Back)
    {
        changeScreen(state, ScreenId::Menu);
        state.diagnostics.addLog("Back to menu");
        return;
    }

    if (input.number >= 1 && input.number <= 3)
    {
        state.controls.speed = static_cast<uint8_t>(input.number);
        state.diagnostics.addLog("Speed " + String(state.controls.speed));
        state.needsRedraw = true;
    }

    dispatchControlCommand(input.action, state, sender);
}

void handleCalibrationInput(const KeyInput &input, AppState &state, CommandSender &sender)
{
    if (input.action == Action::Back)
    {
        changeScreen(state, ScreenId::Menu);
        state.diagnostics.addLog("Back to menu");
        return;
    }

    if (input.action == Action::Select)
    {
        const int trim = state.calibration.selectedAngle - 90;
        const String label = "Save S" + String(state.calibration.selectedServo) + " trim " + (trim >= 0 ? "+" : "") + String(trim);
        if (sendCommand(state, sender, robotcmd::CAL_SAVE, state.calibration.selectedServo, label))
        {
            state.calibration.trims[state.calibration.selectedServo] = trim;
            state.diagnostics.addLog(label);
        }
        return;
    }

    if (input.number >= 0 && input.number <= 7)
    {
        const uint8_t nextServo = static_cast<uint8_t>(input.number);
        if (sendCommand(state, sender, robotcmd::CAL_SELECT_SERVO, nextServo, "Select servo " + String(nextServo)))
        {
            state.calibration.selectedServo = nextServo;
            state.calibration.selectedAngle = 90;
        }
        return;
    }

    int delta = 0;
    if (input.action == Action::Left || input.action == Action::Down)
    {
        delta = -5;
    }
    else if (input.action == Action::Right || input.action == Action::Up)
    {
        delta = 5;
    }

    if (delta != 0)
    {
        const int nextAngle = constrain(state.calibration.selectedAngle + delta, 0, 180);
        if (nextAngle == state.calibration.selectedAngle)
        {
            recordCommandFeedback(state, "Angle limit", true);
            return;
        }

        if (sendCommand(state, sender, robotcmd::CAL_JOG_DELTA, static_cast<uint8_t>(static_cast<int8_t>(delta)), "Jog " + String(delta) + " S" + String(state.calibration.selectedServo)))
        {
            state.calibration.selectedAngle = nextAngle;
        }
    }
}

void handleDiagnosticsInput(const KeyInput &input, AppState &state)
{
    if (input.action == Action::Back)
    {
        changeScreen(state, ScreenId::Menu);
        state.diagnostics.addLog("Back to menu");
    }
}

} // namespace

void handleInput(const KeyInput &input, AppState &state, CommandSender &sender)
{
    if (!input.pressed)
    {
        return;
    }

    if (input.action == Action::Emergency)
    {
        sendCommand(state, sender, robotcmd::EMERGENCY_STOP, 0, "Emergency stop");
        return;
    }

    switch (state.activeScreen)
    {
    case ScreenId::Menu:
        handleMenuInput(input, state);
        break;
    case ScreenId::Controls:
        handleControlInput(input, state, sender);
        break;
    case ScreenId::Diagnostics:
        handleDiagnosticsInput(input, state);
        break;
    case ScreenId::Calibration:
        handleCalibrationInput(input, state, sender);
        break;
    }
}

void refreshDiagnostics(AppState &state)
{
    const unsigned long now = millis();
    if (now - state.lastDiagnosticsRefreshMs < 500)
    {
        return;
    }

    state.lastDiagnosticsRefreshMs = now;

    const int newBattery = M5.Power.getBatteryLevel();
    const int newRssi = WiFi.RSSI();
    const uint32_t newUptime = static_cast<uint32_t>(now / 1000UL);

    if (newBattery != state.diagnostics.batteryPercent ||
        newRssi != state.diagnostics.rssiDbm ||
        newUptime != state.diagnostics.uptimeSec)
    {
        state.diagnostics.batteryPercent = newBattery;
        state.diagnostics.rssiDbm = newRssi;
        state.diagnostics.uptimeSec = newUptime;
        if (state.activeScreen == ScreenId::Diagnostics)
        {
            state.needsRedraw = true;
        }
    }
}

void drawActiveScreen(const AppState &state)
{
    switch (state.activeScreen)
    {
    case ScreenId::Menu:
        drawMenu(state);
        break;
    case ScreenId::Controls:
        drawControls(state);
        break;
    case ScreenId::Diagnostics:
        drawDiagnostics(state);
        break;
    case ScreenId::Calibration:
        drawCalibration(state);
        break;
    }
}
