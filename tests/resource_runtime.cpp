/* SPDX-License-Identifier: Apache-2.0 */
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#define MESHBUS_ARDUBOY_RESOURCES 1
#include "../src/runtime.cpp"

static int setups;
static void setup_resource()
{
    ++setups;
    uint8_t value = 0;
    assert(meshbus::arduboy::resource_length() == 1);
    assert(meshbus::arduboy::resource_read(0, &value, 1) == 0 && value == 42);
}
static void loop_resource() {}
int main()
{
    using namespace meshbus::arduboy;
    ResourceSpec spec{"/extra/apps/fixture.abr", "fixture", 1, 1, 0};
    uint8_t payload = 42; spec.crc32 = save_format::crc32(&payload, 1);
    std::vector<uint8_t> data(65, 0);
    memcpy(data.data(), "MARB", 4);
    save_format::write16(data.data()+4, 1); save_format::write16(data.data()+6, 64);
    save_format::write32(data.data()+8, 1); save_format::write32(data.data()+12, 1);
    save_format::write32(data.data()+16, spec.crc32); memcpy(data.data()+24, "fixture", 7);
    save_format::write32(data.data()+60, save_format::crc32(data.data(), 60)); data[64] = 42;
    files[spec.path] = data;
    SketchConfig config{"fixture", setup_resource, loop_resource}; config.resources = &spec;
    zui_host host{}; mbs_desktop_app_args args{&host};
    fixture::fail = 3;
    assert(run_sketch(&args, config) == -ENOMEM && open_files == 0 && setups == 0);
    fixture::fail = 0;
    assert(run_sketch(&args, config) == 0 && open_files == 0 && setups == 1);
    assert(resource_length() == 0);
    files[spec.path].pop_back();
    assert(run_sketch(&args, config) == -ENODATA && open_files == 0 && setups == 1);
    assert(!host.attached && fixture::routers == 0 && fixture::screens == 0);
}
