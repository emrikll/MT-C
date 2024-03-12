/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"



/*
 * GLOBALS
 */


/*
 * LED FUNCTIONS
 */

/**
 * @brief Configure the on-board LED.
 */
void setup_led() {

    //gpio_init(PICO_DEFAULT_LED_PIN);
    //gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    //led_off();
}

/**
 * @brief Set the on-board LED's state.
 */
void led_set(bool state) {

    //gpio_put(PICO_DEFAULT_LED_PIN, state);
}

/**
 * @brief Turn the on-board LED on.
 */
void led_on() {

    led_set(true);
}


/**
 * @brief Turn the on-board LED off.
 */
void led_off() {

    led_set(false);
}

/*
* GPIO
*/

void setup_gpio() {

    //gpio_init(SW_IRQ_PIN);
    //gpio_set_dir(SW_IRQ_PIN, GPIO_OUT);
    //gpio_put(SW_IRQ_PIN, 0);
}

/*
 * SETUP FUNCTIONS
 */

/**
 * @brief Umbrella hardware setup routine.
 */
void setup() {
}

/*
* DEBUG 
*/

/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
void log_debug(const char* msg) {

#ifdef DEBUG
    uint msg_length = 9 + strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "[DEBUG] %s\n", msg);
    printf("%s", sprintf_buffer);
    free(sprintf_buffer);
#endif
}


/**
 * @brief Show basic device info.
 */
void log_device_info(void) {

    //printf("App: %s %s (%i)\n", APP_NAME, APP_VERSION, BUILD_NUM);
}


/*
 * RUNTIME START
 */

int main() {
    // DEBUG
    #ifdef DEBUG
    stdio_init_all();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    #endif

    // Set up the hardware
    setup();
    
    // Log app info
    #ifdef DEBUG
    log_device_info();
    #endif

            
    // Start the FreeRTOS scheduler if any of the tasks are good
    if (NULL) {

        // Start the scheduler
        //vTaskStartScheduler();
    } else {
        // Flash board LED 5 times
        //uint8_t count = LED_ERROR_FLASHES;
        //while (count > 0) {
            //led_on();
            //vTaskDelay(100);
           // led_off();
            //vTaskDelay(100);
            //count--;
        //}
    }

    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}


