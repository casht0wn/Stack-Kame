#include <M5Cardputer.h>
#include <M5Unified.h>
#include <M5GFX.h>

class Screen
{
public:
    virtual void draw() = 0;
    virtual void update() = 0;
};

class Widget
{
public:
    int x, y, w, h;

    virtual void draw() = 0;
    virtual void update() {}
};

class LogsWidget : public Widget
{
public:
    std::vector<String> logs;
    int logIndex = 0;

    void draw() override
    {
        M5.Display.fillScreen(BLACK);
        int y = 0;
        for (const auto &log : logs)
        {
            M5.Display.setCursor(0, y);
            M5.Display.println(log);
            y += 20;
        }
    }

    void update() override
    {
        // Add new logs as needed
    }

    void log(String msg)
    {
        logs[logIndex] = msg;
        logIndex = (logIndex + 1) % logs.size();
    }
};


class BatteryWidget : public Widget
{
public:
    int percent = 0;

    void draw() override
    {
        M5.Display.drawRect(x, y, w, h, WHITE);

        int fill = (w - 4) * percent / 100;

        M5.Display.fillRect(x + 2, y + 2, fill, h - 4, GREEN);

        M5.Display.setCursor(x + w + 5, y);
        M5.Display.printf("%d%%", percent);
    }

    void update() override
    {
        percent = M5.Power.getBatteryLevel();
    }
};

class NetworkWidget : public Widget
{
public:
    int signalStrength = 0;

    void draw() override
    {
        M5.Display.drawRect(x, y, w, h, WHITE);

        int fill = (w - 4) * signalStrength / 100;

        M5.Display.fillRect(x + 2, y + 2, fill, h - 4, BLUE);

        M5.Display.setCursor(x + w + 5, y);
        M5.Display.printf("%d%%", signalStrength);
    }

    void update() override
    {
        signalStrength = WiFi.RSSI();
    }
};


class Button : public Widget
{
public:
    String label;
    bool selected = false;

    void draw() override
    {
        uint16_t bg = selected ? GREEN : DARKGREY;

        M5.Display.fillRect(x, y, w, h, bg);
        M5.Display.drawRect(x, y, w, h, WHITE);

        M5.Display.drawCentreString(label, x + w / 2, y + 10, 2);
    }

    void update() override
    {
        // Handle button state changes if needed
    }

    void onPress()
    {
        // Handle button press action
    }
};

class MenuScreen : public Screen
{
public:
    std::vector<Button> buttons;
    int selectedIndex = 0;

    void draw() override
    {
        M5.Display.fillScreen(BLACK);

        for (size_t i = 0; i < buttons.size(); i++)
        {
            buttons[i].selected = (i == selectedIndex);
            buttons[i].draw();
        }
    }

    void update() override
    {
        // Handle input to change selectedIndex
        // use arrow key buttons to navigate, enter to select, and backspace to go back

    }

    void addButton(String label)
    {
        Button btn;
        btn.label = label;
        btn.x = 10;
        btn.y = 10 + buttons.size() * 40;
        btn.w = 200;
        btn.h = 30;
        buttons.push_back(btn);
    }

    void selectNext()
    {
        selectedIndex = (selectedIndex + 1) % buttons.size();
    }

    void selectPrevious()
    {
        selectedIndex = (selectedIndex - 1 + buttons.size()) % buttons.size();
    }
};

class CalibrationScreen : public Screen
{
public:
    void draw() override
    {
        M5.Display.fillScreen(BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.println("Calibration Screen");
    }

    void update() override
    {
        // Handle calibration logic and update state
    }
};

class DiagnosticsScreen : public Screen
{
public:
    LogsWidget logsWidget;

    void draw() override
    {
        logsWidget.draw();
    }

    void update() override
    {
        logsWidget.update();
    }
};

class MainControlScreen  : public Screen
{
public:
    void draw() override
    {
        M5.Display.fillScreen(BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.println("Controller Screen");
    }
    
    void update() override
    {
        // Handle controller input and update state
        
    }
};
