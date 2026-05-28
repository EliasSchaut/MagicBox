#include "story.h"
#include "map.h"
#include <string.h>

// -----------------
// Classes & Globals
// -----------------
StoryGraph storyGraph;
// -----------------


// -----------------
// Content — fully declarative. The graph wiring lives in the literals; only
// fields you set matter, the rest are zero-initialized (= nullptr / false / 0).
// Use the helper macros from types.h to avoid manually filling gaps.
// -----------------
StoryNode startNode = {
    .id = 0,
    .display = "You are in a room. A) Door B) Window C) Safe",
    CHOICES(&forestNode, &caveNode, &safeNode, nullptr),
};

StoryNode forestNode = {
    .id = 1,
    .display = "The forest is dark. Walk to the glowing point on the map, or B) Go back",
    CHOICES(nullptr, &startNode, nullptr, nullptr),
    MAP_TARGETS({4, 4, 2}),
};

StoryNode caveNode = {
    .id = 2,
    .display = "It's a smelly cave. Walk to the glowing point to leave, or B) Go back",
    CHOICES(nullptr, &startNode, nullptr, nullptr),
    MAP_TARGETS({4, 4, 0}),
};

StoryNode safeNode = {
    .id = 3,
    .display = "A locked safe. Enter PIN as *NNNN# (hint: 1234). B) Back",
    CHOICES(nullptr, &startNode, nullptr, nullptr),
    MAP_OFF,
    PIN("1234", &treasureNode),
};

StoryNode treasureNode = {
    .id = 4,
    .display = "The safe pops open. You found gold! A) Back to room",
    CHOICES(&startNode, nullptr, nullptr, nullptr),
};
// -----------------


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

void storyBegin() {
  // Just register every node — the graph is wired entirely via pointers in
  // the literals above. addNode() only feeds the ID→node lookup used by
  // setGameState() (which the map collision path calls).
  storyGraph.addNode(&startNode);
  storyGraph.addNode(&forestNode);
  storyGraph.addNode(&caveNode);
  storyGraph.addNode(&safeNode);
  storyGraph.addNode(&treasureNode);

  storyGraph.jumpToNode(0);
}

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
