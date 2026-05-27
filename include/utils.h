//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_UTILS_H
#define MAGICBOX_UTILS_H
#include <HardwareSerial.h>

#include "types.h"

inline Choice charToChoice(char key) {
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
        return static_cast<Choice>(key);
    }
    return Choice::NONE;
}

inline void printSerial(const char* output) {
    Serial.println(F("\n--------------------------------"));
    Serial.println(output);
    Serial.println();
    Serial.println();
}

inline void printDirection(Direction dir) {
    Serial.print(F("Moved: "));
    switch (dir) {
        case Direction::UP: Serial.println(F("UP"));
            break;
        case Direction::DOWN: Serial.println(F("DOWN"));
            break;
        case Direction::LEFT: Serial.println(F("LEFT"));
            break;
        case Direction::RIGHT: Serial.println(F("RIGHT"));
            break;
        case Direction::NONE: Serial.println(F("NONE"));
            break;
    }
}

#endif //MAGICBOX_UTILS_H