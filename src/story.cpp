#include "story.h"
#include "map.h"
#include <string.h>

// -----------------
// Classes & Globals
// -----------------
StoryGraph storyGraph;
// -----------------
//
// Note: the authored story content (node literals + storyBegin) lives in
// src/story_content.cpp, which is the file the editor's "Export C++" overwrites.
// This file holds only the engine.


// -----------------
// StoryGraph Implementation
// -----------------
StoryGraph::StoryGraph() : nodes{}, nodeCount(0), currentNode(nullptr) {}

void StoryGraph::addNode(StoryNode* node) {
  if (nodeCount < 20) nodes[nodeCount++] = node;
}

StoryNode* StoryGraph::findNodeByID(int id) {
  for (int i = 0; i < nodeCount; i++) {
    if (nodes[i]->id == id) return nodes[i];
  }
  return nullptr;
}

void StoryGraph::enterNode(StoryNode* node) {
  // `next` chains let a node auto-advance to another after showing its text.
  // Loop instead of recursing, with a guard against cyclic `next` pointers.
  int guard = 0;
  while (node && guard++ < 64) {
    currentNode = node;

    // Default after every transition: map off. The declarative config below
    // (and/or onEnter) may turn it back on.
    mapDisable();

    printSerial(node->display);

    // Apply declarative map config.
    if (node->mapTargets) {
      for (int i = 0; i < node->mapTargetCount; i++) {
        const MapTarget& t = node->mapTargets[i];
        mapSetTarget(t.x, t.y, t.storyID);
      }
    }
    if (node->activateMap) mapEnable();

    // Escape hatch (e.g. mapTeleportPlayer(...)).
    if (node->onEnter) node->onEnter();

    // Direct transition: continue to `next` without waiting for input.
    node = node->next;
  }
}

void StoryGraph::jumpToNode(int id) {
  StoryNode* node = findNodeByID(id);
  if (node) enterNode(node);
}

void StoryGraph::handleChoice(Choice choice) {
  if (!currentNode) return;
  StoryNode* next = nullptr;
  switch (choice) {
    case Choice::A: next = currentNode->choiceA; break;
    case Choice::B: next = currentNode->choiceB; break;
    case Choice::C: next = currentNode->choiceC; break;
    case Choice::D: next = currentNode->choiceD; break;
    default: break;
  }
  if (next) enterNode(next);
  else printWrongChoice();
}

void StoryGraph::handlePin(const char* pin) {
  if (!currentNode || !currentNode->expectedPin || !currentNode->pinSuccess) {
    printSerial("No PIN expected here");
    return;
  }
  if (strcmp(pin, currentNode->expectedPin) == 0) {
    enterNode(currentNode->pinSuccess);
  } else {
    printSerial("Wrong PIN");
  }
}
// -----------------


// -----------------
// Handler
// -----------------
void printCurrent() {
  if (storyGraph.getCurrentNode()) printSerial(storyGraph.getCurrentNode()->display);
}

// storyBegin() is defined in story_content.cpp (authored content).

void printWrongChoice() {
  printSerial("You can't make this choice\n");
}

void handleChoice(Choice choice) {
  storyGraph.handleChoice(choice);
}

void handlePin(const char* pin) {
  storyGraph.handlePin(pin);
}

void setGameState(int state) {
  storyGraph.jumpToNode(state);
}
// -----------------
