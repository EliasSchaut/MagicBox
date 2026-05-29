#include "map.h"
#include "story.h"

namespace {
    constexpr int MAX_TARGETS = 40;   // shared by blinking targets and blockers
    constexpr unsigned long BLINK_INTERVAL_MS = 250;
    constexpr unsigned long MOVE_DELAY_MS = 300;

    Position playerPos{0, 0};
    bool mapActive = false;

    MapTarget targets[MAX_TARGETS];
    int targetCount = 0;

    bool blinkOn = true;
    unsigned long lastBlinkToggle = 0;

    // Cached frame: rows of the 8x8 matrix. Each byte's bits are columns
    // following LedControl's convention (B10000000 >> column).
    byte frameBuffer[8] = {0};

    int findTargetIndex(int x, int y) {
        for (int i = 0; i < targetCount; i++) {
            if (targets[i].x == x && targets[i].y == y) return i;
        }
        return -1;
    }

    bool isBlockerAt(int x, int y) {
        int idx = findTargetIndex(x, y);
        return idx >= 0 && targets[idx].storyID == MAP_BLOCK;
    }

    void renderFrame() {
        byte newFrame[8] = {0};
        if (mapActive) {
            for (int i = 0; i < targetCount; i++) {
                // Blockers are always lit; targets blink.
                if (targets[i].storyID == MAP_BLOCK || blinkOn) {
                    newFrame[targets[i].x] |= (byte)(B10000000 >> targets[i].y);
                }
            }
            newFrame[playerPos.x] |= (byte)(B10000000 >> playerPos.y);
        }
        for (int row = 0; row < 8; row++) {
            if (newFrame[row] != frameBuffer[row]) {
                lc.setRow(0, row, newFrame[row]);
                frameBuffer[row] = newFrame[row];
            }
        }
    }

    bool checkCollision() {
        int idx = findTargetIndex(playerPos.x, playerPos.y);
        if (idx < 0) return false;
        int storyID = targets[idx].storyID;
        if (storyID == MAP_BLOCK) return false;   // blockers never trigger a story
        setGameState(storyID);
        return true;
    }
}

void mapEnable() {
    mapActive = true;
    blinkOn = true;
    lastBlinkToggle = millis();
    renderFrame();
}

void mapDisable() {
    if (!mapActive) return;
    mapActive = false;
    renderFrame();
}

bool mapIsActive() {
    return mapActive;
}

void mapTeleportPlayer(int x, int y) {
    playerPos.x = constrain(x, 0, 7);
    playerPos.y = constrain(y, 0, 7);
    if (mapActive) renderFrame();
}

void mapSetTarget(int x, int y, int storyID) {
    int idx = findTargetIndex(x, y);
    if (idx >= 0) {
        targets[idx].storyID = storyID;
    } else if (targetCount < MAX_TARGETS) {
        targets[targetCount].x = x;
        targets[targetCount].y = y;
        targets[targetCount].storyID = storyID;
        targetCount++;
    }
    if (mapActive) renderFrame();
}

void mapRemoveTarget(int x, int y) {
    int idx = findTargetIndex(x, y);
    if (idx < 0) return;
    for (int i = idx; i < targetCount - 1; i++) {
        targets[i] = targets[i + 1];
    }
    targetCount--;
    if (mapActive) renderFrame();
}

void mapClearTargets() {
    targetCount = 0;
    if (mapActive) renderFrame();
}

void mapSetBlocker(int x, int y) {
    mapSetTarget(x, y, MAP_BLOCK);
}

void mapRemoveBlocker(int x, int y) {
    int idx = findTargetIndex(x, y);
    if (idx >= 0 && targets[idx].storyID == MAP_BLOCK) {
        mapRemoveTarget(x, y);
    }
}

void mapClearBlockers() {
    // Remove only blocker entries, keeping real targets (progress) intact.
    int w = 0;
    for (int i = 0; i < targetCount; i++) {
        if (targets[i].storyID != MAP_BLOCK) {
            targets[w++] = targets[i];
        }
    }
    targetCount = w;
    if (mapActive) renderFrame();
}

void mapWalk() {
    if (!mapActive) return;

    Direction moveDir = readJoystrickDirection();
    if (moveDir != Direction::NONE) {
        Position next = playerPos;
        next.move(moveDir);
        bool moved = (next.x != playerPos.x || next.y != playerPos.y);
        if (moved && !isBlockerAt(next.x, next.y)) {
            playerPos = next;
            renderFrame();
            if (checkCollision()) return;
            delay(MOVE_DELAY_MS);
            return;
        }
    }

    unsigned long now = millis();
    if (now - lastBlinkToggle >= BLINK_INTERVAL_MS) {
        lastBlinkToggle = now;
        blinkOn = !blinkOn;
        renderFrame();
    }
}
