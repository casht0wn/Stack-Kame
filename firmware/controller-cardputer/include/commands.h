#pragma once

#include <Arduino.h>

namespace robotcmd {
static constexpr uint8_t STOP = 0x01;
static constexpr uint8_t WALK_FORWARD = 0x02;
static constexpr uint8_t WALK_BACKWARD = 0x03;
static constexpr uint8_t TURN_LEFT = 0x04;
static constexpr uint8_t TURN_RIGHT = 0x05;
static constexpr uint8_t MOONWALK_FORWARD = 0x06;
static constexpr uint8_t MOONWALK_BACKWARD = 0x07;
static constexpr uint8_t JUMP = 0x08;
static constexpr uint8_t SHUFFLE_LEFT = 0x09;
static constexpr uint8_t SHUFFLE_RIGHT = 0x0A;
static constexpr uint8_t HOME = 0x0B;
static constexpr uint8_t ZERO = 0x0C;
static constexpr uint8_t CAL_SELECT_SERVO = 0x0D;
static constexpr uint8_t CAL_JOG_DELTA = 0x0E;
static constexpr uint8_t CAL_SAVE = 0x0F;
static constexpr uint8_t EMERGENCY_STOP = 0xFF;
} // namespace robotcmd
