#pragma once
#include <Arduboy2.h>

class Controls {
  public:

    enum Direction : uint8_t { dirSouth, dirSouthEast, dirEast, dirNorthEast, dirNorth, dirNorthWest, dirWest, dirSouthWest };
    
    Direction getDirection();
    bool doMove();
    
  private:
    Direction lastDir = dirSouth;
};
