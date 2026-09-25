#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
static void setup_snapshot()
{
    auto *drawing = meshbus_arduboy_framebuffer();
    memset(drawing, 0x55, 1024); meshbus_arduboy_display(true);
    for (unsigned i=0; i<1024; ++i) assert(drawing[i] == 0);
    memset(drawing, 0xaa, 1024);
    zui_draw_ctx ctx; meshbus::arduboy::draw(&ctx, meshbus::arduboy::active);
    for (auto byte : meshbus::arduboy::visible) assert(byte == 0x55);
    std::atomic<bool> finished{false};
    std::thread ui([&] {
        zui_draw_ctx ui_ctx;
        while (!finished) {
            meshbus::arduboy::draw(&ui_ctx, meshbus::arduboy::active);
            auto value = meshbus::arduboy::visible[0];
            for (auto byte : meshbus::arduboy::visible) assert(byte == value);
        }
    });
    for (int i=0; i<2000; ++i) {
        memset(drawing, (i&1) ? 0xaa : 0x55, 1024);
        meshbus_arduboy_display(true);
        for (unsigned n=0; n<1024; ++n) assert(drawing[n] == 0);
    }
    finished = true; ui.join();
    memset(drawing, 0x33, 1024); meshbus_arduboy_display(true);
    memset(drawing, 0xcc, 1024);
    for (int i=0; i<3; ++i) {
        meshbus::arduboy::draw(&ctx, meshbus::arduboy::active);
        for (auto byte : meshbus::arduboy::visible) assert(byte == 0x33);
    }
}
static void loop_snapshot() {}
int main()
{
    zui_host host{}; mbs_desktop_app_args args{&host};
    meshbus::arduboy::SketchConfig config{"snapshot", setup_snapshot, loop_snapshot};
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
    assert(!host.attached && fixture::screens == 0 && fixture::routers == 0);
}
