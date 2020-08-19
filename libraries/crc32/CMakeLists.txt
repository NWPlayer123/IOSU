add_library(crc32
    ${CMAKE_CURRENT_SOURCE_DIR}/crc32.c
)

target_link_libraries(crc32 libc)

target_include_directories(crc32
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
