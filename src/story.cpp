#include "story.h"
#include "map.h"
#include "nfc.h"
#include <string.h>
#include <EEPROM.h>

// -----------------
// Classes & Globals
// -----------------
StoryGraph storyGraph;
// -----------------


// -----------------
// Progress persistence (EEPROM)
// -----------------
// The current node is saved to EEPROM on every transition so the game resumes
// after a power loss. A per-build signature (FIRMWARE_BUILD_ID, injected by
// inject_build_id.py at compile time) is stored alongside: when new firmware is
// flashed the signature no longer matches, so the save is ignored and the story
// restarts from the beginning — exactly what we want on an update.
#ifndef FIRMWARE_BUILD_ID
#define FIRMWARE_BUILD_ID 0UL   // fallback (e.g. for IDE indexing); real value injected at build
#endif

namespace {
  constexpr int SAVE_ADDR = 0;

  struct SaveData {
    uint32_t buildId;
    int16_t  nodeId;
  };

  void saveProgress(int nodeId) {
    SaveData d{ (uint32_t)FIRMWARE_BUILD_ID, (int16_t)nodeId };
    EEPROM.put(SAVE_ADDR, d);   // update-based: only writes bytes that changed
  }

  // Saved node id, or -1 if there is no valid save for THIS firmware build
  // (fresh upload, or uninitialised/corrupted EEPROM).
  int loadProgress() {
    SaveData d{};
    EEPROM.get(SAVE_ADDR, d);
    if (d.buildId != (uint32_t)FIRMWARE_BUILD_ID) return -1;
    return d.nodeId;
  }
}
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
  if (nodeCount < 64) nodes[nodeCount++] = node;
}

StoryNode* StoryGraph::findNodeByID(int id) {
  for (int i = 0; i < nodeCount; i++) {
    if (nodes[i]->id == id) return nodes[i];
  }
  return nullptr;
}

void StoryGraph::enterNode(StoryNode* node) {
  // Every transition invalidates a pending NFC request of the previous node
  // (analogous to mapDisable below). An onEnter along the way may request a
  // write; armNfc() at the end respects it via its nfcBusy() guard.
  nfcCancel();

  // `next` chains let a node auto-advance to another after showing its text.
  // Loop instead of recursing, with a guard against cyclic `next` pointers.
  int guard = 0;
  while (node && guard++ < 64) {
    currentNode = node;

    // Default after every transition: map off. The declarative config below
    // (and/or onEnter) may turn it back on.
    mapDisable();

    printSerialBlock(node->display);

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

  // Only the resting node listens for tags.
  armNfc();

  // Persist the resting node so a power loss resumes here (not intermediate
  // nodes of a `next` chain).
  if (currentNode) saveProgress(currentNode->id);
}

void StoryGraph::armNfc() {
  // A write requested by onEnter/onNfc takes priority — don't clobber it.
  // Reads for this node are then only armed on the next transition.
  if (nfcBusy()) return;
  if (currentNode && (currentNode->expectedNfc || currentNode->onNfc)) {
    nfcRequestRead(&::handleNfc); // free-function wrapper matches NfcReadCallback
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
  // Echo der eingegebenen Kombination.
  printSerial(F("Eingegebene Kombination: "));
  printSerial(pin);

  if (!currentNode || !currentNode->expectedPin || !currentNode->pinSuccess) {
    printSerial(F("\nHier wird keine Kombination erwartet! Sende Nachricht erneut:"));
    printCurrent();
    return;
  }
  if (strcmp(pin, currentNode->expectedPin) == 0) {
    enterNode(currentNode->pinSuccess);
  } else {
    printSerial(F("\nKombination ungültig! Sende Nachricht erneut:"));
    printCurrent();
  }
}

void StoryGraph::handleNfc(const char* text) {
  printSerial(F("Chip gelesen: "));
  printSerial(text);
  printSerial(F("\n"));

  if (!currentNode) return;
  StoryNode* node = currentNode;

  // 1. onNfc first — custom logic sees every tag and may itself transition.
  if (node->onNfc) node->onNfc(text);

  // If onNfc transitioned the story, the new node's enterNode already
  // cancelled/re-armed NFC — don't also apply the old node's gate.
  if (currentNode != node) return;

  // 2. Declarative gate: matching tag text advances to nfcSuccess.
  if (node->expectedNfc && node->nfcSuccess) {
    if (strcmp(text, node->expectedNfc) == 0) {
      enterNode(node->nfcSuccess);
      return;
    }
    printSerial(F("\nFalscher Chip! Versuche einen anderen:"));
    printCurrent();
  }

  // 3. No transition happened → re-arm so the next tag is read too (the
  // same-tag cooldown in nfc.cpp prevents immediate re-triggering).
  armNfc();
}
// -----------------


// -----------------
// Handler
// -----------------
void printCurrent() {
  if (storyGraph.getCurrentNode()) printSerialBlock(storyGraph.getCurrentNode()->display);
}

// storyBegin() is defined in story_content.cpp (authored content).

void printWrongChoice() {
  printSerial(F("Ungültige Auswahl! Sende Nachricht erneut:"));
  printCurrent();
}

void handleChoice(Choice choice) {
  storyGraph.handleChoice(choice);
}

void handlePin(const char* pin) {
  storyGraph.handlePin(pin);
}

void handleNfc(const char* text) {
  storyGraph.handleNfc(text);
}

void setGameState(int state) {
  storyGraph.jumpToNode(state);
}

void resumeOrStart(int startId) {
  int saved = loadProgress();
  // A matching buildId guarantees the saved id is a node of this firmware, so
  // no extra validity check is needed (we only ever save real node ids).
  storyGraph.jumpToNode(saved >= 0 ? saved : startId);
}
// -----------------
