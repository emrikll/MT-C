# STM32f051 FreeRTOS Template
This is an FreeRTOS example for STM32F051 (ARM Cortex M0 controller). Below are the steps to get it working in Linux (tested on Arch)

It is based of of the template by [bhavink09] (https://github.com/bhavink09/stm32f051_template/tree/master), but made to work more in the terminaland with Neovim and clangd.

### Installation
* Arch
``` sh1
$ sudo pacman -S gcc-arm-none-eabi gdb-common openocd make bear
$ yay -S gdb-multiarch
```

### Neovim Setup
* To get the includes to work in neovim bear is used to generate a compile_commands.json.
``` sh2
$ make clean; bear -- make
```

### Debugging
* To start debugging run these commands.
``` sh3
$ make (if not run before)
```

In Terminal 1
``` sh4
$ make program
```

In Terminal 2
``` sh5
$ chmod +x ./start-gdb.sh
$ ./start-gdb.sh
```

Press y and then prints should show in Terminal 1.


### Using QEMU to emulate code
* The QEMU installed above is specially built for certain ARM boards. However, if you directly try to debug a normal app built for stm on qemu, you will get an error as below:
``` sh7
qemu: fatal: Trying to execute code outside RAM or ROM at 0x08000390
```
This is because for STM32F051, the qemu somehow thinks that the FLASH starts from 0x0 and RAM starts from 0x20000000. The RAM address is correct but the flash actually starts from 0x08000000. To solve this, you need to change the Flash address to 0x0 temporarily in the linker script and re-compile the code.
* Note: Remember to change the address back to original before loading the code into actual chip.
