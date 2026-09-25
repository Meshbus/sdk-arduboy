#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
static int setup_count, loop_count;
static std::thread::id owner;
static zui_host host;
static void sketch_setup() {
    ++setup_count; assert(std::this_thread::get_id() == owner);
    assert(host.attached != nullptr);
    auto *buffer = meshbus_arduboy_framebuffer(); assert(buffer != nullptr);
    memset(buffer, 0x55, 1024); meshbus_arduboy_display(false);
    memset(buffer, 0x33, 1024);
    assert(meshbus::arduboy::submitted[0] == 0x55);
    meshbus_arduboy_random_seed(1); assert(meshbus_arduboy_random(1000) == 369);
}
static void sketch_loop() {
    ++loop_count; assert(std::this_thread::get_id() == owner);
    std::thread ui([] {
        zui_draw_ctx ctx;
        meshbus::arduboy::draw(&ctx, meshbus::arduboy::active);
        assert(meshbus::arduboy::visible[0] == 0x55);
    });
    ui.join();
}
int main() {
    owner = std::this_thread::get_id();
    assert(meshbus_arduboy_framebuffer() == nullptr);
    mbs_desktop_app_args args{&host};
    meshbus::arduboy::SketchConfig config{"native", sketch_setup, sketch_loop};
    fixture::fail = 3;
    assert(meshbus::arduboy::run_sketch(&args, config) == -ENOMEM);
    assert(setup_count == 0 && loop_count == 0 && !host.attached);
    fixture::fail = 0;
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
    assert(setup_count == 1 && loop_count == 1 && !host.attached);
    assert(meshbus_arduboy_framebuffer() == nullptr);
    assert(fixture::routers == 0 && fixture::screens == 0);
}
