cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake)
project(
    MD407-REACTION
    VERSION 1.0
    LANGUAGES C ASM)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Debug)
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/libraries/FreeRTOS-Kernel") 


add_custom_target(
  Debug ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  VERBATIM)

add_executable(
    ${PROJECT_NAME}.elf
    vectors.s
    device/src/system_stm32f4xx.c
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM4F/port.c
    libraries/io/io.c
    driver/src/stm32f4xx_can.c
    driver/src/stm32f4xx_dac.c
    driver/src/stm32f4xx_exti.c
    driver/src/stm32f4xx_gpio.c
    driver/src/stm32f4xx_rcc.c
    driver/src/stm32f4xx_syscfg.c
    driver/src/stm32f4xx_tim.c
    driver/src/stm32f4xx_usart.c
    driver/src/stm32f4xx_rng.c
    driver/src/misc.c
    libraries/utils/timer.c
    startup.c
    src/main.c)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE m)

target_include_directories(
    ${PROJECT_NAME}.elf
    PRIVATE ${PROJECT_SOURCE_DIR}/driver/inc
    PRIVATE ${PROJECT_SOURCE_DIR}/device/inc
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/libraries/io
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${PROJECT_SOURCE_DIR}/libraries/utils
    PRIVATE ${FREERTOS_CFG_DIRECTORY}/
    PRIVATE ${FREERTOS_SRC_DIRECTORY}/include
    PRIVATE ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM4F
  )

# Add FreeRTOS as library
#add_library(FreeRTOS STATIC
#)

# Build FreeRTOS
#target_include_directories(FreeRTOS PUBLIC
#)

add_custom_command(
  TARGET ${PROJECT_NAME}.elf
  POST_BUILD
  COMMAND
    ${CMAKE_OBJCOPY} ARGS -S -O srec
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.s19
  VERBATIM)

