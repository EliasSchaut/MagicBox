//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_UTILS_H
#define MAGICBOX_UTILS_H
#include <HardwareSerial.h>

#include "types.h"

inline void printSerial(const char* output) {
    Serial.println(F("\n--------------------------------"));
    Serial.println(output);
    Serial.println();
    Serial.println();
}

inline Choice charToChoice(char key) {
    switch(key) {
        case 'A': return Choice::A;
        case 'B': return Choice::B;
        case 'C': return Choice::C;
        case 'D': return Choice::D;
        default:  return Choice::NONE;
    }
}

#endif //MAGICBOX_UTILS_H