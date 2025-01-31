#ifndef UTF8_SCANNER_UTF8_SCANNER_H_
#define UTF8_SCANNER_UTF8_SCANNER_H_

#include <array>
#include <cstdint>
#include <istream>
#include <optional>
#include <variant>

namespace utf8_scanner {

enum class Error {
  kStreamIsNotGood,
  kStreamIsEof,
  kStreamIsBad,
  kStreamIsFail,
};

using Byte = std::uint8_t;
using TwoByteSequence = std::array<Byte, 2>;
using ThreeByteSequence = std::array<Byte, 3>;
using FourByteSequence = std::array<Byte, 4>;
using CodePoint = std::uint32_t;

namespace detail {

constexpr std::optional<Error> CheckAfterOp(std::istream& stream) noexcept {
  if (stream.eof()) {
    return Error::kStreamIsEof;
  }

  if (stream.bad()) {
    return Error::kStreamIsBad;
  }

  if (stream.fail()) {
    return Error::kStreamIsFail;
  }

  return std::nullopt;
}

constexpr std::variant<Byte, Error> ReadByte(std::istream& stream) noexcept {
  if (!stream.good()) {
    return Error::kStreamIsNotGood;
  }

  char byte{};
  stream.get(byte);
  if (auto err = CheckAfterOp(stream); err) {
    return *err;
  }

  return static_cast<Byte>(byte);
}

constexpr std::variant<Byte, Error> PeekByte(std::istream& stream) noexcept {
  if (!stream.good()) {
    return Error::kStreamIsNotGood;
  }

  auto byte = stream.peek();
  if (auto err = CheckAfterOp(stream); err) {
    return *err;
  }

  return static_cast<Byte>(byte);
}

}  // namespace detail

// Range: 0x00 ~ 0x7f
constexpr bool IsAscii(Byte byte) noexcept {
  return (byte & 0b10000000) == 0b00000000;
}

// Range: 0xc2 ~ 0xdf
// 0xc0, 0xc1 are disallowed in UTF-8.
constexpr bool IsStartOf2ByteSequence(Byte byte) noexcept {
  return (byte & 0b11100000) == 0b11000000 && 0xc2 <= byte;
}

// Range: 0xe0 ~ 0xef
constexpr bool IsStartOf3ByteSequence(Byte byte) noexcept {
  return (byte & 0b11110000) == 0b11100000;
}

// Range: 0xf0 ~ 0xf4
// 0xf5 ~ 0xff are disallowed in UTF-8.
constexpr bool IsStartOf4ByteSequence(Byte byte) noexcept {
  return (byte & 0b11111000) == 0b11110000 && byte <= 0xf4;
}

// Range: 0x80 ~ 0xbf
// A continuation byte at the start of a character is invalid.
// A non-continuation byte before the end of a character is invalid.
constexpr bool IsContinuationByte(Byte byte) noexcept {
  return (byte & 0b11000000) == 0b10000000;
}

constexpr bool Is3ByteOverlongEncoding(Byte b0, Byte b1) noexcept {
  return b0 == 0xe0 && b1 < 0xa0;
}

constexpr bool Is4ByteOverlongEncoding(Byte b0, Byte b1) noexcept {
  return b0 == 0xf0 && b1 < 0x90;
}

constexpr bool Is4ByteOutOfUnicodeRange(Byte b0, Byte b1) noexcept {
  return b0 == 0xf4 && 0x90 <= b1;
}

constexpr bool Is3ByteUtf16Surrogate(Byte b0, Byte b1) noexcept {
  return b0 == 0xed && 0xa0 <= b1;
}

constexpr CodePoint CodePointFromAscii(Byte byte) noexcept { return byte; }

constexpr CodePoint CodePointFrom2ByteSequence(Byte b0, Byte b1) noexcept {
  return (b0 & 0b00011111) << 6 | (b1 & 0b00111111);
}

constexpr CodePoint CodePointFrom3ByteSequence(Byte b0, Byte b1,
                                               Byte b2) noexcept {
  return (b0 & 0b00001111) << 12 | (b1 & 0b00111111) << 6 | (b2 & 0b00111111);
}

constexpr CodePoint CodePointFrom4ByteSequence(Byte b0, Byte b1, Byte b2,
                                               Byte b3) noexcept {
  return (b0 & 0b00000111) << 18 | (b1 & 0b00111111) << 12 |
         (b2 & 0b00111111) << 6 | (b3 & 0b00111111);
}

constexpr std::variant<Byte, TwoByteSequence, ThreeByteSequence,
                       FourByteSequence, Error>
Scan(std::istream& stream, Byte first_byte) noexcept {
  if (IsAscii(first_byte)) {
    return first_byte;
  }

  if (IsStartOf2ByteSequence(first_byte)) {
    auto&& b1 = detail::PeekByte(stream);
    if (std::holds_alternative<Error>(b1)) {
      return std::get<Error>(b1);
    }

    return TwoByteSequence{first_byte, std::get<Byte>(b1)};
  }

  return Error::kStreamIsBad;
}

constexpr std::variant<Byte, TwoByteSequence, ThreeByteSequence,
                       FourByteSequence, Error>
Scan(std::istream& stream) noexcept {
  auto b0 = detail::ReadByte(stream);
  if (std::holds_alternative<Error>(b0)) {
    return std::get<Error>(b0);
  }

  return Scan(stream, std::get<Byte>(b0));
}

}  // namespace utf8_scanner

#endif  // UTF8_SCANNER_UTF8_SCANNER_H_
