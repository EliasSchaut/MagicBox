//
// Created by elias on 06.01.2026.
//

#ifndef MAGICBOX_STORY_H
#define MAGICBOX_STORY_H

#include "types.h"
#include "utils.h"

class StoryGraph {
public:
    StoryGraph();
    void addNode(StoryNode* node);
    void jumpToNode(int id);
    StoryNode* getCurrentNode() const { return currentNode; }
    void handleChoice(Choice choice);
    void handlePin(const char* pin);

private:
    StoryNode* nodes[20]{}; // Assuming a maximum of 20 nodes for this simple implementation
    int nodeCount;
    StoryNode* currentNode;
    StoryNode* findNodeByID(int id);
    void enterNode(StoryNode* node);
};

// Node literals live in src/story_content.cpp and reference each other via
// their own forward declarations there — no global extern list needed here.
extern StoryGraph storyGraph;

void printCurrent();
void storyBegin();
void printWrongChoice();
void handleChoice(Choice choice);
void handlePin(const char* pin);
void setGameState(int state);

// Jump to the saved node from a previous power cycle if one exists for THIS
// firmware build (EEPROM), otherwise start at startId. Called from storyBegin().
void resumeOrStart(int startId);

#endif //MAGICBOX_STORY_H
