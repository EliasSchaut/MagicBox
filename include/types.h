//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_TYPES_H
#define MAGICBOX_TYPES_H

#include <Arduino.h>

enum class Direction { UP, DOWN, LEFT, RIGHT, NONE };

struct Position {
    int x;
    int y;

    Position(int xPos = 0, int yPos = 0) : x(xPos), y(yPos) {};

    void move(Direction dir) {
        switch (dir) {
            case Direction::UP:
                y = constrain(y + 1, 0, 7);
                break;
            case Direction::DOWN:
                y = constrain(y - 1, 0, 7);
                break;
            case Direction::LEFT:
                x = constrain(x - 1, 0, 7);
                break;
            case Direction::RIGHT:
                x = constrain(x + 1, 0, 7);
                break;
            case Direction::NONE:
                break;
        };
    };
};

enum class Choice {  A,  B,  C,  D, NONE };

struct StoryNode {
    const char* display;
    int gameStateID;
    StoryNode* a;
    StoryNode* b;
    StoryNode* c;
    StoryNode* d;
};

#endif //MAGICBOX_TYPES_H