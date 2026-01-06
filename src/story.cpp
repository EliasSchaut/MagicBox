#include "types.h"
// -----------------
// Classes & Globals
// -----------------
const StoryNode* currentNode = nullptr;
// -----------------


// -----------------
// Content
// -----------------
extern StoryNode startNode, forestNode, caveNode;

StoryNode startNode = {
  "You are in a room. A) Door B) Window", 
  0, &forestNode, &caveNode, nullptr, nullptr 
};

StoryNode forestNode = {
  "The forest is dark. A) Walk B) Go back", 
  1, &forestNode, &startNode, nullptr, nullptr 
};

StoryNode caveNode = {
  "It's a smelly cave. B) Go back", 
  2, &caveNode, &startNode, nullptr, nullptr 
};
// -----------------


// -----------------
// Handler
// -----------------
void printCurrent() {
  printSerial(currentNode->display);
};

void printIntro() {
  currentNode = &startNode;
  printCurrent();
};

void printWrongChoice() {
  printSerial("You can't make this choice");
};

void handleChoice(Choice choice) {
  StoryNode* nextNode = nullptr;

  switch(choice) {
    case Choice::A: nextNode = currentNode->a; break;
    case Choice::B: nextNode = currentNode->b; break;
    case Choice::C: nextNode = currentNode->c; break;
    case Choice::D: nextNode = currentNode->d; break;
    default: break;
  };

  if (nextNode != nullptr) {
    currentNode = nextNode;
    printSerial(currentNode->display);
  } else {
    printWrongChoice();
  }
};
// -----------------
