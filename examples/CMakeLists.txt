add_executable(example example.cc)
target_link_libraries(example PRIVATE utf8_scanner::utf8_scanner)
