#include "map.h"

bool minigame_not_finished = true;

Position position = Position(0, 0);

void mapWalk() {
  Direction moveDir = readJoystrickDirection();
  if (moveDir == Direction::NONE) return;
  Position next = position;
  next.move(moveDir);
  if (next.x == position.x && next.y == position.y) return;
  lc.setLed(0, position.x, position.y, false);
  position = next;
  lc.setLed(0, position.x, position.y, true);
  delay(300);
};
