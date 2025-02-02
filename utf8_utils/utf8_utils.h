#ifndef UTF8_UTILS_UTF8_UTILS_H_
#define UTF8_UTILS_UTF8_UTILS_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <variant>

namespace utf8_utils {

constexpr std::array<std::uint8_t, 256> kUtf8BytesLength = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x00 ~ 0x0f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x10 ~ 0x1f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x20 ~ 0x2f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x30 ~ 0x3f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x40 ~ 0x4f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x50 ~ 0x5f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x60 ~ 0x6f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x70 ~ 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x80 ~ 0x8f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x90 ~ 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xa0 ~ 0xaf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xb0 ~ 0xbf
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 0xc0 ~ 0xcf
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 0xd0 ~ 0xdf
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 0xe0 ~ 0xef
    4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xf0 ~ 0xff
};

constexpr std::uint8_t Utf8BytesLength(const std::uint8_t first_byte) noexcept {
  return utf8_utils::kUtf8BytesLength[first_byte];
}

constexpr bool IsContinuation(const std::uint8_t b) noexcept {
  return (b & 0b11000000) == 0b10000000;
}

constexpr bool IsOverlong3Byte(const std::uint8_t b0,
                               const std::uint8_t b1) noexcept {
  return b0 == 0xe0 && b1 < 0xa0;
}

constexpr bool IsUtf16Surrogate(const std::uint8_t b0,
                                const std::uint8_t b1) noexcept {
  return b0 == 0xed && 0xa0 <= b1;
}

constexpr bool IsOverlong4Byte(const std::uint8_t b0,
                               const std::uint8_t b1) noexcept {
  return b0 == 0xf0 && b1 < 0x90;
}

constexpr bool IsOutOfUnicodeRange(const std::uint8_t b0,
                                   const std::uint8_t b1) noexcept {
  return b0 == 0xf4 && 0x90 <= b1;
}

constexpr char32_t ToUtf32Unchecked(const std::uint8_t byte) noexcept {
  return byte;
}

constexpr char32_t ToUtf32Unchecked(const std::uint8_t b0,
                                    const std::uint8_t b1) noexcept {
  return (b0 & 0b00011111) << 6 | (b1 & 0b00111111);
}

constexpr char32_t ToUtf32Unchecked(const std::uint8_t b0,
                                    const std::uint8_t b1,
                                    const std::uint8_t b2) noexcept {
  return (b0 & 0b00001111) << 12 | (b1 & 0b00111111) << 6 | (b2 & 0b00111111);
}

constexpr char32_t ToUtf32Unchecked(const std::uint8_t b0,
                                    const std::uint8_t b1,
                                    const std::uint8_t b2,
                                    const std::uint8_t b3) noexcept {
  return (b0 & 0b00000111) << 18 | (b1 & 0b00111111) << 12 |
         (b2 & 0b00111111) << 6 | (b3 & 0b00111111);
}

enum class ErrorCode : std::uint8_t {
  kUnexpected,
  kNotAscii,
  kNotFirstOf2Bytes,
  kNotFirstOf3Bytes,
  kNotFirstOf4Bytes,
  kNotSecondIsContinuation,
  kNotThirdIsContinuation,
  kNotFourthIsContinuation,
  kOverlongOf3Bytes,
  kUtf16Surrogate,
  kOverlongOf4Bytes,
  kOutOfUnicodeRange,
  kNullBytesPtr,
  kInvalidBytesLength,
  kDisallowedFirstByte,
  kIncomplete2Bytes,
  kIncomplete3Bytes,
  kIncomplete4Bytes,
};

using TryResult = std::variant<char32_t, utf8_utils::ErrorCode>;

constexpr bool HasValue(const utf8_utils::TryResult& result) noexcept {
  return std::holds_alternative<char32_t>(result);
}

constexpr bool HasError(const utf8_utils::TryResult& result) noexcept {
  return std::holds_alternative<utf8_utils::ErrorCode>(result);
}

constexpr std::optional<char32_t> GetValue(
    const utf8_utils::TryResult& result) noexcept {
  if (utf8_utils::HasValue(result)) {
    return std::get<char32_t>(result);
  }

  return std::nullopt;
}

constexpr std::optional<utf8_utils::ErrorCode> GetError(
    const utf8_utils::TryResult& result) noexcept {
  if (utf8_utils::HasError(result)) {
    return std::get<utf8_utils::ErrorCode>(result);
  }

  return std::nullopt;
}

constexpr char32_t MustValue(const utf8_utils::TryResult& result) noexcept {
  assert(utf8_utils::HasValue(result) && "Result must have a value.");
  return std::get<char32_t>(result);
}

constexpr utf8_utils::ErrorCode MustError(
    const utf8_utils::TryResult& result) noexcept {
  assert(utf8_utils::HasError(result) && "Result must have an error.");
  return std::get<utf8_utils::ErrorCode>(result);
}

constexpr utf8_utils::TryResult TryToUtf32(const std::uint8_t byte) noexcept {
  if (utf8_utils::Utf8BytesLength(byte) != 1) {
    return utf8_utils::ErrorCode::kNotAscii;
  }

  return utf8_utils::ToUtf32Unchecked(byte);
}

constexpr utf8_utils::TryResult TryToUtf32(const std::uint8_t b0,
                                           const std::uint8_t b1) noexcept {
  if (utf8_utils::Utf8BytesLength(b0) != 2) {
    return utf8_utils::ErrorCode::kNotFirstOf2Bytes;
  }

  if (!utf8_utils::IsContinuation(b1)) {
    return utf8_utils::ErrorCode::kNotSecondIsContinuation;
  }

  return utf8_utils::ToUtf32Unchecked(b0, b1);
}

constexpr utf8_utils::TryResult TryToUtf32(const std::uint8_t b0,
                                           const std::uint8_t b1,
                                           const std::uint8_t b2) noexcept {
  if (Utf8BytesLength(b0) != 3) {
    return utf8_utils::ErrorCode::kNotFirstOf3Bytes;
  }

  if (IsOverlong3Byte(b0, b1)) {
    return utf8_utils::ErrorCode::kOverlongOf3Bytes;
  }

  if (IsUtf16Surrogate(b0, b1)) {
    return utf8_utils::ErrorCode::kUtf16Surrogate;
  }

  if (!IsContinuation(b1)) {
    return utf8_utils::ErrorCode::kNotSecondIsContinuation;
  }

  if (!IsContinuation(b2)) {
    return utf8_utils::ErrorCode::kNotThirdIsContinuation;
  }

  return utf8_utils::ToUtf32Unchecked(b0, b1, b2);
}

constexpr utf8_utils::TryResult TryToUtf32(const std::uint8_t b0,
                                           const std::uint8_t b1,
                                           const std::uint8_t b2,
                                           const std::uint8_t b3) noexcept {
  if (Utf8BytesLength(b0) != 4) {
    return utf8_utils::ErrorCode::kNotFirstOf4Bytes;
  }

  if (IsOverlong4Byte(b0, b1)) {
    return utf8_utils::ErrorCode::kOverlongOf4Bytes;
  }

  if (IsOutOfUnicodeRange(b0, b1)) {
    return utf8_utils::ErrorCode::kOutOfUnicodeRange;
  }

  if (!IsContinuation(b1)) {
    return utf8_utils::ErrorCode::kNotSecondIsContinuation;
  }

  if (!IsContinuation(b2)) {
    return utf8_utils::ErrorCode::kNotThirdIsContinuation;
  }

  if (!IsContinuation(b3)) {
    return utf8_utils::ErrorCode::kNotFourthIsContinuation;
  }

  return utf8_utils::ToUtf32Unchecked(b0, b1, b2, b3);
}

constexpr utf8_utils::TryResult TryToUtf32(const std::uint8_t* bytes,
                                           const std::size_t len) noexcept {
  if (bytes == nullptr) {
    return utf8_utils::ErrorCode::kNullBytesPtr;
  }

  switch (len) {
    case 1:
      return utf8_utils::TryToUtf32(bytes[0]);
    case 2:
      return utf8_utils::TryToUtf32(bytes[0], bytes[1]);
    case 3:
      return utf8_utils::TryToUtf32(bytes[0], bytes[1], bytes[2]);
    case 4:
      return utf8_utils::TryToUtf32(bytes[0], bytes[1], bytes[2], bytes[3]);
    default:
      return utf8_utils::ErrorCode::kInvalidBytesLength;
  }
}

constexpr char32_t MustToUtf32(const std::uint8_t byte) noexcept {
  assert(utf8_utils::Utf8BytesLength(byte) == 1 &&
         "Parameter `byte` must be a 1-byte UTF-8 character.");

  return utf8_utils::ToUtf32Unchecked(byte);
}

constexpr char32_t MustToUtf32(const std::uint8_t b0,
                               const std::uint8_t b1) noexcept {
  assert(utf8_utils::Utf8BytesLength(b0) == 2 &&
         "Parameter `b0` must be a first byte of 2-byte UTF-8 character.");
  assert(utf8_utils::IsContinuation(b1) &&
         "Parameter `b1` must be a continuation byte.");

  return utf8_utils::ToUtf32Unchecked(b0, b1);
}

constexpr char32_t MustToUtf32(const std::uint8_t b0, const std::uint8_t b1,
                               const std::uint8_t b2) noexcept {
  assert(utf8_utils::Utf8BytesLength(b0) == 3 &&
         "Parameter `b0` must be a first byte of 3-byte UTF-8 character.");
  assert(!utf8_utils::IsOverlong3Byte(b0, b1) &&
         "Parameter `b0` and `b1` must not form an overlong 3-byte UTF-8 "
         "character.");
  assert(!utf8_utils::IsUtf16Surrogate(b0, b1) &&
         "Parameter `b0` and `b1` must not form a UTF-16 surrogate.");
  assert(utf8_utils::IsContinuation(b1) &&
         "Parameter `b1` must be a continuation byte.");
  assert(utf8_utils::IsContinuation(b2) &&
         "Parameter `b2` must be a continuation byte.");

  return utf8_utils::ToUtf32Unchecked(b0, b1, b2);
}

constexpr char32_t MustToUtf32(const std::uint8_t b0, const std::uint8_t b1,
                               const std::uint8_t b2,
                               const std::uint8_t b3) noexcept {
  assert(utf8_utils::Utf8BytesLength(b0) == 4 &&
         "Parameter `b0` must be a first byte of 4-byte UTF-8 character.");
  assert(!utf8_utils::IsOverlong4Byte(b0, b1) &&
         "Parameter `b0` and `b1` must not form an overlong 4-byte UTF-8 "
         "character.");
  assert(!utf8_utils::IsOutOfUnicodeRange(b0, b1) &&
         "Parameter `b0` and `b1` must not form a character out of "
         "Unicode range.");
  assert(utf8_utils::IsContinuation(b1) &&
         "Parameter `b1` must be a continuation byte.");
  assert(utf8_utils::IsContinuation(b2) &&
         "Parameter `b2` must be a continuation byte.");
  assert(utf8_utils::IsContinuation(b3) &&
         "Parameter `b3` must be a continuation byte.");

  return utf8_utils::ToUtf32Unchecked(b0, b1, b2, b3);
}

constexpr char32_t MustToUtf32(const std::uint8_t* bytes,
                               const std::size_t len) noexcept {
  assert(bytes != nullptr && "Parameter `bytes` must not be nullptr.");
  assert((0 < len && len <= 4) &&
         "Parameter `len` must be greater than 0 and less than or "
         "4.");

  switch (len) {
    case 1:
      return utf8_utils::MustToUtf32(bytes[0]);
    case 2:
      return utf8_utils::MustToUtf32(bytes[0], bytes[1]);
    case 3:
      return utf8_utils::MustToUtf32(bytes[0], bytes[1], bytes[2]);
    case 4:
      return utf8_utils::MustToUtf32(bytes[0], bytes[1], bytes[2], bytes[3]);
    default:
      assert(false && "Parameter `len` must be 1, 2, 3, or 4.");
      return 0;
  }
}

struct CheckError {
  utf8_utils::ErrorCode code{utf8_utils::ErrorCode::kUnexpected};
  std::size_t invalid_position{};
  std::size_t invalid_length{};
};

namespace detail {

constexpr std::optional<utf8_utils::CheckError> Check2Bytes(
    const std::uint8_t* bytes, const std::size_t len, const std::size_t start,
    const std::uint8_t b0, std::size_t& i) noexcept {
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete2Bytes,
                                  start, i - start};
  }

  const std::uint8_t b1 = bytes[i];
  if (!utf8_utils::IsContinuation(b1)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotSecondIsContinuation, start, i - start};
  }

  ++i;
  return std::nullopt;
}

constexpr std::optional<utf8_utils::CheckError> Check3Bytes(
    const std::uint8_t* bytes, const std::size_t len, const std::size_t start,
    const std::uint8_t b0, std::size_t& i) noexcept {
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete3Bytes,
                                  start, i - start};
  }

  const std::uint8_t b1 = bytes[i];
  if (utf8_utils::IsOverlong3Byte(b0, b1)) {
    ++i;
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kOverlongOf3Bytes,
                                  start, i - start};
  }

  if (utf8_utils::IsUtf16Surrogate(b0, b1)) {
    ++i;
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kUtf16Surrogate, start,
                                  i - start};
  }

  if (!utf8_utils::IsContinuation(b1)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotSecondIsContinuation, start, i - start};
  }

  ++i;
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete3Bytes,
                                  start, i - start};
  }

  const std::uint8_t b2 = bytes[i];
  if (!utf8_utils::IsContinuation(b2)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotThirdIsContinuation, start, i - start};
  }

  ++i;
  return std::nullopt;
}

constexpr std::optional<utf8_utils::CheckError> Check4Bytes(
    const std::uint8_t* bytes, const std::size_t len, const std::size_t start,
    const std::uint8_t b0, std::size_t& i) noexcept {
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete4Bytes,
                                  start, i - start};
  }

  const std::uint8_t b1 = bytes[i];
  if (utf8_utils::IsOverlong4Byte(b0, b1)) {
    ++i;
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kOverlongOf4Bytes,
                                  start, i - start};
  }

  if (utf8_utils::IsOutOfUnicodeRange(b0, b1)) {
    ++i;
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kOutOfUnicodeRange,
                                  start, i - start};
  }

  if (!utf8_utils::IsContinuation(b1)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotSecondIsContinuation, start, i - start};
  }

  ++i;
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete4Bytes,
                                  start, i - start};
  }

  const std::uint8_t b2 = bytes[i];
  if (!utf8_utils::IsContinuation(b2)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotThirdIsContinuation, start, i - start};
  }

  ++i;
  if (i >= len) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kIncomplete4Bytes,
                                  start, i - start};
  }

  const std::uint8_t b3 = bytes[i];
  if (!utf8_utils::IsContinuation(b3)) {
    return utf8_utils::CheckError{
        utf8_utils::ErrorCode::kNotFourthIsContinuation, start, i - start};
  }

  ++i;
  return std::nullopt;
}

}  // namespace detail

constexpr std::optional<utf8_utils::CheckError> Check(
    const std::uint8_t* bytes, const std::size_t len) noexcept {
  if (bytes == nullptr) {
    return utf8_utils::CheckError{utf8_utils::ErrorCode::kNullBytesPtr, 0, 0};
  }

  if (len == 0) {
    return std::nullopt;
  }

  std::size_t i{};
  while (i < len) {
    const std::size_t start = i;
    const std::uint8_t b0 = bytes[i++];
    const std::uint8_t len = Utf8BytesLength(b0);

    if (len == 0) {
      return utf8_utils::CheckError{utf8_utils::ErrorCode::kDisallowedFirstByte,
                                    start, i - start};
    }

    if (len == 1) {
      continue;
    }

    if (len == 2) {
      if (auto err = utf8_utils::detail::Check2Bytes(bytes, len, start, b0, i);
          err) {
        return err;
      }

      continue;
    }

    if (len == 3) {
      if (auto err = utf8_utils::detail::Check3Bytes(bytes, len, start, b0, i);
          err) {
        return err;
      }

      continue;
    }

    if (len == 4) {
      if (auto err = utf8_utils::detail::Check4Bytes(bytes, len, start, b0, i);
          err) {
        return err;
      }

      continue;
    }
  }

  return std::nullopt;
}

}  // namespace utf8_utils

#endif  // UTF8_UTILS_UTF8_UTILS_H_
