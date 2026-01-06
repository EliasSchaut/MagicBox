//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_STORY_H
#define MAGICBOX_STORY_H

#include "types.h"
#include "utils.h"

extern StoryNode startNode, forestNode, caveNode;

void printCurrent();
void printIntor();
void printWrongChoice();
void handleChoice(Choice choice);

#endif //MAGICBOX_STORY_H