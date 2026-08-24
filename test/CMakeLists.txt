cmake_minimum_required(VERSION 3.28.3)

project(aby-win-test)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

add_executable(${PROJECT_NAME} 
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE aby-win::aby-win)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/WX /Zc:preprocessor>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)