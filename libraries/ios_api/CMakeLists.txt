project(ios_api
    LANGUAGES C ASM
)

add_library(ios_api
    ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.s
)
add_library(ios::api ALIAS ios_api)

target_compile_options(ios_api
    PRIVATE
        -x assembler-with-cpp
)

target_include_directories(ios_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
