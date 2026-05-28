#include "main.h"

namespace {
    constexpr int PIN_BUFFER_SIZE = 16;
    char pinBuffer[PIN_BUFFER_SIZE];
    int pinLength = 0;
    bool pinActive = false;
}

void setup() {
    Serial.begin(9600);
    setupHardware();
    storyBegin();
}

void loop() {
    mapWalk();

    char key = customKeypad.getKey();
    if (key) {
        handleInput(key);
    }
}

void handleInput(char key) {
    if (key == '*') {
        pinActive = true;
        pinLength = 0;
        return;
    }
    if (key == '#') {
        if (pinActive && pinLength > 0) {
            pinBuffer[pinLength] = '\0';
            handlePin(pinBuffer);
        }
        pinActive = false;
        pinLength = 0;
        return;
    }
    if (key >= '0' && key <= '9') {
        if (pinActive && pinLength < PIN_BUFFER_SIZE - 1) {
            pinBuffer[pinLength++] = key;
        }
        return;
    }

    Choice choice = charToChoice(key);
    if (choice != Choice::NONE) {
        handleChoice(choice);
    }
}
