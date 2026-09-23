# README - aby-win

## Table of Contents

- [Dependency Requirements](#dependency-requirements)
- [Platforms & Backends](#platforms--backends)
- [Cloning](#cloning)
- [Building](#building)
  - [Options](#options)
  - [CMake](#cmake)

## Dependency Requirements

| Dependency | Version | Note |
| ---------- | ------- | ---- |
| [![CMake](https://img.shields.io/badge/CMake-064F8C?logo=cmake&logoColor=fff)](#) | ![Version](https://img.shields.io/badge/version-%3E=_3.28.3-blue) | Building the project |

## Platforms & Backends

| Platform                                                                                                     | Status                                                            | Window Backend                                                                    | Status                                                         |
| ------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------- | --------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| [![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11\&logoColor=white)](#) | ![Platform](https://img.shields.io/badge/platform-Passing-green)  | [![SDL](https://img.shields.io/badge/SDL-000000?logo=sdl\&logoColor=white)](#)    | ![Backend](https://img.shields.io/badge/backend-Passing-green) |
| [![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux\&logoColor=black)](#)                         | ![Platform](https://img.shields.io/badge/platform-Passing-green)  | [![GLFW](https://img.shields.io/badge/GLFW-000000?logo=glfw\&logoColor=white)](#) | ![Backend](https://img.shields.io/badge/backend-Passing-green) |
| [![macOS](https://img.shields.io/badge/macOS-000000?logo=apple\&logoColor=F0F0F0)](#)                        | ![Platform](https://img.shields.io/badge/platform-Unknown-yellow) | [![Qt](https://img.shields.io/badge/Qt-41CD52?logo=qt\&logoColor=white)](#)       | ![Backend](https://img.shields.io/badge/backend-Passing-green) |

## Cloning

```bash
git clone --depth=1 --recurse-submodules https://github.com/xexaaron/aby-win <path>
cd <path>
```

Or as a submodule:

```bash
git submodule add --depth=1 https://github.com/xexaaron/aby-win <path>
git submodule update --init --recursive
```
## Building

### Options

| Option                     | Description                                                       | Default                                               |
| -------------------------- | ----------------------------------------------------------------- | ----------------------------------------------------- |
| `ABY_WIN_ENABLE_LOG_INFO`  | enable info logging                                               | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_LOG_TRACE` | enable trace logging                                              | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_LOG_WARN`  | enable warning logging                                            | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_ASSERT`    | enable assertions. if turned off then only errors will be logged. | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_SDL`       | enable SDL backend                                                | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_GLFW`      | enable GLFW backend                                               | ![value](https://img.shields.io/badge/value-ON-green) |
| `ABY_WIN_ENABLE_QT`        | enable Qt backend                                                 | ![value](https://img.shields.io/badge/value-ON-green) |

### CMake

```bash
cmake -S . -B bin -DCMAKE_BUILD_TYPE=<Debug|Release>
cmake --build bin --config <debug|release>
```

Or, in your CMakeLists.txt file:

```cmake
add_subdirectory(path/to/aby-win)

target_link_libraries(${YOUR_PROJECT_NAME} PRIVATE aby-win::aby-win)
```

---

[Scroll to top](#readme---aby-win)
