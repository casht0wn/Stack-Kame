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

class StackKame
{
public:
    void init();
    void zero();
    void home();

    // Manual servo control
    void setServo(int id, double target);
    void reverseServo(int id);
    double getServo(int id);
    void setTrims(int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7);

    // Oscillator-based movements
    void walk(int steps = 1, int period = 900, bool reverse = false);
    void turn(int steps = 1, int period = 900, bool leftTurn = true);
    void moveServos(int time, double target[8]);
    void execute(int steps, int period[8], int amplitude[8], int offset[8], int phase[8]);

    // Movement patterns
    void jump();
    void home_position();
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
    void _final_position(int time, double target[8]);
};

#endif