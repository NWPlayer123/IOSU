project(bc
    LANGUAGES C
)

add_library(bc
    ${CMAKE_CURRENT_SOURCE_DIR}/bc.c
)
add_library(ios::bc ALIAS bc)

target_include_directories(bc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bc libc log ios::eeprom_drv)
