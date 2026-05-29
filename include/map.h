//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_MAP_H
#define MAGICBOX_MAP_H

#include "types.h"
#include "hardware.h"

void mapWalk();

void mapEnable();
void mapDisable();
bool mapIsActive();

void mapTeleportPlayer(int x, int y);

void mapSetTarget(int x, int y, int storyID);
void mapRemoveTarget(int x, int y);
void mapClearTargets();

// Blockers: constant-lit cells the player cannot walk through. Stored in the
// same table as targets (with storyID == MAP_BLOCK).
void mapSetBlocker(int x, int y);
void mapRemoveBlocker(int x, int y);
void mapClearBlockers();

#endif //MAGICBOX_MAP_H
