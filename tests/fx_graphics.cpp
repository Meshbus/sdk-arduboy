/* SPDX-License-Identifier: Apache-2.0 */
#define MESHBUS_ARDUBOY_FX 1
#include <ArduboyFX.h>
#include <cassert>
#include <vector>
static std::vector<uint8_t> packed;
static uint8_t framebuffer[1024];
static bool exited;
extern "C" uint8_t *meshbus_arduboy_framebuffer() { return framebuffer; }
namespace meshbus::arduboy {
void request_exit() { exited = true; }
int resource_read(uint32_t offset, void *out, size_t count) {
 if(offset>packed.size() || count>packed.size()-offset) return -ERANGE;
 memcpy(out,packed.data()+offset,count);return 0;
}
uint32_t resource_length() { return packed.size(); }
}
static void load(const std::vector<uint8_t>& raw) {
 using namespace meshbus::arduboy::save_format;
 assert(raw.size()<=128);
 packed.assign(28,0);memcpy(packed.data(),"FXPK",4);
 write16(packed.data()+4,1);write16(packed.data()+6,256);
 write32(packed.data()+8,raw.size());write16(packed.data()+12,0xfebe);
 write32(packed.data()+16,1);write32(packed.data()+20,28);
 write32(packed.data()+24,29+raw.size());packed.push_back(raw.size()-1);
 packed.insert(packed.end(),raw.begin(),raw.end());exited=false;FX::begin(0xfebe);
 assert(FX::lastError()==0);
}
int main() {
 // Two frames, 3 x 10 pixels, image/mask bytes interleaved in vertical pages.
 std::vector<uint8_t> raw{0,3,0,10};
 for(unsigned i=0;i<24;++i) raw.push_back(uint8_t(i*37+19));
 for(int frame=0;frame<2;++frame) for(int x: {-3,-1,0,126,128}) for(int y: {-10,-3,7,62,64}) {
  load(raw);memset(framebuffer,0xa5,sizeof(framebuffer));uint8_t expected[1024];memcpy(expected,framebuffer,1024);
  for(int sy=0;sy<10;++sy) for(int sx=0;sx<3;++sx) {
   int dx=x+sx,dy=y+sy;unsigned at=4+frame*12+((sy/8)*3+sx)*2;
   if(dx<0||dx>=128||dy<0||dy>=64||!(raw[at+1]&(1<<(sy%8))))continue;
   unsigned bit=1<<(dy%8),index=dx+(dy/8)*128;
   if(raw[at]&(1<<(sy%8)))expected[index]|=bit;else expected[index]&=~bit;
  }
  FX::drawBitmap(x,y,0,frame,dbmMasked);
  assert(!exited && !memcmp(expected,framebuffer,1024));
 }
 load({0,2,0,8,0x55,0xaa});memset(framebuffer,0xff,1024);
 FX::drawBitmap(1,0,0,0,dbmNormal);assert(framebuffer[0]==255&&framebuffer[1]==0x55&&framebuffer[2]==0xaa);
 uint8_t out[2];FX::readDataArray(2,1,0,2,out,2);assert(out[0]==0x55&&out[1]==0xaa);
 FX::drawBitmap(0,0,0,1,dbmNormal);assert(exited&&FX::lastError()==-ERANGE);
 load({0,2,0,8,0x55,0xaa});FX::drawBitmap(0,0,0,0,99);assert(exited&&FX::lastError()==-ENOTSUP);
 load({0,2,0,8,0x55,0xaa});FX::readDataArray(0xffffff,1,0,2,out,2);assert(exited&&FX::lastError()==-ERANGE);
}
