/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_RUNTIME_HPP_
#define MESHBUS_ARDUBOY_RUNTIME_HPP_
#include <stddef.h>
#include <stdint.h>
#include <meshbus_arduboy/save_format.hpp>
namespace meshbus::arduboy {
struct ResourceSpec;
struct RuntimeHooks {
    uint32_t (*millis)() = nullptr;
    uint32_t (*micros)() = nullptr;
    uint8_t (*buttons)(uint32_t actions) = nullptr;
};
struct SketchConfig {
    const char *id;
    void (*setup)();
    void (*loop)();
    uint8_t fps = 60;
    uint32_t exit_action_mask = 1U << 7; // ZUI_ACTION_CANCEL (long Back).
    const RuntimeHooks *hooks = nullptr;
    bool frame_gated = true;
    const char *save_id = nullptr;
    uint32_t save_schema = 1;
    SaveMigration migrate = nullptr;
    bool silent_audio = false;
    const ResourceSpec *resources = nullptr;
};
int run_sketch(void *desktop_args, const SketchConfig &config);
bool exit_requested();
void request_exit();
}
extern "C" void meshbus_arduboy_display(bool clear_after);
extern "C" void meshbus_arduboy_delay(uint32_t milliseconds);
extern "C" void meshbus_arduboy_set_frame_rate(uint8_t fps);
extern "C" bool meshbus_arduboy_next_frame();
extern "C" uint16_t meshbus_arduboy_frame_count();
extern "C" void meshbus_arduboy_set_frame_count(uint16_t count);
extern "C" uint8_t meshbus_arduboy_cpu_load();
#endif
