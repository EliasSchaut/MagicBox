#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <Keypad.h>
#include <LedControl.h>
#include "types.h"

// Keypad
extern Keypad customKeypad;

// LED Matrix
extern LedControl lc;

// Joystick
struct JoystickData {
    int x;
    int y;
    bool pressed;
};

JoystickData readJoystick();
Direction readJoystrickDirection();
void printDirection(Direction dir);
void setupHardware();

#endif
