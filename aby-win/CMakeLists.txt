cmake_minimum_required(VERSION 3.28.3)

project(aby-win)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

option(ABY_WIN_ENABLE_LOG_INFO "enable info logging" ON)
option(ABY_WIN_ENABLE_LOG_TRACE "enable trace logging" ON)
option(ABY_WIN_ENABLE_LOG_WARN "enable warning logging" ON)
option(ABY_WIN_ENABLE_ASSERT "enable assertions. if turned off then only errors will be logged." ON)

add_library(${PROJECT_NAME} 
    src/window.cpp
    src/common.cpp
    src/backend/glfw/glfw-window.cpp
    src/backend/sdl/sdl-window.cpp
    include/aby-win/window.hpp
    include/aby-win/common.hpp
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/aby-win
    ${VENDOR_DIR}/glfw/include
    ${VENDOR_DIR}/SDL/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw SDL3::SDL3)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/WX /Zc:preprocessor>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    ABY_WIN_ENABLE_LOG_INFO=$<BOOL:${ABY_WIN_ENABLE_LOG_INFO}>
    ABY_WIN_ENABLE_LOG_TRACE=$<BOOL:${ABY_WIN_ENABLE_LOG_TRACE}>
    ABY_WIN_ENABLE_LOG_WARN=$<BOOL:${ABY_WIN_ENABLE_LOG_WARN}> 
    ABY_WIN_ENABLE_ASSERT=$<BOOL:${ABY_WIN_ENABLE_ASSERT}>
)