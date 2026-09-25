#include "Controls.h"
extern Arduboy2 arduboy;

Controls::Direction Controls::getDirection() {
  Direction newDir = lastDir;
  
  if (arduboy.justPressed(DOWN_BUTTON)) {
    if (arduboy.pressed(RIGHT_BUTTON)) {
      newDir = dirSouthEast;
    }
    else if (arduboy.pressed(LEFT_BUTTON)) {
      newDir = dirSouthWest;
    }
    else {
      newDir = dirSouth;
    }
  }
  else if (arduboy.justPressed(UP_BUTTON)) {
    if (arduboy.pressed(RIGHT_BUTTON)) {
      newDir =  dirNorthEast;
    }
    else if (arduboy.pressed(LEFT_BUTTON)) {
      newDir =  dirNorthWest;
    }
    else {
      newDir =  dirNorth;
    }
  }
  else if (arduboy.justPressed(RIGHT_BUTTON)) {
    if (arduboy.pressed(DOWN_BUTTON)){
      newDir = dirSouthEast;
    }
    else if (arduboy.pressed(UP_BUTTON)){
      newDir = dirNorthEast;
    }
    else {
      newDir = dirEast;
    }
  }
  else if (arduboy.justPressed(LEFT_BUTTON)) {
    if (arduboy.pressed(DOWN_BUTTON)){
      newDir = dirSouthWest;
    }
    else if (arduboy.pressed(UP_BUTTON)){
      newDir = dirNorthWest;
    }
    else {
      newDir = dirWest;
    }
  }
  lastDir = newDir;
  return(newDir);
}


bool Controls::doMove() {
  if (arduboy.justPressed(A_BUTTON)) {
    return true;
  }
  return false;
}
