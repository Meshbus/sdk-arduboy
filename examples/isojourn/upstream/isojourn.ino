#include <Arduboy2.h>
#include <ArduboyFX.h>
#include "data.h"
#include "Controls.h"

#define FX_DATA_PAGE 0xFEBE
#define FRAME_RATE 60
#define ISOWIDTH 40
#define ISOHEIGHT 20

#define MAP_TILE_SIZE 3

Arduboy2 arduboy;
Controls controls;

int8_t playerX = 0;
int8_t playerY = 0;

uint8_t mapBuffer[8*4*MAP_TILE_SIZE];

int8_t isoX(int8_t x, int8_t y) {
  return x*ISOWIDTH+(y%2)*(ISOWIDTH/2) - 20;
}

int8_t isoY(int8_t y) {
  return y*(ISOHEIGHT/2) - 46;
}

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(FRAME_RATE);
  FX::disableOLED(); // OLED must be disabled before cart can be used. OLED display should only be enabled prior updating the display.
  FX::begin(FX_DATA_PAGE); // wakeup external flash chip, initialize datapage, detect presence of external flash chip
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();

  Controls::Direction playerFrame = controls.getDirection();

  bool evenRow = (playerY % 2) == 0;
  
  if (controls.doMove()) {
    if (playerFrame == Controls::dirSouth) {
      playerY+=2;
    }
    else if (playerFrame == Controls::dirSouthEast) {
      playerY+=1;
      if (!evenRow) {
        playerX+=1;
      }    
    }
    else if (playerFrame == Controls::dirEast) {
      playerX+=1;
    }
    else if (playerFrame == Controls::dirNorthEast) {
      playerY-=1;
      if (!evenRow) {
        playerX+=1;
      }    
    }
    else if (playerFrame == Controls::dirNorth) {
      playerY-=2;
    }
    else if (playerFrame == Controls::dirNorthWest) {
      playerY-=1;
      if (evenRow) {
        playerX-=1;
      }    
    }
    else if (playerFrame == Controls::dirWest) {
      playerX-=1;
    }
    else if (playerFrame == Controls::dirSouthWest) {
      playerY+=1;
      if (evenRow) {
        playerX-=1;
      }    
    }
  }

  for (int8_t y=0; y < 8; y++) {
    int8_t offset = (playerY%2) && (y%2); // if starting with an odd row, add offset to odd rows
    FX::readDataArray(MAP_START,
                      playerY+y,                // y
                      (playerX+offset)*MAP_TILE_SIZE,    // x
                      MAP_WIDTH*MAP_TILE_SIZE,  // row size
                      mapBuffer+y*4*MAP_TILE_SIZE,
                      4*MAP_TILE_SIZE);
  }

  // Fill screen
  for (int8_t y = 0; y < 8; y++) {
    int8_t iy = isoY(y);
    for (int8_t x = 0; x < 4; x++) {
      uint8_t tile = mapBuffer[y*4*MAP_TILE_SIZE+x*MAP_TILE_SIZE];
      uint8_t obj  = mapBuffer[y*4*MAP_TILE_SIZE+x*MAP_TILE_SIZE+1];
      uint8_t prop = mapBuffer[y*4*MAP_TILE_SIZE+x*MAP_TILE_SIZE+2];

      uint8_t frame = 0;
      int8_t ix = isoX(x,y);
      
      if ((tile == TILE_decoWater)  ||
          (tile == TILE_decoReed)   ||
          (tile == TILE_groundPondrock)) {
          frame = (((arduboy.frameCount) >> 4)+ (x*2) + (y%2)) & 0x7;
          if (frame > 3) {
            frame = 7 - frame;
          }
      }
      else if (tile == TILE_decoCampfire) {
        frame = ((arduboy.frameCount) >> 5) % 3;
      }

      FX::drawBitmap(ix,iy,TILE_START,tile+frame,dbmMasked);

      frame = 0;
      if (obj == TILE_objCloud) {
          frame = (((arduboy.frameCount) >> 4)+ (x*2) + (y%2)) & 0x7;
          if (frame > 3) {
            frame = 7 - frame;
          }
      }
      
      if (obj != 0) {
        FX::drawBitmap(ix,iy,TILE_START,obj+frame,dbmMasked);
      }
      
      // draw player on top of map
      if ((x==1) && (y==5)) {
        if ((playerFrame == 0) &&
            ((((arduboy.frameCount) >> 3) & 0x3f) == 0)) { 
          FX::drawBitmap(ix,iy,TILE_START,TILE_characterOldmanb,dbmMasked);
        }
        else {
          FX::drawBitmap(ix,iy,TILE_START,TILE_characterOldman+playerFrame,dbmMasked);
        }
      }
    }
  }

//  for (int8_t y = 0; y < 8; y++) {
//    for (int8_t x = 0; x < 4; x++) {
//      arduboy.setCursor(x*8*3,y*8);
//      arduboy.print(mapBuffer[y*4*MAP_TILE_SIZE+x*MAP_TILE_SIZE],HEX);
//    }
//  }
  
  FX::enableOLED();// only enable OLED for updating the display
  arduboy.display(CLEAR_BUFFER);
  FX::disableOLED();// disable so flash cart can be used at any time
}
