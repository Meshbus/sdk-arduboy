/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_LLEXT_GAME_HPP_
#define MESHBUS_ARDUBOY_LLEXT_GAME_HPP_

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <desktop/desktop.h>
#include <meshbus_arduboy/input.hpp>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zui/zui.h>

#ifdef MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
#include <meshbus_arduboy/eeprom_runtime.hpp>
#endif

namespace meshbus::arduboy::llext_game {

#ifdef MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
template <typename App> struct EepromConfig {
	const char *save_id;
	uint8_t *(*data)(App *app);
	size_t size;
	uint8_t erased_value = 0xffU;
    uint32_t schema = 0;
    SaveMigration migration = nullptr;
};
#endif

template <typename App> struct FullscreenState {
	zui_host *host;
	zui_router *router;
	zui_screen *screen;
	zui_action_state actions;
    InputLatch input_latch;
    zui_screen_ops owned_ops;
    const zui_screen_ops *original_ops;
	k_sem exit_sem;
	const void *config;
	bool router_registered;
	bool router_attached;
#ifdef MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
	EepromFile eeprom;
	bool eeprom_enabled;
	int eeprom_last_flush_rc;
    uint8_t eeprom_retry_skip;
    uint8_t eeprom_retry_delay;
#endif
};

template <typename App> struct FullscreenConfig {
	const char *log_tag;
	uint32_t screen_id;
	uint16_t fps;
	uint16_t width;
	uint16_t height;
	uint16_t stride;
	enum zui_bitmap_format format;
	uint32_t exit_action_mask;
	uint8_t *(*framebuffer)(App *app);
#ifdef MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
	EepromConfig<App> eeprom;
#endif
	void (*setup)(App *app);
	void (*tick)(App *app);
	void (*idle)(App *app);
	void (*teardown)(App *app);
    // Optional application-thread executor; setup then runs after attach.
    void (*execute)(App *app) = nullptr;
};

template <typename App>
const FullscreenConfig<App> *config(App *app)
{
	return static_cast<const FullscreenConfig<App> *>(app->runtime.config);
}

#ifdef MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM
template <typename App>
bool eeprom_configured(const FullscreenConfig<App> *cfg)
{
	return cfg->eeprom.save_id != nullptr && cfg->eeprom.data != nullptr &&
	       cfg->eeprom.size != 0U;
}

template <typename App>
int setup_eeprom(App *app, const FullscreenConfig<App> *cfg)
{
	uint8_t *data;
	int rc;

	app->runtime.eeprom_enabled = false;
	app->runtime.eeprom_last_flush_rc = 0;
	if (!eeprom_configured(cfg)) {
		return 0;
	}

	data = cfg->eeprom.data(app);
	if (data == nullptr) {
		printk("[%s] EEPROM data buffer is null\n", cfg->log_tag);
		return -EINVAL;
	}

	rc = cfg->eeprom.schema ? app->runtime.eeprom.init_versioned(
        cfg->eeprom.save_id, data, cfg->eeprom.size, cfg->eeprom.schema,
        cfg->eeprom.migration, cfg->eeprom.erased_value) : app->runtime.eeprom.init(cfg->eeprom.save_id, data, cfg->eeprom.size,
				      cfg->eeprom.erased_value);
	if (rc != 0) {
		printk("[%s] EEPROM init failed: %d\n", cfg->log_tag, rc);
		return rc;
	}

	rc = app->runtime.eeprom.load();
	if (rc != 0) {
		printk("[%s] EEPROM load failed: %d\n", cfg->log_tag, rc);
        return rc;
	}

	eeprom_attach(&app->runtime.eeprom);
	app->runtime.eeprom_enabled = true;
    return 0;
}

template <typename App>
void flush_eeprom(App *app, const FullscreenConfig<App> *cfg, bool force = false)
{
	int rc;

	if (!app->runtime.eeprom_enabled) {
		return;
	}

    if (!force && app->runtime.eeprom_retry_skip != 0) {
        --app->runtime.eeprom_retry_skip;
        return;
    }
	rc = app->runtime.eeprom.flush();
    if (rc != 0) {
        auto &delay = app->runtime.eeprom_retry_delay;
        delay = delay == 0 ? 1 : (delay < 8 ? delay * 2 : 8);
        app->runtime.eeprom_retry_skip = delay;
    } else {
        app->runtime.eeprom_retry_delay = 0;
        app->runtime.eeprom_retry_skip = 0;
    }
	if (rc != 0 && rc != app->runtime.eeprom_last_flush_rc) {
		printk("[%s] EEPROM flush failed: %d\n", cfg->log_tag, rc);
	}
	app->runtime.eeprom_last_flush_rc = rc;
}

template <typename App>
void teardown_eeprom(App *app, const FullscreenConfig<App> *cfg)
{
	if (!app->runtime.eeprom_enabled) {
		return;
	}

	flush_eeprom(app, cfg, true);
	eeprom_detach(&app->runtime.eeprom);
	app->runtime.eeprom_enabled = false;
}
#else
template <typename App>
int setup_eeprom(App *app, const FullscreenConfig<App> *cfg)
{
	ARG_UNUSED(app);
	ARG_UNUSED(cfg);
    return 0;
}

template <typename App>
void flush_eeprom(App *app, const FullscreenConfig<App> *cfg)
{
	ARG_UNUSED(app);
	ARG_UNUSED(cfg);
}

template <typename App>
void teardown_eeprom(App *app, const FullscreenConfig<App> *cfg)
{
	ARG_UNUSED(app);
	ARG_UNUSED(cfg);
}
#endif

template <typename App>
void draw(zui_draw_ctx *draw_ctx, void *user_data)
{
	App *app = static_cast<App *>(user_data);
	const FullscreenConfig<App> *cfg = config(app);
	uint8_t *fb = cfg->framebuffer(app);

	if (fb == nullptr) {
		return;
	}

	const zui_framebuffer_view view = {
		.struct_size = sizeof(view),
		.data = fb,
		.width = cfg->width,
		.height = cfg->height,
		.stride = cfg->stride,
		.format = cfg->format,
	};

	zui_draw_framebuffer(draw_ctx, zui_point{0, 0}, &view);
}

template <typename App>
bool input(const zui_input_event *event, void *user_data)
{
	App *app = static_cast<App *>(user_data);
	const FullscreenConfig<App> *cfg = config(app);
    zui_action_state normalized{};
    normalized.down = app->runtime.input_latch.held();
    uint32_t previous = normalized.down;
    int rc = zui_action_state_update(&normalized, event);
    if (rc != 0) { return false; }
    bool accepted = true;
    if (event->action == ZUI_INPUT_ACTION_PRESS) {
        accepted = app->runtime.input_latch.press(normalized.pressed);
    } else if (event->action == ZUI_INPUT_ACTION_RELEASE) {
        app->runtime.input_latch.release(previous & ~normalized.down);
    } else if (event->action == ZUI_INPUT_ACTION_CLICK) {
        accepted = app->runtime.input_latch.click(normalized.pressed);
    }
    if (!accepted) { printk("[%s] input queue saturated\n", cfg->log_tag); }
    app->runtime.actions.long_pressed |= normalized.long_pressed;
    if ((normalized.long_pressed & cfg->exit_action_mask) != 0U) {
        app->runtime.input_latch.reset();
        app->runtime.actions = {};
        k_sem_give(&app->runtime.exit_sem);
    }

	(void)zui_screen_request_redraw(app->runtime.screen);
	return true;
}

template <typename App>
bool event(const zui_screen_event *event, void *user_data)
{
	App *app = static_cast<App *>(user_data);
	const FullscreenConfig<App> *cfg = config(app);

	if (event->type != ZUI_SCREEN_EVENT_TICK) {
		return false;
	}

	for (uint32_t i = 0U; i < event->code; i++) {
        uint32_t previous = app->runtime.actions.down;
        app->runtime.actions.down = app->runtime.input_latch.consume();
        app->runtime.actions.pressed = app->runtime.actions.down & ~previous;
        app->runtime.actions.released = previous & ~app->runtime.actions.down;
		cfg->tick(app);
        zui_action_state_clear_edges(&app->runtime.actions);
	}
	zui_action_state_clear_edges(&app->runtime.actions);
	(void)zui_screen_request_redraw(app->runtime.screen);
	return true;
}

template <typename App>
void enter(void *data)
{
    auto *app = static_cast<App *>(data);
    app->runtime.input_latch.reset();
    app->runtime.actions = {};
    if (app->runtime.original_ops->enter) { app->runtime.original_ops->enter(data); }
}
template <typename App>
void leave(void *data)
{
    auto *app = static_cast<App *>(data);
    app->runtime.input_latch.reset();
    app->runtime.actions = {};
    if (app->runtime.original_ops->exit) { app->runtime.original_ops->exit(data); }
}

template <typename App>
void cleanup(App *app)
{
	if (app == nullptr || app->runtime.config == nullptr) {
		return;
	}
	const FullscreenConfig<App> *cfg = config(app);

	if (app->runtime.router_attached && app->runtime.host != nullptr) {
		(void)zui_host_detach_router(app->runtime.host, ZUI_LAYER_FULLSCREEN);
		(void)zui_host_request_redraw(app->runtime.host);
		app->runtime.router_attached = false;
	}
	if (app->runtime.router_registered && app->runtime.router != nullptr) {
		(void)zui_router_unregister_screen(app->runtime.router, cfg->screen_id);
		app->runtime.router_registered = false;
	}
	if (app->runtime.screen != nullptr) {
		zui_screen_destroy(app->runtime.screen);
		app->runtime.screen = nullptr;
	}
	if (app->runtime.router != nullptr) {
		zui_router_destroy(app->runtime.router);
		app->runtime.router = nullptr;
	}
}

template <typename App>
int run(void *args, App *app, const FullscreenConfig<App> *cfg,
	 const zui_screen_ops *screen_ops)
{
	mbs_desktop_app_args *app_args = static_cast<mbs_desktop_app_args *>(args);
	int rc = 0;
	const char *stage = "allocate";

	if (app == nullptr || cfg == nullptr || screen_ops == nullptr || cfg->framebuffer == nullptr ||
	    cfg->tick == nullptr || cfg->fps == 0U || app_args == nullptr ||
	    app_args->host == nullptr) {
		return -EINVAL;
	}

	printk("[%s] start\n", cfg->log_tag);
	app->runtime.host = app_args->host;
	app->runtime.config = cfg;
    app->runtime.input_latch.reset();
    app->runtime.actions = {};
    app->runtime.original_ops = screen_ops;
    app->runtime.owned_ops = *screen_ops;
    app->runtime.owned_ops.enter = enter<App>;
    app->runtime.owned_ops.exit = leave<App>;
	app->runtime.router_registered = false;
	app->runtime.router_attached = false;
	k_sem_init(&app->runtime.exit_sem, 0, 1);
    stage = "save load";
    rc = setup_eeprom(app, cfg);
    if (rc) { goto out; }
    stage = "allocate";

	if (cfg->setup != nullptr && cfg->execute == nullptr) {
		cfg->setup(app);
	}

	app->runtime.router = zui_router_create();
	app->runtime.screen = zui_screen_create(&app->runtime.owned_ops, app);
	if (app->runtime.router == nullptr || app->runtime.screen == nullptr) {
		rc = -ENOMEM;
		goto out;
	}
	stage = "tick period";
	rc = zui_screen_set_tick_period(app->runtime.screen, MAX(1000U / cfg->fps, 1U));
	if (rc != 0) {
		goto out;
	}
	stage = "register";
	rc = zui_router_register_screen(app->runtime.router, cfg->screen_id, app->runtime.screen);
	if (rc != 0) {
		goto out;
	}
	app->runtime.router_registered = true;
	stage = "switch";
	rc = zui_router_switch(app->runtime.router, cfg->screen_id);
	if (rc != 0) {
		goto out;
	}
	stage = "attach";
	rc = zui_host_attach_router(app->runtime.host, ZUI_LAYER_FULLSCREEN, app->runtime.router);
	if (rc != 0) {
		goto out;
	}
	app->runtime.router_attached = true;
	(void)zui_host_send_layer_to_front(app->runtime.host, ZUI_LAYER_FULLSCREEN);
	(void)zui_host_set_layer_enabled(app->runtime.host, ZUI_LAYER_FULLSCREEN, true);
	(void)zui_host_request_redraw(app->runtime.host);

    if (cfg->execute != nullptr) {
        if (cfg->setup != nullptr) { cfg->setup(app); }
        cfg->execute(app);
        goto out;
    }

	while (k_sem_take(&app->runtime.exit_sem, K_MSEC(250)) != 0) {
		if (cfg->idle != nullptr) {
			cfg->idle(app);
		}
		flush_eeprom(app, cfg);
	}

out:
	if (rc != 0) {
		printk("[%s] start failed at %s: %d\n", cfg->log_tag, stage, rc);
	}
	cleanup(app);
	if (cfg->teardown != nullptr) {
		cfg->teardown(app);
	}
	teardown_eeprom(app, cfg);
	printk("[%s] exit\n", cfg->log_tag);
	return rc;
}

} /* namespace meshbus::arduboy::llext_game */

#endif /* MESHBUS_ARDUBOY_LLEXT_GAME_HPP_ */
