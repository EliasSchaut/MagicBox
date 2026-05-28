// =============================================================
// Story content — the authored game graph.
//
// This whole file is safe to overwrite with the "Export C++" output from
// editor.html. It holds ONLY content (node literals + storyBegin); the engine
// lives in story.cpp and is never touched by the export.
// =============================================================
#include "story.h"
#include "map.h"

// ---- forward declarations (so literals can reference each other) ----
extern StoryNode startNode, forestNode, caveNode, safeNode, treasureNode, leaveNode;

// ---- onEnter callbacks ----
// (none)

// ---- nodes ----
// Fully declarative; only the fields you set matter (rest = nullptr/false/0).
// See the authoring macros in types.h (CHOICES, GOTO, MAP_TARGETS, MAP_OFF, PIN).
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
    .display = "The safe pops open. You found gold! A) Leave",
    CHOICES(&leaveNode, nullptr, nullptr, nullptr),
};

// Direct-transition (merge) node: shows its line, then auto-advances to
// startNode without waiting for input.
StoryNode leaveNode = {
    .id = 5,
    .display = "You pocket the gold and head back...",
    GOTO(&startNode),
};

// ---- registration ----
// Just register every node (feeds the id->node lookup used by setGameState /
// map collisions) and jump to the start node.
void storyBegin() {
  storyGraph.addNode(&startNode);
  storyGraph.addNode(&forestNode);
  storyGraph.addNode(&caveNode);
  storyGraph.addNode(&safeNode);
  storyGraph.addNode(&treasureNode);
  storyGraph.addNode(&leaveNode);

  storyGraph.jumpToNode(0);
}
