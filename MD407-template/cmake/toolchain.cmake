set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER
    arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX}
    CACHE INTERNAL "")

set(CMAKE_C_FLAGS
    "-g \
    -O0 \
    -Wall \
    -mthumb \
    -mcpu=cortex-m4 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -fverbose-asm -DSTM32F40_41xxx"
    CACHE INTERNAL "")
set(CMAKE_EXE_LINKER_FLAGS
    "--specs=nano.specs \
    -nostartfiles \
    -T ${PROJECT_SOURCE_DIR}/md407-ram.x \
    -Wl,-Map=${PROJECT_SOURCE_DIR}/Debug/${PROJECT_NAME}.map,--cref"
    CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
