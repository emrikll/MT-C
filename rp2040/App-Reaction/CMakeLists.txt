cmake_minimum_required(VERSION 3.14)

# Make project data accessible to compiler
add_compile_definitions(APP_NAME="${APP_3_NAME}")
add_compile_definitions(APP_VERSION="${APP_3_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app source code file(s)
add_executable(${APP_3_NAME}
    ${APP_3_SRC_DIRECTORY}/main.c
)

# Link to built librariespico_stdlib
target_link_libraries(${APP_3_NAME} LINK_PUBLIC
    pico_stdlib
    pico_rand
    FreeRTOS)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_3_NAME} 1)
pico_enable_stdio_uart(${APP_3_NAME} 1)

# Enable extra build products
pico_add_extra_outputs(${APP_3_NAME})
