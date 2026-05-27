#include "main.h"

void setup() {
    Serial.begin(9600);
    setupHardware();
    storyBegin();
}

void loop() {
    mapWalk();

    char choice = customKeypad.getKey();
    if (choice){
        handleInput(choice);
    }
}

void handleInput(char key) {
    Choice choice = charToChoice(key);
    return handleChoice(choice);
}
