#include <stackkame.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // 0x40

// Depending on your servo make, the pulse width min and max may vary, you want to tweak these as necessary to prevent damage to your servos. You will likely need to
// adjust these as per your servo's specifications, which commonly are around 500-2400 microseconds. The 'map' function is used to convert from degrees to pulse length.
#define SERVOMIN 150  // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600  // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates, digital at ~333 Hz

void StackKame::init()
{
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(SERVO_FREQ);

    // Initialize trim values to zero
    for (int i = 0; i < 8; i++)
    {
        trim[i] = 0;
        reverse[i] = false;
        _servo_position[i] = 90.0;
    }

    _isMoving = false;

    // Set servos to home position
    home();
}

void StackKame::zero()
{
    for (int i = 0; i < 8; i++)
        setServo(i, 90.0);
}

void StackKame::home()
{
    int ap = 20;
    int hi = 35;
    int home_position[8] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
    for (int i = 0; i < 8; i++)
        setServo(i, home_position[i]);
}

void StackKame::reverseServo(int id)
{
    if (reverse[id])
        reverse[id] = 0;
    else
        reverse[id] = 1;
}

void StackKame::setServo(int id, double target)
{
    if (!reverse[id])
        pwm.setPWM(id, 0, degToPulse(target + trim[id]));
    else
        pwm.setPWM(id, 0, degToPulse(180 - target + trim[id]));
    _servo_position[id] = target;
}

double StackKame::getServo(int id)
{
    return _servo_position[id];
}

double StackKame::degToPulse(int degrees)
{
    return map(degrees, 0, 180, SERVOMIN, SERVOMAX);
}

void StackKame::setTrims(int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7)
{
    trim[0] = t0;
    trim[1] = t1;
    trim[2] = t2;
    trim[3] = t3;
    trim[4] = t4;
    trim[5] = t5;
    trim[6] = t6;
    trim[7] = t7;
}

void StackKame::update()
{
    if (_isMoving)
    {
        for (int i = 0; i < 8; i++)
        {
            if (oscillator[i].isRunning())
            {
                double pos = oscillator[i].refresh();
                setServo(i, pos);
            }
        }
    }
}

void StackKame::stopMovement()
{
    _isMoving = false;
    for (int i = 0; i < 8; i++)
    {
        oscillator[i].stop();
    }
}

void StackKame::moveServos(int time, double target[8])
{
    _final_position(time, target);
}

void StackKame::_final_position(int time, double target[8])
{
    if (time > 10)
    {
        for (int i = 0; i < 8; i++)
        {
            double startPos = _servo_position[i];
            double delta = target[i] - startPos;
            int steps = time / 10;

            for (int step = 0; step <= steps; step++)
            {
                double pos = startPos + (delta * step / steps);
                setServo(i, pos);
                delay(10);
            }
        }
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            setServo(i, target[i]);
        }
    }
}

void StackKame::execute(int steps, int period[8], int amplitude[8], int offset[8], int phase[8])
{
    // Setup oscillators with provided parameters
    for (int i = 0; i < 8; i++)
    {
        oscillator[i].setPeriod(period[i]);
        oscillator[i].setAmplitude(amplitude[i]);
        oscillator[i].setOffset(offset[i]);
        oscillator[i].setPhaseOffset(phase[i] * 3.14159 / 180.0); // Convert degrees to radians
        oscillator[i].reset();
        oscillator[i].start();
    }

    _isMoving = true;

    // Run for specified number of steps
    unsigned long maxPeriod = period[0];
    for (int i = 1; i < 8; i++)
    {
        if (period[i] > maxPeriod)
            maxPeriod = period[i];
    }

    unsigned long endTime = millis() + (steps * maxPeriod);

    while (millis() < endTime)
    {
        update();
        delay(20); // Update at ~50Hz
    }

    stopMovement();
}

void StackKame::walk(int steps, int period, bool reverse)
{
    int amplitude[8] = {30, 20, 30, 20, 30, 20, 30, 20};
    int offset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int phase[8];

    if (!reverse)
    {
        // Forward walking gait
        int tempPhase[8] = {0, 90, 180, 90, 180, 90, 0, 90};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }
    else
    {
        // Reverse walking gait
        int tempPhase[8] = {180, 90, 0, 90, 0, 90, 180, 90};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }

    int periods[8];
    for (int i = 0; i < 8; i++)
        periods[i] = period;

    execute(steps, periods, amplitude, offset, phase);
}

void StackKame::turn(int steps, int period, bool leftTurn)
{
    int amplitude[8] = {30, 20, 30, 20, 30, 20, 30, 20};
    int offset[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int phase[8];

    if (leftTurn)
    {
        // Turn left
        int tempPhase[8] = {0, 90, 0, 90, 0, 90, 0, 90};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }
    else
    {
        // Turn right
        int tempPhase[8] = {180, 90, 180, 90, 180, 90, 180, 90};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }

    int periods[8];
    for (int i = 0; i < 8; i++)
        periods[i] = period;

    execute(steps, periods, amplitude, offset, phase);
}

void StackKame::moonwalk(int steps, int period, bool reverse)
{
    int amplitude[8] = {25, 25, 25, 25, 25, 25, 25, 25};
    int offset[8] = {0, -10, 0, -10, 0, -10, 0, -10};
    int phase[8];

    if (!reverse)
    {
        int tempPhase[8] = {0, 120, 0, 120, 180, 240, 180, 240};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }
    else
    {
        int tempPhase[8] = {180, 240, 180, 240, 0, 120, 0, 120};
        for (int i = 0; i < 8; i++)
            phase[i] = tempPhase[i];
    }

    int periods[8];
    for (int i = 0; i < 8; i++)
        periods[i] = period;

    execute(steps, periods, amplitude, offset, phase);
}

void StackKame::lateral_fuerte(bool left, int steps, int period)
{
    int amplitude[8] = {25, 25, 0, 25, 25, 25, 0, 25};
    int offset[8];
    int phase[8];

    if (left)
    {
        int tempOffset[8] = {15, -10, 0, -10, -15, -10, 0, -10};
        int tempPhase[8] = {0, 0, 90, 0, 0, 0, 90, 0};
        for (int i = 0; i < 8; i++)
        {
            offset[i] = tempOffset[i];
            phase[i] = tempPhase[i];
        }
    }
    else
    {
        int tempOffset[8] = {-15, -10, 0, -10, 15, -10, 0, -10};
        int tempPhase[8] = {0, 0, 90, 0, 0, 0, 90, 0};
        for (int i = 0; i < 8; i++)
        {
            offset[i] = tempOffset[i];
            phase[i] = tempPhase[i];
        }
    }

    int periods[8];
    for (int i = 0; i < 8; i++)
        periods[i] = period;

    execute(steps, periods, amplitude, offset, phase);
}

void StackKame::jump()
{
    double up[8] = {90, 120, 90, 120, 90, 120, 90, 120};
    double down[8] = {90, 60, 90, 60, 90, 60, 90, 60};

    moveServos(200, up);
    delay(200);
    moveServos(100, down);
    delay(100);
    home();
}

void StackKame::home_position()
{
    home();
}