cmake_minimum_required(VERSION 3.28.3)

project(aby-win)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

option(ABY_WIN_ENABLE_LOG_INFO  "enable info logging" ON)
option(ABY_WIN_ENABLE_LOG_TRACE "enable trace logging" ON)
option(ABY_WIN_ENABLE_LOG_WARN  "enable warning logging" ON)
option(ABY_WIN_ENABLE_ASSERT    "enable assertions. if turned off then only errors will be logged." ON)

option(ABY_WIN_ENABLE_SDL  "enable SDL backend" ON)
option(ABY_WIN_ENABLE_GLFW "enable GLFW backend" ON)
option(ABY_WIN_ENABLE_QT   "enable Qt backend" ON)


add_library(${PROJECT_NAME}
    src/window.cpp
    src/common.cpp
    src/monitor.cpp

    include/aby-win/monitor.hpp
    include/aby-win/window.hpp
    include/aby-win/common.hpp

    $<$<BOOL:${ABY_WIN_ENABLE_QT}>:
        include/aby-win/backend/qt/qt-window.hpp
    >
    $<$<BOOL:${ABY_WIN_ENABLE_QT}>:
        src/backend/qt/qt-window.cpp
    >

    $<$<BOOL:${ABY_WIN_ENABLE_SDL}>:
        include/aby-win/backend/sdl/sdl-window.hpp
    >
    $<$<BOOL:${ABY_WIN_ENABLE_SDL}>:
        src/backend/sdl/sdl-window.cpp
    >

    $<$<BOOL:${ABY_WIN_ENABLE_GLFW}>:
        include/aby-win/backend/glfw/glfw-window.hpp
    >
    $<$<BOOL:${ABY_WIN_ENABLE_GLFW}>:
        src/backend/glfw/glfw-window.cpp
    >
)


if (ABY_WIN_ENABLE_QT)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
endif()


add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/aby-win

PRIVATE
    $<$<BOOL:${ABY_WIN_ENABLE_GLFW}>:${VENDOR_DIR}/glfw/include>
    $<$<BOOL:${ABY_WIN_ENABLE_SDL}>:${VENDOR_DIR}/SDL/include>
)


target_link_libraries(${PROJECT_NAME}
PRIVATE
    $<$<BOOL:${ABY_WIN_ENABLE_GLFW}>:glfw>
    $<$<BOOL:${ABY_WIN_ENABLE_SDL}>:SDL3::SDL3>
    $<$<BOOL:${ABY_WIN_ENABLE_QT}>:Qt6::Widgets>

    $<$<PLATFORM_ID:Linux>:wayland-client>
    $<$<PLATFORM_ID:Linux>:X11>
)


target_compile_options(${PROJECT_NAME}
PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/WX /Zc:preprocessor>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
)


target_compile_definitions(${PROJECT_NAME}
PUBLIC
    ABY_WIN_ENABLE_LOG_INFO=$<BOOL:${ABY_WIN_ENABLE_LOG_INFO}>
    ABY_WIN_ENABLE_LOG_TRACE=$<BOOL:${ABY_WIN_ENABLE_LOG_TRACE}>
    ABY_WIN_ENABLE_LOG_WARN=$<BOOL:${ABY_WIN_ENABLE_LOG_WARN}>
    ABY_WIN_ENABLE_ASSERT=$<BOOL:${ABY_WIN_ENABLE_ASSERT}>
    ABY_WIN_ENABLE_SDL=$<BOOL:${ABY_WIN_ENABLE_SDL}>
    ABY_WIN_ENABLE_GLFW=$<BOOL:${ABY_WIN_ENABLE_GLFW}>
    ABY_WIN_ENABLE_QT=$<BOOL:${ABY_WIN_ENABLE_QT}>
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)