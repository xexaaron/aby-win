cmake_minimum_required(VERSION 3.28.3)

project(aby-win)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

add_library(${PROJECT_NAME} 
    src/log.cpp
    src/window.cpp
    src/backend/glfw/glfw-window.cpp
    include/aby-win/log.hpp
    include/aby-win/typedefs.hpp
    include/aby-win/common.hpp
    include/aby-win/window.hpp
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

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/WX /Zc:preprocessor>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)