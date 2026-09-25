/* SPDX-License-Identifier: Apache-2.0 */
#include <meshbus_arduboy/fx_data.hpp>
#include <cassert>
#include <vector>
std::vector<uint8_t> bytes;
namespace meshbus::arduboy {
int resource_read(uint32_t offset, void *out, size_t count) {
    if (offset > bytes.size() || count > bytes.size() - offset) { return -ERANGE; }
    memcpy(out, bytes.data() + offset, count); return 0;
}
uint32_t resource_length() { return bytes.size(); }
}
int main()
{
    using namespace meshbus::arduboy;
    bytes.assign(32, 0); memcpy(bytes.data(), "FXPK", 4);
    save_format::write16(bytes.data()+4, 1); save_format::write16(bytes.data()+6, 256);
    save_format::write32(bytes.data()+8, 300); save_format::write16(bytes.data()+12, 0xfebe);
    save_format::write32(bytes.data()+16, 2);
    constexpr unsigned offsets[] = {32,38,83};
    for (unsigned i=0; i<3; ++i) { save_format::write32(bytes.data()+20+i*4, offsets[i]); }
    bytes.insert(bytes.end(), {0,42,255,1,250,1}); bytes.push_back(43);
    for (unsigned i=0; i<44; ++i) { bytes.push_back(i); }
    FxData data; uint8_t got[32];
    assert(data.begin(0xfebe)==0 && data.length()==300);
    assert(data.read(250,got,32)==0 && data.decodes==2);
    for (unsigned i=0; i<32; ++i) { assert(got[i] == (i<6 ? 42 : i-6)); }
    assert(data.read(256,got,32)==0 && data.decodes==2);
    assert(data.read(UINT32_MAX,got,1)==-ERANGE);
    assert(data.begin(0)==-EPROTONOSUPPORT);
    auto good=bytes;
    bytes[35]=0; assert(data.begin(0xfebe)==0 && data.read(0,got,1)==-EBADMSG);
    bytes=good; bytes.pop_back(); assert(data.begin(0xfebe)==-EBADMSG);
    bytes=good; save_format::write32(bytes.data()+24,32); assert(data.begin(0xfebe)==-EBADMSG);
    bytes=good; bytes[38]=127; assert(data.begin(0xfebe)==0 && data.read(256,got,1)==-EBADMSG);
}
