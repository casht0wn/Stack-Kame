#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <Arduino.h>

class Oscillator
{
public:
    Oscillator();

    // Set oscillator parameters
    void setAmplitude(double amplitude);
    void setOffset(double offset);
    void setPhase(double phase);
    void setPeriod(unsigned long period);
    void setPhaseOffset(double phaseOffset);

    // Get current parameters
    double getAmplitude() const { return _amplitude; }
    double getOffset() const { return _offset; }
    double getPhase() const { return _phase; }
    unsigned long getPeriod() const { return _period; }

    // Start/stop oscillation
    void start();
    void stop();
    void reset();

    // Calculate current position based on time
    double refresh();

    // Get current position without recalculating
    double getPosition() const { return _position; }

    // Check if oscillating
    bool isRunning() const { return _running; }

    // Set position directly (manual mode)
    void setPosition(double position);

private:
    double _amplitude;        // Oscillation amplitude in degrees
    double _offset;           // Center offset in degrees
    double _phase;            // Current phase
    double _phaseOffset;      // Initial phase offset in radians
    unsigned long _period;    // Period in milliseconds
    unsigned long _startTime; // Time when oscillation started
    double _position;         // Current calculated position
    bool _running;            // Is oscillator running
};

#endif // OSCILLATOR_H
