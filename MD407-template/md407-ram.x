/*-----------------------------------------------------------------------------
* Author: Boris Vinogradov(nis) : no111u3@gmail.com
* File: stm32f4xxxg_flash.ld
* Type: linker script and map
* Project: STM32F4D USB OTG
* 2013
*-----------------------------------------------------------------------------*/
/* Entry Point */
ENTRY(reset_handler)
/* Specify the memory areas */
MEMORY {
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 112K
    VECTOR (rwx) : ORIGIN = 0x200C1000, LENGTH = 2048
}
/* Define output sections */
SECTIONS {
    . = ORIGIN(FLASH);
    .text :
    {
    
        *(.text) /* Program code */
        *(.rodata) /* Read only data */
        *(.rodata*)
        __text_end = .;
    } >FLASH

    /*
     * This is the initialized data section
     * The program executes knowing that the data is in the RAM
     * but the loader puts the initial values in the FLASH (inidata).
     * One task of "startup" is to copy the initial values from FLASH to RAM.
     */
    .data :
    {
        /* This is used by the startup in order to initialize the .data secion */
        PROVIDE (__data_start = .);
        _sdata = .;
        _sidata = .;
        *(.data)
        *(.data.*)
        /* This is used by the startup in order to initialize the .data secion */
        PROVIDE (__data_end = .);
        _edata = .;
    } >RAM AT >FLASH

    .bss :
    {
        PROVIDE(__bss_start = .);
        _sbss = .;
        __bss_start__ = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        PROVIDE(__bss_end = .);
        _ebss = .;
        __bss_end__ = .;
    } >RAM

    . = ALIGN(4);

    _stack_start = .;

}

_end = .;
end = .;
/* Provide stack end address */
PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM) - 4);