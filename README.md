# lexy-vdf
Repo for the lexy-vdf, a [VDF](https://developer.valvesoftware.com/wiki/KeyValues) parser implemented using [lexy](https://lexy.foonathan.net/)

## Example Usage
See [main.cpp](src/headless/main.cpp)

## Required
* [CMake](https://cmake.org/) 3.28+
* [Ninja](https://ninja-build.org/)

## Build Instructions
1. Pick a configure preset from `CMakePresets.json` (`windows-x64-md`, `windows-x64-mt`, `linux-x64`, `macos-universal`).
2. Run `cmake --preset <preset>` in the project root (dependencies are fetched automatically; no submodules needed).
3. Run `cmake --build --preset <preset>-debug` (or `<preset>-release`). The static library and the headless executable land in `out/build/<preset>/bin/<Config>/`.

The headless executable is built by default in standalone builds; disable with `-DLEXY_VDF_BUILD_HEADLESS=OFF`.

## Link Instructions
Use CMake: `add_subdirectory(lexy-vdf)` and link against `openvic::lexy-vdf`.
