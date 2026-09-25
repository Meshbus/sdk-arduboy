/* SPDX-License-Identifier: Apache-2.0 */
#define MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
#include <meshbus_arduboy/runtime.hpp>
#include <meshbus_arduboy/clock.hpp>
#include <meshbus_arduboy/llext_game.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <indicator/indicator.h>
#ifdef MESHBUS_ARDUBOY_RESOURCES
#include <meshbus_arduboy/resources.hpp>
#endif


extern "C" bool meshbus_arduboy_runtime_ready();
#include <meshbus_arduboy/legacy_score.hpp>
#include "runtime_audio.hpp"

namespace meshbus::arduboy {
namespace {
struct AppContext {
    llext_game::FullscreenState<AppContext> runtime;
    const SketchConfig *sketch;
    k_sem bridge_lock;
    InputLatch input;
    FrameClock clock;
    bool stack_reported;
    unsigned stopping;
    uint8_t buttons;
    uint32_t random_state;
#ifdef MESHBUS_ARDUBOY_RESOURCES
    ResourceBundle resources;
#endif
};
// Per-extension storage is charged to the loader, not the shared application stack.
uint8_t drawing[1024], submitted[1024], visible[1024], eeprom_bytes[1024];
AppContext *active;
bool unbound_reported;

AppContext *context()
{
    if (active == nullptr && !unbound_reported) {
        printk("[arduboy] runtime unavailable before run_sketch or after exit\n");
        unbound_reported = true;
    }
    return active;
}
struct Guard {
    k_sem &lock;
    explicit Guard(k_sem &sem) : lock(sem) { k_sem_take(&lock, K_FOREVER); }
    ~Guard() { k_sem_give(&lock); }
};
uint8_t *framebuffer(AppContext *) { return drawing; }
uint8_t *eeprom_data(AppContext *) { return eeprom_bytes; }
void unused_tick(AppContext *) {}
void setup(AppContext *app) { meshbus_arduboy_audio_begin(); app->sketch->setup(); }
void draw(zui_draw_ctx *draw_ctx, void *data)
{
    auto *app = static_cast<AppContext *>(data);
    {
        Guard guard(app->bridge_lock);
        memcpy(visible, submitted, sizeof(visible));
    }
    const zui_framebuffer_view view = {
        .struct_size = sizeof(view), .data = visible,
        .width = 128, .height = 64, .stride = 128,
        .format = ZUI_BITMAP_FORMAT_MONO_VLSB,
    };
    zui_draw_framebuffer(draw_ctx, zui_point{0, 0}, &view);
}
bool input(const zui_input_event *event, void *data)
{
    auto *app = static_cast<AppContext *>(data);
    bool stop = false, accepted = true;
    {
        Guard guard(app->bridge_lock);
        zui_action_state state{};
        state.down = app->input.held();
        uint32_t previous = state.down;
        if (zui_action_state_update(&state, event) != 0) { return false; }
        if (event->action == ZUI_INPUT_ACTION_PRESS) {
            accepted = app->input.press(state.pressed);
        } else if (event->action == ZUI_INPUT_ACTION_RELEASE) {
            app->input.release(previous & ~state.down);
        } else if (event->action == ZUI_INPUT_ACTION_CLICK) {
            accepted = app->input.click(state.pressed);
        }
        stop = (state.long_pressed & app->sketch->exit_action_mask) != 0;
        if (stop) { app->input.reset(); }
    }
    if (!accepted) { printk("[arduboy] input queue saturated\n"); }
    if (stop) {
        __atomic_store_n(&app->stopping, 1U, __ATOMIC_RELEASE);
        k_sem_give(&app->runtime.exit_sem);
    }
    return true;
}
void reset_input(void *data)
{
    auto *app = static_cast<AppContext *>(data);
    Guard guard(app->bridge_lock);
    app->input.reset();
}
void report_stack();
uint32_t wait_ticks(uint32_t milliseconds)
{
    // Callers bound waits to <=1000ms; supported tick rates divide 1 MHz.
    return (milliseconds * CONFIG_SYS_CLOCK_TICKS_PER_SEC + 999U) / 1000U;
}
void consume_input(AppContext *app)
{
    ++tone_frame;
    uint32_t actions;
    {
        Guard guard(app->bridge_lock);
        actions = app->input.consume();
    }
    app->buttons = app->sketch->hooks && app->sketch->hooks->buttons
                       ? app->sketch->hooks->buttons(actions) : buttons_from_zui(actions);
}
void execute(AppContext *app)
{
    uint32_t save_at = ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC);
    while (!exit_requested()) {
        if (!app->sketch->frame_gated) { consume_input(app); }
        app->sketch->loop();
        if (exit_requested()) { break; }
        if (tone_frame >= 120U && !app->stack_reported) { report_stack(); app->stack_reported = true; }
        uint32_t now = ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC);
        if (now - save_at >= 250U) {
            llext_game::flush_eeprom(app, llext_game::config(app));
            save_at = now;
        }
        uint32_t milliseconds = app->sketch->frame_gated
                                    ? app->clock.wait_ms(meshbus_arduboy_millis())
                                    : 1000U / app->sketch->fps;
        // An over-budget frame still gives lower-priority management work a
        // useful scheduling window; one 32-us clock tick is insufficient.
        uint32_t ticks = wait_ticks(milliseconds ? milliseconds : 1U);
        if (k_sem_take(&app->runtime.exit_sem, K_TICKS(ticks)) == 0) {
            __atomic_store_n(&app->stopping, 1U, __ATOMIC_RELEASE);
        }
    }
}
void report_stack()
{
#if defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_STACK_INFO)
    size_t unused = 0;
    if (k_thread_stack_space_get(k_sched_current_thread_query(), &unused) == 0) {
        printk("[arduboy] application stack unused: %u bytes\n", static_cast<unsigned>(unused));
    }
#endif
}
void teardown(AppContext *) { meshbus_arduboy_tone_stop(); report_stack(); }
} // namespace

bool exit_requested()
{
#ifdef MESHBUS_ARDUBOY_MANAGED_STOP
    if (mbs_desktop_app_stop_requested()) { return true; }
#endif
    return active == nullptr || __atomic_load_n(&active->stopping, __ATOMIC_ACQUIRE) != 0;
}
void request_exit()
{
    if (auto *app = context()) {
        __atomic_store_n(&app->stopping, 1U, __ATOMIC_RELEASE);
        k_sem_give(&app->runtime.exit_sem);
    }
}

int run_sketch(void *args, const SketchConfig &sketch)
{
    if (active != nullptr) { return -EBUSY; }
    if (!save_id_valid(sketch.id) || !sketch.setup || !sketch.loop || sketch.fps == 0) {
        return -EINVAL;
    }
    AppContext app{};
#ifdef MESHBUS_ARDUBOY_RESOURCES
    if (sketch.resources) {
        int rc = app.resources.open(*sketch.resources);
        if (rc != 0) {
            printk("[arduboy] resource open failed: %d\n", rc);
            return rc;
        }
    }
#else
    if (sketch.resources) { return -ENOTSUP; }
#endif
    app.sketch = &sketch;
    app.random_state = 1;
    app.clock.set_rate(sketch.fps);
    k_sem_init(&app.bridge_lock, 1, 1);
    memset(drawing, 0, sizeof(drawing));
    memset(submitted, 0, sizeof(submitted));
    memset(visible, 0, sizeof(visible));
    tone_frame = 0;
    legacy_scores = nullptr; legacy_score_count = 0;
    tone_token = 0; audio_status = 0; audio_enabled = true; audio_silent = sketch.silent_audio;
    const zui_screen_ops ops = {
        .draw = draw, .input = input, .enter = reset_input, .exit = reset_input,
        .event = nullptr,
    };
    const llext_game::FullscreenConfig<AppContext> config = {
        .log_tag = sketch.id, .screen_id = 1, .fps = sketch.fps,
        .width = 128, .height = 64, .stride = 128, .format = ZUI_BITMAP_FORMAT_MONO_VLSB,
        .exit_action_mask = sketch.exit_action_mask, .framebuffer = framebuffer,
        .eeprom = {sketch.save_id ? sketch.save_id : sketch.id, eeprom_data,
                   sizeof(eeprom_bytes), 0xff, sketch.save_schema, sketch.migrate},
        .setup = setup, .tick = unused_tick, .idle = nullptr, .teardown = teardown,
        .execute = execute,
    };
    active = &app;
    int rc = llext_game::run(args, &app, &config, &ops);
#ifdef MESHBUS_ARDUBOY_RESOURCES
    int close_rc = app.resources.close();
    if (close_rc != 0) {
        printk("[arduboy] resource close failed: %d\n", close_rc);
        if (rc == 0) { rc = close_rc; }
    }
#endif
    active = nullptr;
    return rc;
}
#ifdef MESHBUS_ARDUBOY_RESOURCES
int resource_read(uint32_t offset, void *destination, size_t size)
{
    auto *app = context();
    return app ? app->resources.read(offset, destination, size) : -ENODEV;
}
uint32_t resource_length()
{
    auto *app = context();
    return app ? app->resources.length() : 0;
}
#endif
} // namespace meshbus::arduboy

using namespace meshbus::arduboy;
extern "C" void meshbus_arduboy_set_frame_count(uint16_t count)
{
    if (auto *app = context()) { app->clock.set_count(count); }
}
extern "C" bool meshbus_arduboy_runtime_ready() { return context() != nullptr; }
extern "C" uint8_t *meshbus_arduboy_framebuffer() { return context() ? drawing : nullptr; }
extern "C" uint8_t meshbus_arduboy_buttons() { auto *app = context(); return app ? app->buttons : 0; }
extern "C" uint32_t meshbus_arduboy_millis()
{
    auto *app = context();
    if (!app) { return 0; }
    return app->sketch->hooks && app->sketch->hooks->millis
               ? app->sketch->hooks->millis() : ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC);
}
extern "C" uint32_t meshbus_arduboy_micros()
{
    auto *app = context();
    if (!app) { return 0; }
    if (app->sketch->hooks && app->sketch->hooks->micros) { return app->sketch->hooks->micros(); }
    static_assert(1000000U % CONFIG_SYS_CLOCK_TICKS_PER_SEC == 0);
    return static_cast<uint32_t>(k_uptime_ticks()) * (1000000U / CONFIG_SYS_CLOCK_TICKS_PER_SEC);
}
extern "C" void meshbus_arduboy_random_seed(uint32_t seed)
{
    if (auto *app = context()) { app->random_state = seed ? seed : 1; }
}
extern "C" long meshbus_arduboy_random(long maximum)
{
    auto *app = context();
    if (!app || maximum <= 0) { return 0; }
    auto &state = app->random_state;
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state % static_cast<uint32_t>(maximum);
}
extern "C" long meshbus_arduboy_random_range(long low, long high)
{
    if (high <= low) { return low; }
    unsigned long range = static_cast<unsigned long>(high) - static_cast<unsigned long>(low);
    auto *app = context();
    if (!app) { return low; }
    auto &state = app->random_state;
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return static_cast<long>(static_cast<unsigned long>(low) + state % range);
}
extern "C" int meshbus_arduboy_rand() { return static_cast<int>(meshbus_arduboy_random(INT32_MAX)); }

extern "C" void meshbus_arduboy_score_stop() { meshbus_arduboy_tone_stop(); }
extern "C" void meshbus_arduboy_display(bool clear_after)
{
    auto *app = context();
    if (!app) { return; }
    {
        Guard guard(app->bridge_lock);
        memcpy(submitted, drawing, sizeof(submitted));
    }
    if (clear_after) { memset(drawing, 0, sizeof(drawing)); }
    zui_screen_request_redraw(app->runtime.screen);
}

extern "C" void meshbus_arduboy_set_frame_rate(uint8_t fps)
{
    if (auto *app = context()) {
        if (!app->clock.set_rate(fps)) { printk("[arduboy] frame rate must be nonzero\n"); }
    }
}
extern "C" bool meshbus_arduboy_next_frame()
{
    auto *app = context();
    if (!app || exit_requested()) { return false; }
    if (!app->clock.next(meshbus_arduboy_millis())) { return false; }
    consume_input(app);
    return true;
}
extern "C" uint16_t meshbus_arduboy_frame_count()
{
    auto *app = context(); return app ? app->clock.count() : 0;
}
extern "C" uint8_t meshbus_arduboy_cpu_load()
{
    auto *app = context(); return app ? app->clock.load() : 0;
}
extern "C" void meshbus_arduboy_delay(uint32_t milliseconds)
{
    auto *app = context();
    if (!app) { return; }
#ifdef MESHBUS_ARDUBOY_TRACE_TIMING
    struct Trace {
        uint32_t requested, started;
        ~Trace() {
            uint32_t actual = ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC) - started;
            printk("[arduboy] delay requested_ms=%u actual_ms=%u cancelled=%u\n",
                   requested, actual, exit_requested() ? 1U : 0U);
        }
    } trace{milliseconds, ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC)};
#endif
    while (milliseconds && !exit_requested()) {
        uint32_t before = ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC);
        uint32_t chunk = milliseconds < 1000U ? milliseconds : 1000U;
        if (k_sem_take(&app->runtime.exit_sem, K_TICKS(wait_ticks(chunk))) == 0) {
            __atomic_store_n(&app->stopping, 1U, __ATOMIC_RELEASE);
            return;
        }
        uint32_t elapsed = ticks_to_millis(k_uptime_ticks(), CONFIG_SYS_CLOCK_TICKS_PER_SEC) - before;
        if (elapsed >= milliseconds) { return; }
        milliseconds -= elapsed;
    }
}
