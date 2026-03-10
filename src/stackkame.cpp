#include <stackkame.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // 0x40

// Depending on your servo make, the pulse width min and max may vary, you want to tweak these as necessary to prevent damage to your servos. You will likely need to
// adjust these as per your servo's specifications, which commonly are around 500-2400 microseconds. The 'map' function is used to convert from degrees to pulse length.
#define SERVOMIN 150  // This is the 'minimum' pulse length count (out of 4096) ~1000µs
#define SERVOMAX 600  // This is the 'maximum' pulse length count (out of 4096) ~2000µs
#define SERVO_FREQ 50 // Standard servo frequency for MG90S digital servos

void StackKame::init()
{
    // Initialize PWM driver
    Serial.println("  PWM: Starting initialization...");
    delay(100);  // Let I2C bus settle
    
    // Simple single check for PWM wing at 0x40
    Serial.println("  PWM: Checking for PWM wing at 0x40...");
    Wire.beginTransmission(0x40);
    int error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("  PWM: Device ACK received at 0x40");
    } else {
        Serial.print("  PWM: WARNING - Error code ");
        Serial.print(error);
        Serial.println(" when checking 0x40");
        Serial.println("  PWM: Attempting initialization anyway...");
    }
    delay(100);
    
    Serial.println("  PWM: Calling pwm.begin()...");
    if (!pwm.begin()) {
        Serial.println("  PWM: ERROR - pwm.begin() returned false");
    } else {
        Serial.println("  PWM: pwm.begin() succeeded");
    }
    delay(100);
    
    Serial.println("  PWM: Setting oscillator frequency to 27MHz...");
    pwm.setOscillatorFrequency(27000000);
    delay(100);
    
    Serial.println("  PWM: Setting PWM frequency to 50 Hz...");
    pwm.setPWMFreq(SERVO_FREQ);
    delay(100);

    // Initialize trim values to zero
    Serial.println("  Initializing trim values...");
    for (int i = 0; i < 8; i++)
    {
        trim[i] = 0;
        reverse[i] = false;
        _servo_position[i] = 90.0;
    }

    _isMoving = false;

    Serial.println("StackKame servo controller ready");
}

void StackKame::zero()
{
    Serial.println("    zero(): Setting all servos to 90 degrees...");
    for (int i = 0; i < 8; i++) {
        Serial.print("      Servo ");
        Serial.print(i);
        Serial.println(": 90");
        setServo(i, 90.0);
        delay(10);
    }
    Serial.println("    zero(): Complete");
}

void StackKame::home()
{
    Serial.println("    home(): Setting home position...");
    int ap = 20;
    int hi = 35;
    int home_position[8] = {90 - ap, 90 - hi, 90 + ap, 90 + hi, 90 - ap, 90 - hi, 90 + ap, 90 + hi};
    for (int i = 0; i < 8; i++) {
        Serial.print("      Servo ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(home_position[i]);
        setServo(i, home_position[i]);
        delay(10);
    }
    Serial.println("    home(): Complete");
}

void StackKame::setServo(int id, double target)
{
    // Bounds check: constrain target to safe servo range
    if (id < 0 || id > 7) {
        Serial.print("WARNING: Servo ID out of range: ");
        Serial.println(id);
        return;
    }
    
    double constrainedTarget = constrain(target, 0.0, 180.0);
    if (constrainedTarget != target) {
        Serial.print("WARNING: Servo ");
        Serial.print(id);
        Serial.print(" target clamped from ");
        Serial.print(target);
        Serial.print(" to ");
        Serial.println(constrainedTarget);
    }
    
    int pulseValue = degToPulse(constrainedTarget + trim[id]);
    // Clamp pulse value to safe PWM output range
    pulseValue = constrain(pulseValue, SERVOMIN, SERVOMAX);
    
    Serial.print("        setServo(");
    Serial.print(id);
    Serial.print(", ");
    Serial.print(target);
    Serial.print(") -> pulse=");
    Serial.println(pulseValue);
    
    // Write to PWM driver and check return code
    uint8_t result = 0;
    if (!reverse[id])
        result = pwm.setPWM(id, 0, pulseValue);
    else
        result = pwm.setPWM(id, 0, degToPulse(180 - constrainedTarget + trim[id]));
    
    if (result != 0) {
        Serial.print("          ERROR: PWM write failed with code ");
        Serial.println(result);
    }
    
    _servo_position[id] = constrainedTarget;
}

double StackKame::degToPulse(int degrees)
{
    int pulse = map(degrees, 0, 180, SERVOMIN, SERVOMAX);
    Serial.print("          degToPulse(");
    Serial.print(degrees);
    Serial.print("°) = ");
    Serial.print(pulse);
    Serial.print(" (min=");
    Serial.print(SERVOMIN);
    Serial.print(", max=");
    Serial.print(SERVOMAX);
    Serial.println(")");
    return pulse;
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
    // Quick up-down motion using direct servo calls
    for (int i = 0; i < 8; i++) {
        setServo(i, (i % 2 == 0) ? 90 : 120);  // Hips stay, feet up
    }
    delay(200);
    
    for (int i = 0; i < 8; i++) {
        setServo(i, (i % 2 == 0) ? 90 : 60);   // Feet down
    }
    delay(100);
    
    home();
}