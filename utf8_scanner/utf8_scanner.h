#ifndef UTF8_SCANNER_UTF8_SCANNER_H_
#define UTF8_SCANNER_UTF8_SCANNER_H_

#include <cassert>
#include <cstdint>
#include <istream>
#include <optional>
#include <variant>
#include <vector>

namespace utf8_scanner {

// Range: 0x00 ~ 0x7f
constexpr bool IsAscii(const char8_t b) noexcept {
  return (b & 0b10000000) == 0b00000000;
}

// Range: 0xc2 ~ 0xdf
// 0xc0, 0xc1 are disallowed in UTF-8.
constexpr bool IsStartOf2Byte(const char8_t b) noexcept {
  return (b & 0b11100000) == 0b11000000 && 0xc2 <= b;
}

// Range: 0xe0 ~ 0xef
constexpr bool IsStartOf3Byte(const char8_t b) noexcept {
  return (b & 0b11110000) == 0b11100000;
}

// Range: 0xf0 ~ 0xf4
// 0xf5 ~ 0xff are disallowed in UTF-8.
constexpr bool IsStartOf4Byte(const char8_t b) noexcept {
  return (b & 0b11111000) == 0b11110000 && b <= 0xf4;
}

// Range: 0x80 ~ 0xbf
// A continuation byte at the start of a character is invalid.
// A non-continuation byte before the end of a character is invalid.
constexpr bool IsContinuation(const char8_t b) noexcept {
  return (b & 0b11000000) == 0b10000000;
}

constexpr bool IsOverlong3Byte(const char8_t b0, const char8_t b1) noexcept {
  return b0 == 0xe0 && b1 < 0xa0;
}

constexpr bool IsUtf16Surrogate(const char8_t b0, const char8_t b1) noexcept {
  return b0 == 0xed && 0xa0 <= b1;
}

constexpr bool IsOverlong4Byte(const char8_t b0, const char8_t b1) noexcept {
  return b0 == 0xf0 && b1 < 0x90;
}

constexpr bool IsOutOfUnicodeRange(const char8_t b0,
                                   const char8_t b1) noexcept {
  return b0 == 0xf4 && 0x90 <= b1;
}

constexpr char32_t ToUtf32Unsafe(const char8_t byte) noexcept { return byte; }

constexpr char32_t ToUtf32Unsafe(const char8_t b0, const char8_t b1) noexcept {
  return (b0 & 0b00011111) << 6 | (b1 & 0b00111111);
}

constexpr char32_t ToUtf32Unsafe(const char8_t b0, const char8_t b1,
                                 const char8_t b2) noexcept {
  return (b0 & 0b00001111) << 12 | (b1 & 0b00111111) << 6 | (b2 & 0b00111111);
}

constexpr char32_t ToUtf32Unsafe(const char8_t b0, const char8_t b1,
                                 const char8_t b2, const char8_t b3) noexcept {
  return (b0 & 0b00000111) << 18 | (b1 & 0b00111111) << 12 |
         (b2 & 0b00111111) << 6 | (b3 & 0b00111111);
}

constexpr std::uint8_t Utf8CharLength(const char8_t first_byte) noexcept {
  if (IsAscii(first_byte)) {
    return 1;
  }

  if (IsStartOf2Byte(first_byte)) {
    return 2;
  }

  if (IsStartOf3Byte(first_byte)) {
    return 3;
  }

  if (IsStartOf4Byte(first_byte)) {
    return 4;
  }

  return 0;
}

constexpr char32_t ToUtf32(const char8_t byte) noexcept {
  assert(IsAscii(byte) && "Parameter `byte` must be ASCII.");

  return ToUtf32Unsafe(byte);
}

constexpr std::optional<char32_t> TryToUtf32(const char8_t byte) noexcept {
  if (IsAscii(byte) == false) {
    return std::nullopt;
  }

  return ToUtf32Unsafe(byte);
}

constexpr char32_t ToUtf32(const char8_t b0, const char8_t b1) noexcept {
  assert(IsStartOf2Byte(b0) &&
         "Parameter `b0` must be the start of a 2-byte UTF-8 character.");
  assert(IsContinuation(b1) && "Parameter `b1` must be a continuation byte.");

  return ToUtf32Unsafe(b0, b1);
}

constexpr std::optional<char32_t> TryToUtf32(const char8_t b0,
                                             const char8_t b1) noexcept {
  if (IsStartOf2Byte(b0) == false) {
    return std::nullopt;
  }

  if (IsContinuation(b1) == false) {
    return std::nullopt;
  }

  return ToUtf32Unsafe(b0, b1);
}

constexpr char32_t ToUtf32(const char8_t b0, const char8_t b1,
                           const char8_t b2) noexcept {
  assert(IsStartOf3Byte(b0) &&
         "Parameter `b0` must be the start of a 3-byte UTF-8 character.");
  assert(!IsOverlong3Byte(b0, b1) &&
         "Parameter `b0` and `b1` must not form an overlong 3-byte UTF-8 "
         "character.");
  assert(!IsUtf16Surrogate(b0, b1) &&
         "Parameter `b0` and `b1` must not form a UTF-16 surrogate.");
  assert(IsContinuation(b1) && "Parameter `b1` must be a continuation byte.");
  assert(IsContinuation(b2) && "Parameter `b2` must be a continuation byte.");

  return ToUtf32Unsafe(b0, b1, b2);
}

constexpr std::optional<char32_t> TryToUtf32(const char8_t b0, const char8_t b1,
                                             const char8_t b2) noexcept {
  if (IsStartOf3Byte(b0) == false) {
    return std::nullopt;
  }

  if (IsOverlong3Byte(b0, b1)) {
    return std::nullopt;
  }

  if (IsUtf16Surrogate(b0, b1)) {
    return std::nullopt;
  }

  if (IsContinuation(b1) == false) {
    return std::nullopt;
  }

  if (IsContinuation(b2) == false) {
    return std::nullopt;
  }

  return ToUtf32Unsafe(b0, b1, b2);
}

constexpr char32_t ToUtf32(const char8_t b0, const char8_t b1, const char8_t b2,
                           const char8_t b3) noexcept {
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

  return ToUtf32Unsafe(b0, b1, b2, b3);
}

constexpr std::optional<char32_t> TryToUtf32(const char8_t b0, const char8_t b1,
                                             const char8_t b2,
                                             const char8_t b3) noexcept {
  if (IsStartOf4Byte(b0) == false) {
    return std::nullopt;
  }

  if (IsOverlong4Byte(b0, b1)) {
    return std::nullopt;
  }

  if (IsOutOfUnicodeRange(b0, b1)) {
    return std::nullopt;
  }

  if (IsContinuation(b1) == false) {
    return std::nullopt;
  }

  if (IsContinuation(b2) == false) {
    return std::nullopt;
  }

  if (IsContinuation(b3) == false) {
    return std::nullopt;
  }

  return ToUtf32Unsafe(b0, b1, b2, b3);
}

constexpr char32_t ToUtf32(const char8_t* utf8_char,
                           const std::size_t utf8_char_length) noexcept {
  assert(utf8_char != nullptr && "Parameter `utf8_char` must not be nullptr.");
  assert((0 < utf8_char_length && utf8_char_length <= 4) &&
         "Parameter `utf8_char_length` must be greater than 0 and less than or "
         "4.");

  switch (utf8_char_length) {
    case 1:
      return ToUtf32(utf8_char[0]);
    case 2:
      return ToUtf32(utf8_char[0], utf8_char[1]);
    case 3:
      return ToUtf32(utf8_char[0], utf8_char[1], utf8_char[2]);
    case 4:
      return ToUtf32(utf8_char[0], utf8_char[1], utf8_char[2], utf8_char[3]);
    default:
      assert(false && "Parameter `utf8_char_length` must be 1, 2, 3, or 4.");
      return 0;
  }
}

constexpr std::optional<char32_t> TryToUtf32(
    const char8_t* utf8_char, const std::size_t utf8_char_length) noexcept {
  if (utf8_char == nullptr) {
    return std::nullopt;
  }

  if ((0 < utf8_char_length && utf8_char_length <= 4) == false) {
    return std::nullopt;
  }

  switch (utf8_char_length) {
    case 1:
      return TryToUtf32(utf8_char[0]);
    case 2:
      return TryToUtf32(utf8_char[0], utf8_char[1]);
    case 3:
      return TryToUtf32(utf8_char[0], utf8_char[1], utf8_char[2]);
    case 4:
      return TryToUtf32(utf8_char[0], utf8_char[1], utf8_char[2], utf8_char[3]);
    default:
      return std::nullopt;
  }
}

enum class ResultCode : std::uint8_t {
  kValidCharacter,
  kDisallowedStartByte,
  kStartWithContinuationByte,
  kIncomplete2Byte,
  kIncomplete3Byte,
  kIncomplete4Byte,
  kOverlong3Byte,
  kOverlong4Byte,
  kUtf16Surrogate,
  kOutOfUnicodeRange,
  kProviderEof,
  kStreamBad,
  kStreamFail,
  kStreamUnexpected,
};

template <typename T>
concept ResultCodeable = requires(T t) {
  { ToResultCode(t) } -> std::same_as<ResultCode>;
};

template <typename T, typename E>
concept Provider = requires(T t) {
  { t.Peek() } -> std::same_as<std::variant<char8_t, E>>;
  { t.Advance() } -> std::same_as<void>;
} && ResultCodeable<E>;

template <typename T>
concept Handler = requires(T t) {
  { t.Clear() } -> std::same_as<void>;
  { t.Handle(char8_t{}) } -> std::same_as<void>;
};

template <ResultCodeable ProviderError, Provider<ProviderError> Provider,
          Handler Handler>
class Scanner {
 public:
  using ProviderResult = std::variant<char8_t, ProviderError>;

  explicit Scanner(Provider& provider, Handler& handler) noexcept
      : provider_(provider), handler_(handler) {}

  ~Scanner() noexcept = default;

  Scanner(const Scanner&) = delete;
  Scanner& operator=(const Scanner&) = delete;

  Scanner(Scanner&&) = delete;
  Scanner& operator=(Scanner&&) = delete;

  ResultCode Scan() noexcept {
    const ProviderResult maybe_b0 = provider_.Peek();
    if (maybe_b0.index() != 0) {
      return ToResultCode(std::get<1>(maybe_b0));
    }

    const char8_t b0 = std::get<0>(maybe_b0);
    Consume(b0);

    if (IsAscii(b0)) {
      return ResultCode::kValidCharacter;
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
      return ResultCode::kStartWithContinuationByte;
    }

    return ResultCode::kDisallowedStartByte;
  }

 private:
  ResultCode OnStartOf2Byte(const char8_t b0) noexcept {
    const ProviderResult maybe_b1 = provider_.Peek();
    if (maybe_b1.index() != 0) {
      return ToResultCode(std::get<1>(maybe_b1));
    }

    const char8_t b1 = std::get<0>(maybe_b1);
    if (IsContinuation(b1)) {
      Consume(b1);
      return ResultCode::kValidCharacter;
    }

    return ResultCode::kIncomplete2Byte;
  }

  ResultCode OnStartOf3Byte(const char8_t b0) noexcept {
    const ProviderResult maybe_b1 = provider_.Peek();
    if (maybe_b1.index() != 0) {
      return ToResultCode(std::get<1>(maybe_b1));
    }

    const char8_t b1 = std::get<0>(maybe_b1);
    if (IsOverlong3Byte(b0, b1)) {
      Consume(b1);
      return ResultCode::kOverlong3Byte;
    }

    if (IsUtf16Surrogate(b0, b1)) {
      Consume(b1);
      return ResultCode::kUtf16Surrogate;
    }

    if (IsContinuation(b1)) {
      Consume(b1);

      const ProviderResult maybe_b2 = provider_.Peek();
      if (maybe_b2.index() != 0) {
        return ToResultCode(std::get<1>(maybe_b2));
      }

      const char8_t b2 = std::get<0>(maybe_b2);
      if (IsContinuation(b2)) {
        Consume(b2);
        return ResultCode::kValidCharacter;
      }
    }

    return ResultCode::kIncomplete3Byte;
  }

  ResultCode OnStartOf4Byte(const char8_t b0) noexcept {
    const ProviderResult maybe_b1 = provider_.Peek();
    if (maybe_b1.index() != 0) {
      return ToResultCode(std::get<1>(maybe_b1));
    }

    const char8_t b1 = std::get<0>(maybe_b1);
    if (IsOverlong4Byte(b0, b1)) {
      Consume(b1);
      return ResultCode::kOverlong4Byte;
    }

    if (IsOutOfUnicodeRange(b0, b1)) {
      Consume(b1);
      return ResultCode::kOutOfUnicodeRange;
    }

    if (IsContinuation(b1)) {
      Consume(b1);

      const ProviderResult maybe_b2 = provider_.Peek();
      if (maybe_b2.index() != 0) {
        return ToResultCode(std::get<1>(maybe_b2));
      }

      const char8_t b2 = std::get<0>(maybe_b2);
      if (IsContinuation(b2)) {
        Consume(b2);

        const ProviderResult maybe_b3 = provider_.Peek();
        if (maybe_b3.index() != 0) {
          return ToResultCode(std::get<1>(maybe_b3));
        }

        const char8_t b3 = std::get<0>(maybe_b3);
        if (IsContinuation(b3)) {
          Consume(b3);
          return ResultCode::kValidCharacter;
        }
      }
    }

    return ResultCode::kIncomplete4Byte;
  }

  void Consume(const char8_t byte) noexcept {
    handler_.Handle(byte);
    provider_.Advance();
  }

  Provider& provider_;
  Handler& handler_;
};

enum class StreamError : std::uint8_t {
  kUnexpected,
  kEof,
  kBad,
  kFail,
};

constexpr ResultCode ToResultCode(const StreamError error) noexcept {
  switch (error) {
    case StreamError::kUnexpected:
      return ResultCode::kStreamUnexpected;
    case StreamError::kEof:
      return ResultCode::kProviderEof;
    case StreamError::kBad:
      return ResultCode::kStreamBad;
    case StreamError::kFail:
      return ResultCode::kStreamFail;
  }
}

static_assert(ResultCodeable<StreamError>,
              "StreamError must be a ResultCodeable.");

class StreamProvider {
 public:
  explicit StreamProvider(std::istream& stream) noexcept : stream_(stream) {}

  ~StreamProvider() noexcept = default;

  StreamProvider(const StreamProvider&) = delete;
  StreamProvider& operator=(const StreamProvider&) = delete;

  StreamProvider(StreamProvider&&) = delete;
  StreamProvider& operator=(StreamProvider&&) = delete;

  std::variant<char8_t, StreamError> Peek() noexcept {
    if (stream_.good()) {
      std::istream::int_type v = stream_.peek();
      if (stream_.good()) {
        return static_cast<char8_t>(v);
      }
    }

    if (stream_.eof()) {
      return StreamError::kEof;
    }

    if (stream_.bad()) {
      return StreamError::kBad;
    }

    if (stream_.fail()) {
      return StreamError::kFail;
    }

    return StreamError::kUnexpected;
  }

  void Advance() noexcept {
    assert(stream_.good() && "Member `stream_` must be good before advancing.");
    static_cast<void>(stream_.get());
    assert(stream_.good() && "Member `stream_` must be good after advancing.");
  }

 private:
  std::istream& stream_;
};

static_assert(Provider<StreamProvider, StreamError>,
              "StreamProvider must be a Provider.");

class BufferHandler {
 public:
  BufferHandler() noexcept : buffer_(4) {}

  ~BufferHandler() noexcept = default;

  BufferHandler(const BufferHandler&) = delete;
  BufferHandler& operator=(const BufferHandler&) = delete;

  BufferHandler(BufferHandler&&) = delete;
  BufferHandler& operator=(BufferHandler&&) = delete;

  void Clear() noexcept { buffer_.clear(); }

  void Handle(const char8_t byte) noexcept { buffer_.emplace_back(byte); }

  const std::vector<char8_t>& Bytes() const noexcept { return buffer_; }

  std::optional<char32_t> TryToUtf32() const noexcept {
    return utf8_scanner::TryToUtf32(buffer_.data(), buffer_.size());
  }

 private:
  std::vector<char8_t> buffer_;
};

static_assert(Handler<BufferHandler>, "BufferHandler must be a Handler.");

inline ResultCode ScanUtf8(std::istream& stream,
                           BufferHandler& buffer_handler) noexcept {
  auto provider = StreamProvider(stream);
  return Scanner<StreamError, StreamProvider, BufferHandler>(provider,
                                                             buffer_handler)
      .Scan();
}

}  // namespace utf8_scanner

#endif  // UTF8_SCANNER_UTF8_SCANNER_H_
