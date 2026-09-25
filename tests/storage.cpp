#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <meshbus_arduboy/eeprom.hpp>
#define MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
#include <meshbus_arduboy/llext_game.hpp>
using meshbus::arduboy::EepromFile;
std::map<std::string,std::vector<uint8_t>> files;
int failure=0, writes=0, attempts=0;
int open_files=0;
EepromFile *interleaved = nullptr;
int fs_open(fs_file_t *f,const char *p,int flags) {
 if((flags&FS_O_READ)&&!files.count(p))return -ENOENT;
 ++open_files;f->path=p;f->offset=0;if(flags&FS_O_TRUNC)files[p].clear();return 0;
}
int fs_close(fs_file_t*) {--open_files;return failure==2?-EIO:0;}
int fs_seek(fs_file_t *f,off_t offset,int origin) {
 if(origin!=FS_SEEK_SET||offset<0)return -EINVAL;f->offset=offset;return 0;
}
int fs_sync(fs_file_t*) {return failure==3?-EIO:0;}
int fs_mkdir(const char*) {return 0;}
int fs_unlink(const char *p) {return files.erase(p)?0:-ENOENT;}
int fs_rename(const char *a,const char *b) {if(failure==4)return -EIO;files[b]=files[a];files.erase(a);return 0;}
ssize_t fs_read(fs_file_t *f,void *dst,size_t n) {
 if(failure==7)return -EIO;
 auto &v=files[f->path];n=(std::min)(n,v.size()-f->offset);memcpy(dst,v.data()+f->offset,n);f->offset+=n;return n;
}
ssize_t fs_write(fs_file_t *f,const void *src,size_t n) {
 ++attempts;
 if(failure==5)return -ENOSPC;
 if(failure==1&&writes++)return 0;
 if(failure==1||failure==6)n=(std::min)(n,size_t(2));
 if (interleaved) { auto *e = interleaved; interleaved = nullptr; e->write(0, 0x99); }
 auto &v=files[f->path];auto *p=static_cast<const uint8_t*>(src);v.insert(v.end(),p,p+n);return n;
}
#ifndef MESHBUS_ARDUBOY_TEST_NO_MAIN
int main() {
 for(int mode=1;mode<=5;mode++) {
  files.clear();files["/extra/saves/game.dat"]={1,2,3,4};
  EepromFile e;uint8_t ram[4];e.init("game",ram,4);assert(e.load()==0);e.write(0,9);
  failure=mode;writes=0;assert(e.flush()<0&&e.dirty());failure=0;
  EepromFile previous;uint8_t saved[4];previous.init("game",saved,4);assert(previous.load()==0);
  assert(saved[0]==1&&saved[1]==2&&saved[2]==3&&saved[3]==4);
  assert(e.flush()==0&&!e.dirty());assert(previous.load()==0&&saved[0]==9);
 }
 files["/extra/saves/game.dat.tmp"]={0xff};
 EepromFile e;uint8_t ram[4];e.init("game",ram,4);e.write(0,7);failure=6;
 assert(e.flush()==0);failure=0;assert(e.load()==0&&ram[0]==7);
 assert(!files.count("/extra/saves/game.dat.tmp"));
 e.write(0, 0x77); interleaved = &e;
 assert(e.flush() == 0);
 assert(files[e.path()][0] == 0x77 && e.read(0) == 0x99 && e.dirty());
 assert(e.flush() == 0 && files[e.path()][0] == 0x99 && !e.dirty());
 EepromFile concurrent; uint8_t mirror[64]; concurrent.init("concurrent", mirror, sizeof(mirror));
 std::atomic<bool> done{false};
 std::thread writer([&] {
     uint8_t block[64];
     for (int i = 0; i < 2000; ++i) {
         memset(block, (i & 1) ? 0x55 : 0xaa, sizeof(block));
         concurrent.update_block(0, block, sizeof(block));
     }
     memset(block, 0x99, sizeof(block)); concurrent.write_block(0, block, sizeof(block));
     done = true;
 });
 while (!done) {
     assert(concurrent.flush() == 0);
     auto it = files.find(concurrent.path());
     if (it != files.end()) for (auto byte : it->second) assert(byte == it->second[0]);
 }
 writer.join(); assert(concurrent.flush() == 0 && !concurrent.dirty());
 for (auto byte : files[concurrent.path()]) assert(byte == 0x99);
 struct App { meshbus::arduboy::llext_game::FullscreenState<App> runtime; } app{};
 meshbus::arduboy::llext_game::FullscreenConfig<App> cfg{}; cfg.log_tag = "test";
 uint8_t retry_ram[4]; app.runtime.eeprom.init("retry", retry_ram, sizeof(retry_ram));
 app.runtime.eeprom_enabled = true; failure = 5; attempts = 0;
 for (int i=0; i<30; ++i) {
     app.runtime.eeprom.write(0, static_cast<uint8_t>(i));
     meshbus::arduboy::llext_game::flush_eeprom(&app, &cfg);
 }
 assert(attempts < 10 && app.runtime.eeprom.dirty());
 failure = 0;
 meshbus::arduboy::llext_game::flush_eeprom(&app, &cfg, true);
 assert(!app.runtime.eeprom.dirty() && files[app.runtime.eeprom.path()][0] == 29);

}

#endif
