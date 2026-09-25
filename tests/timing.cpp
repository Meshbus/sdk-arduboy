#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
static Arduboy2 board;
static std::vector<uint32_t> frame_times;
static int mode;
static uint32_t delay_elapsed;
static void setup_timing()
{
    uint32_t before = millis();
    if (mode == 1) { kernel_fixture::stop_at = kernel_fixture::ticks + 3125; }
    board.display();
    delay(mode == 1 ? 60000 : 3000);
    delay_elapsed = millis() - before;
    board.setFrameRate(15);
}
static void loop_timing()
{
    if (!board.nextFrame()) { return; }
    frame_times.push_back(millis());
    assert(board.everyXFrames(3) == (frame_times.size() % 3 == 0));
    if (frame_times.size() == 4) { board.setFrameRate(30); }
    if (frame_times.size() == 8) { board.setFrameRate(60); }
    if (frame_times.size() == 12) {
        __atomic_store_n(&meshbus::arduboy::active->stopping, 1U, __ATOMIC_RELEASE);
    }
}
int main()
{
    kernel_fixture::finite_exits = false;
    delay(123); assert(kernel_fixture::waits == 0);
    zui_host host{}; mbs_desktop_app_args args{&host};
    meshbus::arduboy::SketchConfig config{"timing", setup_timing, loop_timing};
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
    assert(delay_elapsed == 3000 && frame_times.size() == 12);
    for (size_t i=1; i<frame_times.size(); ++i) {
        uint32_t expected = i < 4 ? 66 : (i < 8 ? 33 : 16);
        uint32_t actual = frame_times[i] - frame_times[i-1];
        assert(actual >= expected && actual <= expected + 1);
    }
    assert(kernel_fixture::waits < 50); // The executor waits, rather than polling in a busy loop.
    mode = 1; frame_times.clear();
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
    assert(delay_elapsed == 100 && frame_times.empty() && !host.attached);
    assert(fixture::routers == 0 && fixture::screens == 0);
}
