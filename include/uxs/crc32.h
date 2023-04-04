#pragma once

#include "common.h"

#include <array>

namespace uxs {

namespace detail {
// CRC32 lookup table
struct crc32_table_t {
    std::array<uint32_t, 256> v{};
    CONSTEXPR crc32_table_t() {
        // The official polynomial used by CRC-32 in PKZip, WinZip and Ethernet
        const uint32_t poly = 0x04c11db7;
        const uint32_t msbit = 0x80000000;
        for (unsigned i = 0; i < 256; ++i) {
            uint32_t r = reflect(i, 8) << 24;
            for (int j = 0; j < 8; ++j) { r = (r << 1) ^ (r & msbit ? poly : 0); }
            v[i] = reflect(r, 32);
        }
    }
    CONSTEXPR uint32_t reflect(uint32_t ref, unsigned ch) {
        uint32_t value = 0;
        for (unsigned i = 1; i < ch + 1; ++i) {  // Swap bit 0 for bit 7 bit 1 for bit 6, etc.
            if (ref & 1) { value |= 1 << (ch - i); }
            ref >>= 1;
        }
        return value;
    }
};
}  // namespace detail

class crc32 {
 public:
    template<typename InputIt>
    static CONSTEXPR uint32_t calc(InputIt it, InputIt end, uint32_t crc32 = 0xffffffff) {
        while (it != end) { crc32 = (crc32 >> 8) ^ table_.v[(crc32 & 0xff) ^ static_cast<uint8_t>(*it++)]; }
        return crc32;
    }
    static CONSTEXPR uint32_t calc(const char* zstr, uint32_t crc32 = 0xffffffff) {
        while (*zstr) { crc32 = (crc32 >> 8) ^ table_.v[(crc32 & 0xff) ^ static_cast<uint8_t>(*zstr++)]; }
        return crc32;
    }

 private:
#if __cplusplus < 201703L
    static UXS_EXPORT const detail::crc32_table_t table_;
#else   // __cplusplus < 201703L
    static constexpr detail::crc32_table_t table_{};
#endif  // __cplusplus < 201703L
};

}  // namespace uxs
