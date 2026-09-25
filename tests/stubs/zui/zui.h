#pragma once
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#define ZUI_ACTION_UP 1U
#define ZUI_ACTION_DOWN 2U
#define ZUI_ACTION_LEFT 4U
#define ZUI_ACTION_RIGHT 8U
#define ZUI_ACTION_PRIMARY 16U
#define ZUI_ACTION_SECONDARY 32U
#define ZUI_ACTION_MENU 64U
#define ZUI_ACTION_CANCEL 128U
#define ZUI_SCREEN_EVENT_TICK 1
#define ZUI_LAYER_FULLSCREEN 1
struct zui_draw_ctx {};
enum zui_input_action { ZUI_INPUT_ACTION_PRESS, ZUI_INPUT_ACTION_RELEASE,
    ZUI_INPUT_ACTION_CLICK, ZUI_INPUT_ACTION_LONG_PRESS };
struct zui_input_event { uint32_t mask; zui_input_action action; };
struct zui_screen_event {int type; uint32_t code;};
struct zui_action_state {uint32_t down,pressed,released,long_pressed;};
enum zui_bitmap_format {ZUI_BITMAP_FORMAT_MONO_VLSB};
struct zui_point {int16_t x,y;};
struct zui_framebuffer_view {size_t struct_size;uint8_t *data;uint16_t width,height,stride;zui_bitmap_format format;};
struct zui_screen_ops {
 void (*draw)(zui_draw_ctx*,void*);
 bool (*input)(const zui_input_event*,void*);
 void (*enter)(void*);
 void (*exit)(void*);
 bool (*event)(const zui_screen_event*,void*);
};
struct zui_screen {};
struct zui_router {};
struct zui_host {zui_router *attached;};
namespace fixture { inline int fail=0,routers=0,screens=0,detaches=0; }
inline void zui_draw_framebuffer(zui_draw_ctx*,zui_point,const zui_framebuffer_view*) {}
inline int zui_action_state_update(zui_action_state *s,const zui_input_event *e) {
 if (e->action == ZUI_INPUT_ACTION_PRESS) { s->pressed |= e->mask & ~s->down; s->down |= e->mask; }
 if (e->action == ZUI_INPUT_ACTION_RELEASE) { s->released |= e->mask & s->down; s->down &= ~e->mask; }
 if (e->action == ZUI_INPUT_ACTION_CLICK) { s->pressed |= e->mask; s->released |= e->mask; }
 if (e->action == ZUI_INPUT_ACTION_LONG_PRESS) { s->long_pressed |= e->mask; }
 return 0;
}
inline void zui_action_state_clear_edges(zui_action_state *s) {s->pressed=s->released=s->long_pressed=0;}
inline zui_router *zui_router_create() {if(fixture::fail==1)return nullptr;fixture::routers++;return new zui_router;}
inline zui_screen *zui_screen_create(const zui_screen_ops*,void*) {if(fixture::fail==2)return nullptr;fixture::screens++;return new zui_screen;}
inline void zui_router_destroy(zui_router *r) {if(r){fixture::routers--;delete r;}}
inline void zui_screen_destroy(zui_screen *s) {if(s){fixture::screens--;delete s;}}
inline int zui_router_register_screen(zui_router*,uint32_t,zui_screen*) {return fixture::fail==3?-ENOMEM:0;}
inline int zui_router_unregister_screen(zui_router*,uint32_t) {return 0;}
inline int zui_router_switch(zui_router*,uint32_t) {return fixture::fail==4?-EINVAL:0;}
inline int zui_host_attach_router(zui_host *h,int,zui_router*r) {if(h->attached&&h->attached!=r)return -EALREADY;h->attached=r;return 0;}
inline int zui_host_detach_router(zui_host*h,int) {fixture::detaches++;h->attached=nullptr;return 0;}
inline int zui_host_request_redraw(zui_host*) {return 0;}
inline int zui_screen_request_redraw(zui_screen*) {return 0;}
inline int zui_screen_set_tick_period(zui_screen*,uint32_t) {return 0;}
inline int zui_host_send_layer_to_front(zui_host*,int) {return 0;}
inline int zui_host_set_layer_enabled(zui_host*,int,bool) {return 0;}
