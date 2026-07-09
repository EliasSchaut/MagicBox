#ifndef HARDWARE_H
#define HARDWARE_H

#include <Keypad.h>
#include <LedControl.h>
#include <MFRC522.h>
#include "types.h"

// Keypad
extern Keypad customKeypad;

// LED Matrix
extern LedControl lc;

// RFID reader (MFRC522, hardware SPI)
extern MFRC522 rfid;

// Joystick
struct JoystickData {
    int x;
    int y;
    bool pressed;
};

JoystickData readJoystick();
Direction readJoystrickDirection();
void setupHardware();

#endif
