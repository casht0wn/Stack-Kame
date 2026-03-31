#pragma once

#include <Arduino.h>

enum class ScreenId
{
    Menu,
    Controls,
    Diagnostics,
    Calibration,
};

enum class Action
{
    None,
    Up,
    Down,
    Left,
    Right,
    Select,
    Back,
    Emergency,
    Home,
    Zero,
};

struct KeyInput
{
    Action action = Action::None;
    int number = -1;
    bool pressed = false;
};

struct MenuState
{
    int selected = 0;
};

struct ControlsState
{
    uint8_t speed = 1;
};

struct CommandFeedbackState
{
    String label;
    bool success = true;
    bool hasValue = false;
    unsigned long updatedMs = 0;
};

struct DiagnosticsState
{
    static constexpr size_t LOG_CAPACITY = 8;

    int batteryPercent = 0;
    int rssiDbm = 0;
    uint32_t uptimeSec = 0;

    String logs[LOG_CAPACITY];
    size_t logCount = 0;

    void addLog(const String &line)
    {
        if (logCount < LOG_CAPACITY)
        {
            logs[logCount++] = line;
            return;
        }

        for (size_t i = 1; i < LOG_CAPACITY; ++i)
        {
            logs[i - 1] = logs[i];
        }
        logs[LOG_CAPACITY - 1] = line;
    }
};

struct CalibrationState
{
    uint8_t selectedServo = 0;
    int selectedAngle = 90;
    int trims[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // saved trim per servo (degrees from center)
};

struct AppState
{
    ScreenId activeScreen = ScreenId::Menu;
    MenuState menu;
    ControlsState controls;
    CommandFeedbackState commandFeedback;
    DiagnosticsState diagnostics;
    CalibrationState calibration;
    unsigned long lastInputMs = 0;
    unsigned long lastDiagnosticsRefreshMs = 0;
    unsigned long lastRepeatMs = 0;
    Action heldAction = Action::None;
    int heldNumber = -1;
    bool repeatActive = false;
    bool needsRedraw = true;
};
