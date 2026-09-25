#include <cassert>
#include <fstream>
#include "../examples/save_migration/migrations.hpp"
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
static int migrate_copy(uint32_t from, const uint8_t *src, size_t n, uint8_t *dst, size_t size)
{
    if (from > 1 || n != size) { return -EINVAL; }
    memcpy(dst, src, n); return 0;
}
int main()
{
    files.clear();
    files["/extra/saves/migrate.dat"] = std::vector<uint8_t>(1024, 0x5a);
    auto original = files["/extra/saves/migrate.dat"];
    EepromFile e; uint8_t ram[1024];
    assert(e.init_versioned("migrate", ram, sizeof(ram), 1) == 0);
    assert(e.load() == -EAGAIN);
    assert(e.load_status() == meshbus::arduboy::SaveLoadStatus::legacy_available);
    e.write(16, 1); assert(e.flush() == -EACCES);
    assert(!files.count("/extra/saves/migrate.sav"));
    assert(e.init_versioned("migrate", ram, sizeof(ram), 1, migrate_copy) == 0);
    assert(e.load() == 0 && e.load_status() == meshbus::arduboy::SaveLoadStatus::migrated);
    assert(ram[16] == 0x5a && files["/extra/saves/migrate.dat"] == original);
    auto saved = files[e.path()];
    files[e.path()].back() ^= 1;
    auto damaged = files[e.path()];
    assert(e.load() == -EBADMSG);
    e.write(16, 2); assert(e.flush() == -EACCES && files[e.path()] == damaged);
    files[e.path()] = saved;
    assert(e.init_versioned("migrate", ram, sizeof(ram), 2, migrate_copy) == 0);
    failure = 4; assert(e.load() == -EIO); failure = 0;
    assert(files[e.path()] == saved && files["/extra/saves/migrate.dat"] == original);
    assert(e.load() == 0 && e.read(16) == 0x5a);

    using namespace meshbus::arduboy;
    assert(e.init_versioned("fresh", ram, sizeof(ram), 1) == 0);
    assert(e.load() == 0 && e.load_status() == SaveLoadStatus::fresh);
    e.write(16, 42); assert(e.flush() == 0);
    auto good = files[e.path()];
    failure = 7; assert(e.load() == -EIO); failure = 0;
    e.write(16, 9); assert(e.flush() == -EACCES && files[e.path()] == good);
    files[e.path()].pop_back(); assert(e.load() == -EMSGSIZE);
    e.write(16, 9); assert(e.flush() == -EACCES);
    files[e.path()] = good;
    files[e.path()][24] = 'x';
    save_format::write32(files[e.path()].data() + 72, save_format::crc32(files[e.path()].data(), 72));
    assert(e.load() == -EXDEV && e.flush() == -EACCES);
    for (const auto *id : {"hopper", "hollow"}) {
        std::string fixture = std::string(__FILE__);
        fixture = fixture.substr(0, fixture.find_last_of('/')) + "/fixtures/saves/" + id + ".dat";
        std::ifstream input(fixture, std::ios::binary);
        std::vector<uint8_t> raw((std::istreambuf_iterator<char>(input)), {});
        assert(raw.size() == 1024);
        std::string path = std::string("/extra/saves/") + id + ".dat";
        files[path] = raw;
        auto migrate = strcmp(id, "hopper") == 0 ? save_examples::hopper : save_examples::hollow;
        assert(e.init_versioned(id, ram, sizeof(ram), 1, migrate) == 0);
        assert(e.load() == 0 && ram[2] == raw[0]);
        assert(memcmp(ram + 16, raw.data() + 16, 1008) == 0 && files[path] == raw);
        size_t base = strcmp(id, "hopper") == 0 ? 800 : 768;
        uint32_t signature = base == 800 ? 0x024e424f : 0x014e424f;
        save_format::write32(raw.data() + base, signature);
        uint16_t sum = static_cast<uint16_t>(signature + (signature >> 16)*2);
        for (unsigned i=0; i<13; ++i) {
            save_format::write16(raw.data() + base + 4 + i*2, i+100);
            sum = static_cast<uint16_t>(sum + (i+100)*(i+3));
        }
        save_format::write16(raw.data() + base + 30, sum);
        assert(migrate(0, raw.data(), raw.size(), ram, sizeof(ram)) == 0);
        assert(memcmp(ram + base, raw.data() + base, 32) == 0);
        raw[base+5] ^= 1;
        assert(migrate(0, raw.data(), raw.size(), ram, sizeof(ram)) == -EBADMSG);
    }
    uint8_t avr[1024] = {};
    save_format::write16(avr + 16, 60000); save_format::write16(avr + 18, 0xff85);
    assert(save_examples::avr_to_c2(1, avr, sizeof(avr), ram, sizeof(ram)) == 0);
    assert(save_format::read32(ram + 16) == 60000);
    assert(save_format::read32(ram + 20) == UINT32_C(0xffffff85));
}
