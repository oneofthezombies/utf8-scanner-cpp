# UTF-8 Scanner C++

A lightweight, header-only UTF-8 scanner designed for lexers.

## Why I Created This?

Most UTF-8 libraries are designed for general-purpose text processing and include complex features such as encoding conversions (e.g., UTF-8 to UTF-16) and normalization. However, in a lexer, these features are unnecessary and can introduce unwanted overhead.

This library was created to provide a minimal, efficient way to scan UTF-8 byte streams without additional abstraction. It focuses solely on detecting valid UTF-8 sequences, returning their lengths, and handling errors efficiently. Unlike other libraries, it does not allocate memory unless explicitly needed by the lexer.
