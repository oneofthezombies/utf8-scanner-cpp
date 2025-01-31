#include <utf8_scanner/utf8_scanner.h>

#include <iostream>

int main() {
  std::uint8_t byte = 0x80;
  std::cout << std::boolalpha << utf8_scanner::IsAscii(byte) << std::endl;
  std::cout << std::boolalpha << utf8_scanner::IsAscii2(byte) << std::endl;
  return 0;
}
