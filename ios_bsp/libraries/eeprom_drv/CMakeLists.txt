project(eeprom_drv
    LANGUAGES C
)

add_library(eeprom_drv
    ${CMAKE_CURRENT_SOURCE_DIR}/eeprom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_bits.c
)
add_library(ios::eeprom_drv ALIAS eeprom_drv)

target_include_directories(eeprom_drv
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eeprom_drv libc ios::api ios::bsp_entity)
