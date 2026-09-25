#include <cassert>
#include <climits>
#include <string>
#include <cstdio>
#include <cstring>
#include <Arduboy2.h>
static uint8_t framebuffer[1024];
extern "C" uint8_t *meshbus_arduboy_framebuffer() { return framebuffer; }

static bool pixel(int x,int y) { return framebuffer[x+(y/8)*128]&(1<<(y%8)); }
int main() {
 char number[sizeof(long) * 8U + 2U];
 assert(std::string(ltoa(LONG_MIN, number, 10)) == std::to_string(LONG_MIN));
 // A custom game canvas must not accidentally draw into the runtime canvas.
 uint8_t custom[1152] = {};
 meshbus::arduboy::fill_rect_to(custom, 0, 31, 128, 1, WHITE);
 const uint8_t page_sprite[] = {0x03};
 meshbus::arduboy::draw_bitmap_to(custom, 4, 32, page_sprite, 1, 2, WHITE);
 const uint8_t xy_sprite[] = {0x80};
 meshbus::arduboy::draw_xy_bitmap_to(custom, 5, 34, xy_sprite, 1, 1, WHITE);
 for (int x=0;x<128;++x) assert(custom[3*128+x] == 0x80);
 assert(custom[4*128+4] == 3 && custom[4*128+5] == 4);
 for (auto byte : framebuffer) assert(byte == 0);
 for (int x=1024;x<1152;++x) assert(custom[x] == 0);
 memcpy(framebuffer, custom, 1024); // Publish before clearing the custom canvas.
 memset(custom, 0xaa, sizeof(custom));
 assert(pixel(0,31) && pixel(127,31) && pixel(4,32) && pixel(5,34));
 Arduboy2 a;
 // An MSB-first, 10-pixel-wide XY bitmap crosses a byte boundary and clips left.
 const uint8_t bitmap[]={0x80,0x40,0x40,0x80};
 a.clear(); a.drawSlowXYBitmap(-1,7,bitmap,10,2);
 assert(pixel(8,7)&&pixel(0,8)&&pixel(7,8));
 int count=0; for(int y=0;y<64;y++) for(int x=0;x<128;x++) count+=pixel(x,y);
 assert(count==3);
 a.clear(); a.drawCircle(2,2,2,WHITE);
 const int points[][2]={{2,0},{2,4},{0,2},{4,2},{1,0},{3,0},{1,4},{3,4},{0,1},{4,1},{0,3},{4,3}};
 for(auto &p:points) assert(pixel(p[0],p[1]));
 count=0; for(int y=0;y<64;y++) for(int x=0;x<128;x++) count+=pixel(x,y); assert(count==12);
 a.clear(); a.drawLine(-2,-2,3,3,WHITE); for(int i=0;i<4;i++) assert(pixel(i,i));
 a.clear(); a.drawLine(-32768, 20, 32767, 20, WHITE); for (int x=0; x<128; ++x) assert(pixel(x,20));
 a.clear(); a.drawRect(126,62,4,4,WHITE); assert(pixel(126,62)&&pixel(127,62)&&pixel(126,63)&&!pixel(127,63));
 // uint8_t scores must print decimal digits, not CP437 glyphs.
 a.clear(); assert(a.print(static_cast<uint8_t>(255))==3); uint8_t expected[1024]; memcpy(expected,framebuffer,1024);
 a.clear(); a.print("255"); assert(!memcmp(expected,framebuffer,1024));
 a.clear(); a.print("A"); const uint8_t glyph[]={0x7c,0x12,0x11,0x12,0x7c};
 assert(!memcmp(framebuffer,glyph,5));
 puts("PASS: XY layout/clipping, circle, line, rectangle, numeric text, font");
}
