add_library(bsp_api
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp.c
)
add_library(ios::bsp ALIAS bsp_api)

target_compile_options(bsp_api
    PRIVATE
        -x assembler-with-cpp
)

target_include_directories(bsp_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
