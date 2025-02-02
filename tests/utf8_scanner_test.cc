#include "utf8_scanner/utf8_scanner.h"

#include <gtest/gtest.h>

#include <sstream>

#include "gtest/gtest.h"

namespace utf8_scanner {

TEST(all_tests, Basic) {
  EXPECT_TRUE(true);
  // std::istringstream stream(std::string_view("Hello, world!"));
}

// TEST(IsAscii, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (0x00 <= b && b <= 0x7f) {
//       EXPECT_TRUE(IsAscii(b));
//     } else {
//       EXPECT_FALSE(IsAscii(b));
//     }
//   }
// }

// TEST(IsStartOf2Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (0xc2 <= b && b <= 0xdf) {
//       EXPECT_TRUE(IsStartOf2Byte(b));
//     } else {
//       EXPECT_FALSE(IsStartOf2Byte(b));
//     }
//   }
// }

// TEST(IsStartOf3Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (0xe0 <= b && b <= 0xef) {
//       EXPECT_TRUE(IsStartOf3Byte(b));
//     } else {
//       EXPECT_FALSE(IsStartOf3Byte(b));
//     }
//   }
// }

// TEST(IsStartOf4Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (0xf0 <= b && b <= 0xf4) {
//       EXPECT_TRUE(IsStartOf4Byte(b));
//     } else {
//       EXPECT_FALSE(IsStartOf4Byte(b));
//     }
//   }
// }

// TEST(IsContinuation, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (0x80 <= b && b <= 0xbf) {
//       EXPECT_TRUE(IsContinuation(b));
//     } else {
//       EXPECT_FALSE(IsContinuation(b));
//     }
//   }
// }

// TEST(IsOverlong3Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       if (b0 == 0xe0 && b1 < 0xa0) {
//         EXPECT_TRUE(IsOverlong3Byte(b0, b1));
//       } else {
//         EXPECT_FALSE(IsOverlong3Byte(b0, b1));
//       }
//     }
//   }
// }

// TEST(IsUtf16Surrogate, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       if (b0 == 0xed && 0xa0 <= b1) {
//         EXPECT_TRUE(IsUtf16Surrogate(b0, b1));
//       } else {
//         EXPECT_FALSE(IsUtf16Surrogate(b0, b1));
//       }
//     }
//   }
// }

// TEST(IsOverlong4Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       if (b0 == 0xf0 && b1 < 0x90) {
//         EXPECT_TRUE(IsOverlong4Byte(b0, b1));
//       } else {
//         EXPECT_FALSE(IsOverlong4Byte(b0, b1));
//       }
//     }
//   }
// }

// TEST(IsOutOfUnicodeRange, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       if (b0 == 0xf4 && 0x90 <= b1) {
//         EXPECT_TRUE(IsOutOfUnicodeRange(b0, b1));
//       } else {
//         EXPECT_FALSE(IsOutOfUnicodeRange(b0, b1));
//       }
//     }
//   }
// }

// TEST(CodePointFromAscii, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b = static_cast<Byte>(i);
//     if (IsAscii(b)) {
//       EXPECT_EQ(CodePointFromAscii(b), b);
//     }
//   }
// }

// TEST(CodePointFrom2Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       if (IsStartOf2Byte(b0) && IsContinuation(b1)) {
//         EXPECT_EQ(CodePointFrom2Byte(b0, b1),
//                   (b0 & 0b00011111) << 6 | (b1 & 0b00111111));
//       }
//     }
//   }
// }

// TEST(CodePointFrom3Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       for (int k = 0x00; k <= 0xff; ++k) {
//         const Byte b2 = static_cast<Byte>(k);
//         if (IsStartOf3Byte(b0) && !IsOverlong3Byte(b0, b1) &&
//             !IsUtf16Surrogate(b0, b1) && IsContinuation(b1) &&
//             IsContinuation(b2)) {
//           EXPECT_EQ(CodePointFrom3Byte(b0, b1, b2), (b0 & 0b00001111) << 12 |
//                                                         (b1 & 0b00111111) <<
//                                                         6 | (b2 &
//                                                         0b00111111));
//         }
//       }
//     }
//   }
// }

// TEST(CodePointFrom4Byte, Basic) {
//   for (int i = 0x00; i <= 0xff; ++i) {
//     const Byte b0 = static_cast<Byte>(i);
//     for (int j = 0x00; j <= 0xff; ++j) {
//       const Byte b1 = static_cast<Byte>(j);
//       for (int k = 0x00; k <= 0xff; ++k) {
//         const Byte b2 = static_cast<Byte>(k);
//         for (int l = 0x00; l <= 0xff; ++l) {
//           const Byte b3 = static_cast<Byte>(l);
//           if (IsStartOf4Byte(b0) && !IsOverlong4Byte(b0, b1) &&
//               !IsOutOfUnicodeRange(b0, b1) && IsContinuation(b1) &&
//               IsContinuation(b2) && IsContinuation(b3)) {
//             EXPECT_EQ(CodePointFrom4Byte(b0, b1, b2, b3),
//                       (b0 & 0b00000111) << 18 | (b1 & 0b00111111) << 12 |
//                           (b2 & 0b00111111) << 6 | (b3 & 0b00111111));
//           }
//         }
//       }
//     }
//   }
// }

}  // namespace utf8_scanner
