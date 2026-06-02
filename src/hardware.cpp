#include "hardware.h"

// --------------
// Keypad
// --------------
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
// --------------


// --------------
// LED Matrix
// --------------
/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12, 10, 11, 1);
// --------------


// --------------
// Joystick
// --------------
const int JoySW = 13; // digital pin connected to switch output
const int JoyX = 0; // analog pin connected to X output
const int JoyY = 1; // analog pin connected to Y output

JoystickData readJoystick() {
    JoystickData data{};
    data.x = analogRead(JoyX);
    data.y = analogRead(JoyY);
    data.pressed = (digitalRead(JoySW) == LOW);
    return data;
};

Direction readJoystrickDirection() {
    JoystickData data = readJoystick();
    if (data.x < 300) return Direction::LEFT;
    if (data.x > 700) return Direction::RIGHT;
    if (data.y < 300) return Direction::DOWN;
    if (data.y > 700) return Direction::UP;
    return Direction::NONE;
};
// --------------

void setupHardware() {
    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);
    pinMode(JoySW, INPUT);
    digitalWrite(JoySW, HIGH);
};
