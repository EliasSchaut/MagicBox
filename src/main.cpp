#include "hardware.cpp"
#include "story.cpp"
#include "map.cpp"
#include "utils.h"

void setup() {
    Serial.begin(9600);
    setupHardware();
    //printIntro();
}

void handleInput(char key) {
    Choice choice = charToChoice(key);
    return handleChoice(choice);
}

void loop() {
    mapWalk();

    char choice = customKeypad.getKey();
    if (choice){
        handleInput(choice);
    }
}