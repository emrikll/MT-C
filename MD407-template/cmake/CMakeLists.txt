cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake)
project(
  RTS-Lab
  VERSION 1.0
  LANGUAGES C ASM)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Debug)

add_custom_target(
  Debug ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  VERBATIM)

add_executable(
  ${PROJECT_NAME}.elf
  dispatch.s
  driver/src/stm32f4xx_can.c
  driver/src/stm32f4xx_dac.c
  driver/src/stm32f4xx_exti.c
  driver/src/stm32f4xx_gpio.c
  driver/src/stm32f4xx_rcc.c
  driver/src/stm32f4xx_syscfg.c
  driver/src/stm32f4xx_tim.c
  driver/src/stm32f4xx_usart.c
  startup.c
  TinyTimber.c
  sciTinyTimber.c
  canTinyTimber.c
  application.c)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE m)

target_include_directories(
  ${PROJECT_NAME}.elf
  PRIVATE ${PROJECT_SOURCE_DIR}/driver/inc
  PRIVATE ${PROJECT_SOURCE_DIR}/device/inc)

add_custom_command(
  TARGET ${PROJECT_NAME}.elf
  POST_BUILD
  COMMAND
    ${CMAKE_OBJCOPY} ARGS -S -O srec
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.s19
  VERBATIM)
