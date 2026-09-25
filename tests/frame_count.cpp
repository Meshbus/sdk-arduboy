#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
static Arduboy2 board;
static int frames;
static uint64_t returned;
static void start() { board.frameCount = 65534; }
static void step() {
    if (returned) { assert(kernel_fixture::ticks - returned >= 31); }
    if (!board.nextFrame()) { returned=kernel_fixture::ticks; return; }
    ++frames;
    if(frames==1) assert(board.frameCount==65535);
    if(frames==2) { assert(board.frameCount==0); board.frameCount=20; }
    if(frames==3) { assert(board.frameCount==21); assert(board.everyXFrames(3)); meshbus::arduboy::request_exit(); }
    kernel_fixture::ticks+=3125; // 100 ms rendering, over the 16 ms frame budget.
    returned=kernel_fixture::ticks;
}
int main() {
    kernel_fixture::finite_exits=false;
    zui_host host{}; mbs_desktop_app_args args{&host};
    meshbus::arduboy::SketchConfig config{"frame_count",start,step};
    assert(meshbus::arduboy::run_sketch(&args,config)==0);
    assert(frames==3 && !host.attached);
}
