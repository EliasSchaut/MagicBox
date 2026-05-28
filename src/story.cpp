#include "story.h"
#include "map.h"

// -----------------
// Classes & Globals
// -----------------
StoryGraph storyGraph;
// -----------------


// -----------------
// onEnter callbacks
// -----------------
static void onEnterForest() {
  mapSetTarget(4, 4, 2);
  mapEnable();
}

static void onEnterCave() {
  mapSetTarget(4, 4, 0);
  mapEnable();
}
// -----------------


// -----------------
// Content
// -----------------
StoryNode startNode = {
  "You are in a room. A) Door B) Window",
  0, nullptr, nullptr, nullptr, nullptr, nullptr
};

StoryNode forestNode = {
  "The forest is dark. Walk to the glowing point on the map, or B) Go back",
  1, nullptr, nullptr, nullptr, nullptr, onEnterForest
};

StoryNode caveNode = {
  "It's a smelly cave. Walk to the glowing point to leave, or B) Go back",
  2, nullptr, nullptr, nullptr, nullptr, onEnterCave
};
// -----------------


// -----------------
// StoryGraph Implementation
// -----------------
StoryGraph::StoryGraph() : nodes{}, nodeCount(0), currentNode(nullptr) {}

void StoryGraph::addNode(StoryNode* node) {
  if (nodeCount < 20) {
    nodes[nodeCount++] = node;
  }
}

StoryNode* StoryGraph::findNodeByID(int id) {
  for (int i = 0; i < nodeCount; i++) {
    if (nodes[i]->gameStateID == id) {
      return nodes[i];
    }
  }
  return nullptr;
}

void StoryGraph::connectNodes(int fromID, Choice choice, int toID) {
  StoryNode* fromNode = findNodeByID(fromID);
  StoryNode* toNode = findNodeByID(toID);

  if (fromNode && toNode) {
    switch (choice) {
      case Choice::A: fromNode->a = toNode; break;
      case Choice::B: fromNode->b = toNode; break;
      case Choice::C: fromNode->c = toNode; break;
      case Choice::D: fromNode->d = toNode; break;
      default: break;
    }
  }
}

void StoryGraph::enterNode(StoryNode* node) {
  currentNode = node;
  mapDisable();
  printSerial(currentNode->display);
  if (currentNode->onEnter) currentNode->onEnter();
}

void StoryGraph::jumpToNode(int stateID) {
  StoryNode* node = findNodeByID(stateID);
  if (node) {
    enterNode(node);
  }
}

void StoryGraph::handleChoice(Choice choice) {
  StoryNode* nextNode = nullptr;

  switch(choice) {
    case Choice::A: nextNode = currentNode->a; break;
    case Choice::B: nextNode = currentNode->b; break;
    case Choice::C: nextNode = currentNode->c; break;
    case Choice::D: nextNode = currentNode->d; break;
    default: break;
  };

  if (nextNode != nullptr) {
    enterNode(nextNode);
  } else {
    printWrongChoice();
  }
}
// -----------------


// -----------------
// Handler
// -----------------
void printCurrent() {
  if (storyGraph.getCurrentNode()) {
    printSerial(storyGraph.getCurrentNode()->display);
  }
};

void storyBegin() {
  storyGraph.addNode(&startNode);
  storyGraph.addNode(&forestNode);
  storyGraph.addNode(&caveNode);

  storyGraph.connectNodes(0, Choice::A, 1);
  storyGraph.connectNodes(0, Choice::B, 2);
  storyGraph.connectNodes(1, Choice::B, 0);
  storyGraph.connectNodes(2, Choice::B, 0);

  storyGraph.jumpToNode(0);
};

void printWrongChoice() {
  printSerial("You can't make this choice");
};

void handleChoice(Choice choice) {
  storyGraph.handleChoice(choice);
};

void setGameState(int state) {
  storyGraph.jumpToNode(state);
}
// -----------------
