# lexy-vdf
Repo for the lexy-vdf, a [VDF](https://developer.valvesoftware.com/wiki/KeyValues) parser implemented using [lexy](https://lexy.foonathan.net/)

## Example Usage
See [main.cpp](src/headless/main.cpp)

## Required
* [scons](https://scons.org/)

## Build Instructions
1. Install [scons](https://scons.org/) for your system.
2. Run the command `git submodule update --init --recursive` to retrieve all related submodules.
3. Run `scons build_lvdf_library=yes` in the project root, you should see a liblexy-vdf file in `bin`.

## Link Instructions
1. Call `lvdf_env = SConscript("lexy-vdf/SConstruct")`
2. Use the values stored in the `lvdf_env.lexy_vdf` to link and compile against:

| Variable Name | Description                               | Correlated ENV variable   |
| ---           | ---                                       | ---                       |
| `LIBPATH`     | Library path list                         | `env["LIBPATH"]`          |
| `LIBS`        | Library files names in the library paths  | `env["LIBS"]`             |
| `INCPATH`     | Library include files                     | `env["CPPPATH"]`          |
