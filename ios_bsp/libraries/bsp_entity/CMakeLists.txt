project(bsp_entity
    LANGUAGES C
)

add_library(bsp_entity
    ${CMAKE_CURRENT_SOURCE_DIR}/entity.c
)
add_library(ios::bsp_entity ALIAS bsp_entity)

target_include_directories(bsp_entity
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bsp_entity ios::bsp libc)
