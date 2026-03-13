#ifndef stackkame_h
#define stackkame_h

#include "../lib/Octosnake/Oscillator.h"

// Servo pin definitions for 8-channel PWM board
#define FRONT_LEFT_HIP 0
#define FRONT_LEFT_FOOT 1
#define BACK_LEFT_HIP 2
#define BACK_LEFT_FOOT 3
#define BACK_RIGHT_HIP 4
#define BACK_RIGHT_FOOT 5
#define FRONT_RIGHT_HIP 6
#define FRONT_RIGHT_FOOT 7

/*
    StackKame - 8-DOF quadruped robot controller using PCA9685 PWM driver and oscillator-based gait generation.

    This class provides high-level movement functions (walk, turn, jump, shuffle) that internally use oscillators to create smooth, natural motions. It also includes direct servo control for manual adjustments and calibration.

    The main loop should call the update() method regularly to ensure the oscillators are refreshed and the servos are updated accordingly.
*/
class StackKame
{
public:
    void init();

    // Base positioning functions
    void zero();
    void home();

    // Manual servo control
    void setServo(int id, double target);
    void setTrims(int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7);

    // Oscillator-based movements
    void walk(int steps = 1, int period = 900, bool reverse = false);
    void turn(int steps = 1, int period = 900, bool leftTurn = true);
    void execute(int steps, int period[8], int amplitude[8], int offset[8], int phase[8]);

    // Movement patterns
    void jump();
    void lateral_fuerte(bool left, int steps, int period);
    void moonwalk(int steps, int period, bool reverse);

    // Update function - must be called regularly in loop()
    void update();

    // Stop all movements
    void stopMovement();

private:
    int servo[8];
    int trim[8];
    bool reverse[8];
    double _servo_position[8];
    Oscillator oscillator[8];
    bool _isMoving;

    double degToPulse(int degrees);
};

#endif