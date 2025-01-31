#ifndef UTF8_SCANNER_UTF8_SCANNER_H_
#define UTF8_SCANNER_UTF8_SCANNER_H_

#include <cstdint>

namespace utf8_scanner {

constexpr bool IsAscii(std::uint8_t byte) noexcept {
  // 0xxxxxxx, 0x00 ~ 0x7f
  return (byte & 0b10000000) == 0b00000000;
}

}  // namespace utf8_scanner

#endif  // UTF8_SCANNER_UTF8_SCANNER_H_
