#include <cassert>
#include <vector>
#include <meshbus_arduboy/llext_game.hpp>
namespace game=meshbus::arduboy::llext_game;
struct App {game::FullscreenState<App> runtime;uint8_t fb[1024];};
uint8_t *fb(App *a) {return a->fb;}
std::vector<uint32_t> consumed;
bool setup_attached=false, execute_attached=false;
void owned_setup(App *a) { setup_attached = a->runtime.host->attached == a->runtime.router; }
void execute(App *a) { execute_attached = a->runtime.host->attached == a->runtime.router; }
void tick(App *a) { consumed.push_back(a->runtime.actions.down); }
const zui_screen_ops ops={game::draw<App>,game::input<App>,nullptr,nullptr,game::event<App>};
const game::FullscreenConfig<App> cfg={"test",1,60,128,64,128,ZUI_BITMAP_FORMAT_MONO_VLSB,ZUI_ACTION_CANCEL,fb,nullptr,tick,nullptr,nullptr};
int main() {
 for(int mode=0;mode<5;mode++) {
  App app{}; zui_router original;zui_host host{&original};mbs_desktop_app_args args{&host};fixture::fail=mode;
  int rc=game::run(&args,&app,&cfg,&ops);
  assert(rc==(mode==0?-EALREADY:mode==4?-EINVAL:-ENOMEM));
  assert(host.attached==&original); // Failure must preserve the existing app.
  assert(fixture::routers==0&&fixture::screens==0);
 }
 fixture::fail=0;zui_host host{};mbs_desktop_app_args args{&host};App app{};
 assert(game::run(&args,&app,&cfg,&ops)==0); assert(!host.attached);

 zui_input_event click{ZUI_ACTION_PRIMARY, ZUI_INPUT_ACTION_CLICK};
 game::input<App>(&click, &app); game::input<App>(&click, &app);
 zui_screen_event frames{ZUI_SCREEN_EVENT_TICK, 4}; game::event<App>(&frames, &app);
 assert((consumed == std::vector<uint32_t>{ZUI_ACTION_PRIMARY, 0, ZUI_ACTION_PRIMARY, 0}));
 game::input<App>(&click, &app); game::leave<App>(&app);
 consumed.clear(); frames.code = 1; game::event<App>(&frames, &app); assert(consumed[0] == 0);
 game::cleanup(&app);assert(!host.attached);assert(fixture::routers==0&&fixture::screens==0);
 auto owner_cfg = cfg; owner_cfg.setup = owned_setup; owner_cfg.execute = execute;
 App owned{}; assert(game::run(&args, &owned, &owner_cfg, &ops) == 0);
 assert(setup_attached && execute_attached && !host.attached);

}
