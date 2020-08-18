cmake_minimum_required(VERSION 3.2)

add_executable(ios-bsp
    bsp_main.c
    bsp_hwrevs.c
)

target_include_directories(ios-bsp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(ios-bsp
    PRIVATE
        -DACCURATE
)

add_subdirectory(libraries/bsp_entity)
add_subdirectory(libraries/bc)
add_subdirectory(libraries/eeprom_drv)

target_link_libraries(ios-bsp libc ios::log ios::eeprom_drv ios::bc ios::bsp_entity)
