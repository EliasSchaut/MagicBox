#include "map.h"

bool minigame_not_finished = true;

Position position = Position(0, 0);

void mapWalk() {
  Direction moveDir = readJoystrickDirection();
  if (moveDir == Direction::NONE) return;
  lc.setLed(0, position.x, position.y, false);
  position.move(moveDir);
  lc.setLed(0, position.x, position.y, true);
  delay(300);
};
