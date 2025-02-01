#ifndef UTF8_SCANNER_UTF8_SCANNER_H_
#define UTF8_SCANNER_UTF8_SCANNER_H_

#include <cassert>
#include <cstdint>
#include <istream>
#include <variant>
#include <vector>

namespace utf8_scanner {

using Byte = std::uint8_t;
using CodePoint = std::uint32_t;

// Range: 0x00 ~ 0x7f
constexpr bool IsAscii(const Byte b) noexcept {
  return (b & 0b10000000) == 0b00000000;
}

// Range: 0xc2 ~ 0xdf
// 0xc0, 0xc1 are disallowed in UTF-8.
constexpr bool IsStartOf2Byte(const Byte b) noexcept {
  return (b & 0b11100000) == 0b11000000 && 0xc2 <= b;
}

// Range: 0xe0 ~ 0xef
constexpr bool IsStartOf3Byte(const Byte b) noexcept {
  return (b & 0b11110000) == 0b11100000;
}

// Range: 0xf0 ~ 0xf4
// 0xf5 ~ 0xff are disallowed in UTF-8.
constexpr bool IsStartOf4Byte(const Byte b) noexcept {
  return (b & 0b11111000) == 0b11110000 && b <= 0xf4;
}

// Range: 0x80 ~ 0xbf
// A continuation byte at the start of a character is invalid.
// A non-continuation byte before the end of a character is invalid.
constexpr bool IsContinuation(const Byte b) noexcept {
  return (b & 0b11000000) == 0b10000000;
}

constexpr bool IsOverlong3Byte(const Byte b0, const Byte b1) noexcept {
  return b0 == 0xe0 && b1 < 0xa0;
}

constexpr bool IsUtf16Surrogate(const Byte b0, const Byte b1) noexcept {
  return b0 == 0xed && 0xa0 <= b1;
}

constexpr bool IsOverlong4Byte(const Byte b0, const Byte b1) noexcept {
  return b0 == 0xf0 && b1 < 0x90;
}

constexpr bool IsOutOfUnicodeRange(const Byte b0, const Byte b1) noexcept {
  return b0 == 0xf4 && 0x90 <= b1;
}

constexpr CodePoint CodePointFromAscii(const Byte b) noexcept {
  assert(IsAscii(b) && "Parameter `b` must be ASCII.");
  return b;
}

constexpr CodePoint CodePointFrom2Byte(const Byte b0, const Byte b1) noexcept {
  assert(IsStartOf2Byte(b0) &&
         "Parameter `b0` must be the start of a 2-byte UTF-8 character.");
  assert(IsContinuation(b1) && "Parameter `b1` must be a continuation byte.");
  return (b0 & 0b00011111) << 6 | (b1 & 0b00111111);
}

constexpr CodePoint CodePointFrom3Byte(const Byte b0, const Byte b1,
                                       const Byte b2) noexcept {
  assert(IsStartOf3Byte(b0) &&
         "Parameter `b0` must be the start of a 3-byte UTF-8 character.");
  assert(!IsOverlong3Byte(b0, b1) &&
         "Parameter `b0` and `b1` must not form an overlong 3-byte UTF-8 "
         "character.");
  assert(!IsUtf16Surrogate(b0, b1) &&
         "Parameter `b0` and `b1` must not form a UTF-16 surrogate.");
  assert(IsContinuation(b1) && "Parameter `b1` must be a continuation byte.");
  assert(IsContinuation(b2) && "Parameter `b2` must be a continuation byte.");
  return (b0 & 0b00001111) << 12 | (b1 & 0b00111111) << 6 | (b2 & 0b00111111);
}

constexpr CodePoint CodePointFrom4Byte(const Byte b0, const Byte b1,
                                       const Byte b2, const Byte b3) noexcept {
  assert(IsStartOf4Byte(b0) &&
         "Parameter `b0` must be the start of a 4-byte UTF-8 character.");
  assert(!IsOverlong4Byte(b0, b1) &&
         "Parameter `b0` and `b1` must not form an overlong 4-byte UTF-8 "
         "character.");
  assert(!IsOutOfUnicodeRange(b0, b1) &&
         "Parameter `b0` and `b1` must not form a character out of "
         "Unicode range.");
  assert(IsContinuation(b1) && "Parameter `b1` must be a continuation byte.");
  assert(IsContinuation(b2) && "Parameter `b2` must be a continuation byte.");
  assert(IsContinuation(b3) && "Parameter `b3` must be a continuation byte.");
  return (b0 & 0b00000111) << 18 | (b1 & 0b00111111) << 12 |
         (b2 & 0b00111111) << 6 | (b3 & 0b00111111);
}

constexpr CodePoint CodePointFromBytes(const Byte* bytes,
                                       const std::size_t length) noexcept {
  assert(bytes != nullptr && "Parameter `bytes` must not be nullptr.");
  assert(length > 0 && "Parameter `length` must be greater than 0.");
  assert(length <= 4 && "Parameter `length` must be less than or equal to 4.");
  switch (length) {
    case 1:
      return CodePointFromAscii(bytes[0]);
    case 2:
      return CodePointFrom2Byte(bytes[0], bytes[1]);
    case 3:
      return CodePointFrom3Byte(bytes[0], bytes[1], bytes[2]);
    case 4:
      return CodePointFrom4Byte(bytes[0], bytes[1], bytes[2], bytes[3]);
    default:
      assert(false && "Parameter `length` must be 1, 2, 3, or 4.");
      return 0;
  }
}

enum class ScanResultCode : std::uint8_t {
  kValidCodePoint,
  kDisallowedStartByte,
  kStartWithContinuationByte,
  kIncomplete2Byte,
  kIncomplete3Byte,
  kIncomplete4Byte,
  kOverlong3Byte,
  kOverlong4Byte,
  kUtf16Surrogate,
  kOutOfUnicodeRange,
  kStreamEof,
  kStreamBad,
  kStreamFail,
  kStreamUnexpected,
};

using EncodedCodePoint = std::vector<Byte>;

class Scanner {
 public:
  explicit Scanner(std::istream& stream, EncodedCodePoint& buffer) noexcept
      : stream_(stream), buffer_(buffer) {
    buffer_.clear();
  }

  ~Scanner() noexcept = default;

  Scanner(const Scanner&) = delete;
  Scanner& operator=(const Scanner&) = delete;

  Scanner(Scanner&&) = delete;
  Scanner& operator=(Scanner&&) = delete;

  ScanResultCode Scan() noexcept {
    const PeekResult maybe_b0 = Peek();
    if (!HasValue(maybe_b0)) {
      return GetError(maybe_b0);
    }

    const Byte b0 = GetValue(maybe_b0);
    Consume(b0);

    if (IsAscii(b0)) {
      return ScanResultCode::kValidCodePoint;
    }

    if (IsStartOf2Byte(b0)) {
      return OnStartOf2Byte(b0);
    }

    if (IsStartOf3Byte(b0)) {
      return OnStartOf3Byte(b0);
    }

    if (IsStartOf4Byte(b0)) {
      return OnStartOf4Byte(b0);
    }

    if (IsContinuation(b0)) {
      return ScanResultCode::kStartWithContinuationByte;
    }

    return ScanResultCode::kDisallowedStartByte;
  }

 private:
  ScanResultCode OnStartOf2Byte(const Byte b0) noexcept {
    const PeekResult maybe_b1 = Peek();
    if (!HasValue(maybe_b1)) {
      return GetError(maybe_b1);
    }

    const Byte b1 = GetValue(maybe_b1);
    if (IsContinuation(b1)) {
      Consume(b1);
      return ScanResultCode::kValidCodePoint;
    }

    return ScanResultCode::kIncomplete2Byte;
  }

  ScanResultCode OnStartOf3Byte(const Byte b0) noexcept {
    const PeekResult maybe_b1 = Peek();
    if (!HasValue(maybe_b1)) {
      return GetError(maybe_b1);
    }

    const Byte b1 = GetValue(maybe_b1);
    if (IsOverlong3Byte(b0, b1)) {
      Consume(b1);
      return ScanResultCode::kOverlong3Byte;
    }

    if (IsUtf16Surrogate(b0, b1)) {
      Consume(b1);
      return ScanResultCode::kUtf16Surrogate;
    }

    if (IsContinuation(b1)) {
      Consume(b1);

      const PeekResult maybe_b2 = Peek();
      if (!HasValue(maybe_b2)) {
        return GetError(maybe_b2);
      }

      const Byte b2 = GetValue(maybe_b2);
      if (IsContinuation(b2)) {
        Consume(b2);
        return ScanResultCode::kValidCodePoint;
      }
    }

    return ScanResultCode::kIncomplete3Byte;
  }

  ScanResultCode OnStartOf4Byte(const Byte b0) noexcept {
    const PeekResult maybe_b1 = Peek();
    if (!HasValue(maybe_b1)) {
      return GetError(maybe_b1);
    }

    const Byte b1 = GetValue(maybe_b1);
    if (IsOverlong4Byte(b0, b1)) {
      Consume(b1);
      return ScanResultCode::kOverlong4Byte;
    }

    if (IsOutOfUnicodeRange(b0, b1)) {
      Consume(b1);
      return ScanResultCode::kOutOfUnicodeRange;
    }

    if (IsContinuation(b1)) {
      Consume(b1);

      const PeekResult maybe_b2 = Peek();
      if (!HasValue(maybe_b2)) {
        return GetError(maybe_b2);
      }

      const Byte b2 = GetValue(maybe_b2);
      if (IsContinuation(b2)) {
        Consume(b2);

        const PeekResult maybe_b3 = Peek();
        if (!HasValue(maybe_b3)) {
          return GetError(maybe_b3);
        }

        const Byte b3 = GetValue(maybe_b3);
        if (IsContinuation(b3)) {
          Consume(b3);
          return ScanResultCode::kValidCodePoint;
        }
      }
    }

    return ScanResultCode::kIncomplete4Byte;
  }

  using PeekValue = std::istream::int_type;
  using PeekResult = std::variant<PeekValue, ScanResultCode>;

  bool HasValue(const PeekResult& res) noexcept {
    return std::holds_alternative<PeekValue>(res);
  }

  Byte GetValue(const PeekResult& res) noexcept {
    return static_cast<Byte>(std::get<PeekValue>(res));
  }

  ScanResultCode GetError(const PeekResult& res) noexcept {
    return std::get<ScanResultCode>(res);
  }

  PeekResult Peek() noexcept {
    if (stream_.good()) {
      PeekValue v = stream_.peek();
      if (stream_.good()) {
        return v;
      }
    }

    if (stream_.eof()) {
      return ScanResultCode::kStreamEof;
    }

    if (stream_.bad()) {
      return ScanResultCode::kStreamBad;
    }

    if (stream_.fail()) {
      return ScanResultCode::kStreamFail;
    }

    return ScanResultCode::kStreamUnexpected;
  }

  void Consume(const Byte byte) noexcept {
    assert(stream_.good() && "Member `stream_` must be good before consuming.");
    PeekValue v = stream_.get();
    assert(stream_.good() && "Member `stream_` must be good after consuming.");
    assert(static_cast<Byte>(v) == byte &&
           "Consumed byte must be same as the peeked byte.");
    buffer_.emplace_back(byte);
  }

  std::istream& stream_;
  EncodedCodePoint& buffer_;
};

inline ScanResultCode Scan(std::istream& stream,
                           EncodedCodePoint& buffer) noexcept {
  return Scanner(stream, buffer).Scan();
}

}  // namespace utf8_scanner

#endif  // UTF8_SCANNER_UTF8_SCANNER_H_
