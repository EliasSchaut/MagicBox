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
    void connectNodes(int fromID, Choice choice, int toID);
    void jumpToNode(int stateID);
    StoryNode* getCurrentNode() const { return currentNode; }
    void handleChoice(Choice choice);

private:
    StoryNode* nodes[20]{}; // Assuming a maximum of 20 nodes for this simple implementation
    int nodeCount;
    StoryNode* currentNode;
    StoryNode* findNodeByID(int id);
};

extern StoryNode startNode, forestNode, caveNode;
extern StoryGraph storyGraph;

void printCurrent();
void storyBegin();
void printWrongChoice();
void handleChoice(Choice choice);
void setGameState(int state);

#endif //MAGICBOX_STORY_H