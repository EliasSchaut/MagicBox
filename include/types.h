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

enum class Choice {  A = 'A',  B = 'B',  C = 'C',  D = 'D', NONE = '\0' };

struct MapTarget {
    int x;
    int y;
    int storyID;
};

struct StoryNode;

// StoryNode is a plain aggregate authored with designated initializers.
// avr-gcc 7's caveat: a designated initializer must set a CONTIGUOUS PREFIX of
// the fields (in declaration order, no gaps). Trailing fields may be omitted
// and are zero-initialized (→ nullptr / false / 0).
//
// So the field order is chosen so the most common scenarios trail-skip cleanly:
//   - "merge / auto-advance"         → set .next, trail-skip
//   - "just choices"                 → set up to .choiceD, trail-skip
//   - "choices + map"                → set up to .mapTargets, trail-skip
//   - "choices + map + pin"          → set up to .pinSuccess, trail-skip
//   - anything fancier               → use .onEnter (custom callback)
//
// Use the helper macros below (GOTO, CHOICES, MAP_TARGETS, MAP_OFF, ...) so you
// don't have to manually fill the gap with nullptr/0/false when you want to
// reach a later field.
struct StoryNode {
    int id;
    const char* display;

    // Direct transition: if non-null, after this node's display text (and map
    // config / onEnter) is shown, the engine immediately continues to `next`
    // without waiting for input. Used to merge several branches back together.
    // Authored via GOTO(node); CHOICES(...) sets it to nullptr automatically.
    StoryNode* next;

    // Choices — nullptr = invalid choice.
    StoryNode* choiceA;
    StoryNode* choiceB;
    StoryNode* choiceC;
    StoryNode* choiceD;

    // Map config — applied declaratively on enter.
    // Authored via the MAP_TARGETS(...) / MAP_OFF macros so you don't manage
    // mapTargetCount by hand.
    bool activateMap;
    int mapTargetCount;
    const MapTarget* mapTargets;

    // PIN entry. Both nullptr → node accepts no PIN.
    const char* expectedPin;
    StoryNode* pinSuccess;

    // Escape hatch for anything the declarative fields don't cover (e.g.
    // teleporting the player via mapTeleportPlayer(x, y)). Runs after the
    // declarative map config is applied.
    void (*onEnter)();
};

// ----- Authoring helpers ---------------------------------------------------

// Direct transition / merge node: show this node's display text, then continue
// straight to `node` (no input needed). Place right after .display; the rest of
// the fields trail-skip. Example:
//   StoryNode mergeNode = { .id = 9, .display = "Paths converge.", GOTO(&hubNode) };
#define GOTO(node) .next = (node)

// All four choices at once. Pass nullptr where there is no transition.
// Implicitly sets .next = nullptr (so it stays a contiguous prefix and the node
// waits for input instead of auto-advancing).
#define CHOICES(a, b, c, d) \
    .next = nullptr, .choiceA = (a), .choiceB = (b), .choiceC = (c), .choiceD = (d)

// Short-hand: no choices at all (e.g. dead-end intermediate node).
#define NO_CHOICES CHOICES(nullptr, nullptr, nullptr, nullptr)

// Activate the map and define its blinking targets in one go. Each argument
// is a {x, y, storyID} brace-initialiser:
//   MAP_TARGETS({4, 4, 2}, {7, 0, 5})
#define MAP_TARGETS(...) \
    .activateMap = true, \
    .mapTargetCount = (int)(sizeof((const MapTarget[]){__VA_ARGS__}) / sizeof(MapTarget)), \
    .mapTargets = (const MapTarget[]){__VA_ARGS__}

// Explicit "this node does not show the map". Only needed when a later field
// (PIN, onEnter) must be set on a node that has no map — designated init
// can't skip middle fields in avr-gcc 7.
#define MAP_OFF \
    .activateMap = false, \
    .mapTargetCount = 0, \
    .mapTargets = nullptr

// PIN entry pair.
#define PIN(expected, successNode) \
    .expectedPin = (expected), .pinSuccess = (successNode)

#endif //MAGICBOX_TYPES_H
