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

// Plain output — just prints the text, no decoration.
// Overloaded for RAM strings (const char*) and flash strings (F()/FSTR()).
inline void printSerial(const char* output) {
    Serial.print(output);
}
inline void printSerial(const __FlashStringHelper* output) {
    Serial.print(output);
}

// Block output — wraps the text in a separator + trailing blank lines, for
// rendering larger paragraphs (e.g. story node text).
inline void printSerialBlock(const char* output) {
    Serial.println(F("\n--------------------------------"));
    Serial.println(output);
    Serial.println();
    Serial.println();
}
inline void printSerialBlock(const __FlashStringHelper* output) {
    Serial.println(F("\n--------------------------------"));
    Serial.println(output);
    Serial.println();
    Serial.println();
}

inline void printDirection(Direction dir) {
    printSerial("Moved: ");
    switch (dir) {
        case Direction::UP: printSerial("UP\n");
            break;
        case Direction::DOWN: printSerial("DOWN\n");
            break;
        case Direction::LEFT: printSerial("LEFT\n");
            break;
        case Direction::RIGHT: printSerial("RIGHT\n");
            break;
        case Direction::NONE: printSerial("NONE\n");
            break;
    }
}

#endif //MAGICBOX_UTILS_H