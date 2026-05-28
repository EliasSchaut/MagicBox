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

#endif //MAGICBOX_MAP_H
