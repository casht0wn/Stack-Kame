#include "Oscillator.h"
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

Oscillator::Oscillator() {
    _amplitude = 0;
    _offset = 0;
    _phase = 0;
    _phaseOffset = 0;
    _period = 2000; // Default 2 second period
    _position = 0;
    _running = false;
    _startTime = 0;
}

void Oscillator::setAmplitude(double amplitude) {
    _amplitude = amplitude;
}

void Oscillator::setOffset(double offset) {
    _offset = offset;
}

void Oscillator::setPhase(double phase) {
    _phase = phase;
}

void Oscillator::setPeriod(unsigned long period) {
    if (period > 0) {
        _period = period;
    }
}

void Oscillator::setPhaseOffset(double phaseOffset) {
    _phaseOffset = phaseOffset;
}

void Oscillator::start() {
    if (!_running) {
        _running = true;
        _startTime = millis();
    }
}

void Oscillator::stop() {
    _running = false;
}

void Oscillator::reset() {
    _phase = 0;
    _startTime = millis();
}

double Oscillator::refresh() {
    if (!_running) {
        return _position;
    }
    
    // Calculate time since start in milliseconds
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - _startTime;
    
    // Calculate phase based on elapsed time and period
    // phase = 2*PI * (t / T) + phaseOffset
    _phase = (2.0 * PI * (double)elapsedTime / (double)_period) + _phaseOffset;
    
    // Calculate position using sine wave
    // position = A * sin(phase) + offset
    _position = _amplitude * sin(_phase) + _offset;
    
    return _position;
}

void Oscillator::setPosition(double position) {
    _position = position;
    _running = false; // Manual position sets stop oscillation
}
