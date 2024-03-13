# cmake
CMake files for the lab project.

## Dependencies
* `cmake`
* gcc-arm-none-eabi:
    * `arm-none-eabi-gcc`
    * `arm-none-eabi-as`
    * `arm-none-eabi-objcopy`

## Usage
Put [CMakeLists.txt](CMakeLists.txt) and [toolchain.cmake](toolchain.cmake) in
the project folder (where `application.c` and `TinyTimber.c` are located). Run
`cmake -S . -B build` from there. Henceforth, run `cmake --build build` to build
the project.

It is assumed that the compiler directory is on the `PATH` environment variable;
if not, edit the `CMAKE_C_COMPILER` setting at [line 6 of
toolchain.cmake](toolchain.cmake#L6), specifying the full path to the
`arm-none-eabi-gcc` compiler (e.g.
`/home/USER/Downloads/gcc-arm-none-eabi-VERSION/bin/arm-none-eabi-gcc`).

If you create additional `.c` files, they need to be added to the list of
sources starting at [line 16 of CMakeLists.txt](CMakeLists.txt#L16).
