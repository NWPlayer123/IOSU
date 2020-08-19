add_library(bsp_api
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp.c
)
add_library(ios::bsp ALIAS bsp_api)

target_link_libraries(bsp_api ios::api)

target_include_directories(bsp_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
