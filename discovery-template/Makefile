# put your *.o targets here, make should handle the rest!
SRCS += device/startup_stm32f0xx.s # add startup file to build

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJ_NAME=discovery-test

# Location of the Libraries folder from the STM32F0xx Standard Peripheral Library
STD_PERIPH_LIB=libraries

# Location of the linker scripts
LDSCRIPT_INC=device/ldscripts

# location of OpenOCD Board .cfg files (only used with 'make program')
OPENOCD_BOARD_DIR=$(OPENOCD_PATH)/../scripts/			board

# Configuration (cfg) file containing programming directives for OpenOCD
OPENOCD_PROC_FILE=extra/stm32f0-openocd.cfg

OPENOCD=openocd

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size

CFLAGS  = -Wall -g -O3  
CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=build/$(PROJ_NAME).map
#CFLAGS += --specs=nosys.specs
CFLAGS += -specs=rdimon.specs
CFLAGS += -lc -lrdimon
#CFLAGS += -specs=nano.specs -u _printf_float -specs=rdimon.specs -lc -lrdimon


###################################################

vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS += -I inc -I $(STD_PERIPH_LIB) -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc
CFLAGS += -include $(STD_PERIPH_LIB)/stm32f0xx_conf.h
CFLAGS += -I Include/
CFLAGS += -I $(STD_PERIPH_LIB)/FreeRTOS-Kernel/portable/GCC/ARM_CM0
CFLAGS += -I $(STD_PERIPH_LIB)/FreeRTOS-Kernel/include
CFLAGS += -I libraries/utils
CFLAGS += -I /usr/include

SRCS += libraries/FreeRTOS-Kernel/tasks.c
SRCS += src/system_stm32f0xx.c
SRCS += libraries/FreeRTOS-Kernel/list.c
SRCS += libraries/FreeRTOS-Kernel/croutine.c
SRCS += libraries/FreeRTOS-Kernel/event_groups.c
SRCS += libraries/FreeRTOS-Kernel/queue.c
SRCS += libraries/FreeRTOS-Kernel/timers.c
SRCS += libraries/FreeRTOS-Kernel/stream_buffer.c
SRCS += libraries/FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c
SRCS += libraries/utils/usart.c
SRCS += libraries/utils/timer.c
SRCS += libraries/utils/idle.c
SRCS += libraries/utils/rng.c
SRCS += libraries/utils/printf.c
OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: main

lib:
	$(MAKE) -C $(STD_PERIPH_LIB)

main: lib buildmain generateelf program

reaction: lib buildreaction generateelf program

matrix: lib buildmatrix generateelf program

concurrency: lib buildconcurrency generateelf program

generateelf:
	$(OBJCOPY) -O ihex build/$(PROJ_NAME).elf build/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary build/$(PROJ_NAME).elf build/$(PROJ_NAME).bin
	$(OBJDUMP) -St build/$(PROJ_NAME).elf > build/$(PROJ_NAME).lst
	$(SIZE) build/$(PROJ_NAME).elf

stack: () buildreaction
	

	mkdir -p build
	$(CC) $(CFLAGS) src/main.c $^ -o build/$(PROJ_NAME).elf -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

buildreaction: $(SRCS)
	mkdir -p build
	$(CC) $(CFLAGS) src/reaction.c $^ -o build/$(PROJ_NAME).elf -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

buildmain: $(SRCS)
	mkdir -p build
	$(CC) $(CFLAGS) src/main.c $^ -o build/$(PROJ_NAME).elf -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

buildconcurrency: $(SRCS)
	mkdir -p build
	$(CC) $(CFLAGS) src/concurrency.c $^ -o build/$(PROJ_NAME).elf -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

buildmatrix: $(SRCS)
	mkdir -p build
	$(CC) $(CFLAGS) src/matrix.c $^ -o build/$(PROJ_NAME).elf -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld

program: build/$(PROJ_NAME).bin
	$(OPENOCD) -f ./openocd.cfg -f $(OPENOCD_PROC_FILE) -c "stm_flash `pwd`/build/$(PROJ_NAME).bin"

stop:
	$(OPENOCD) -f ./openocd.cfg -f $(OPENOCD_PROC_FILE) -c "stm_stop"

clean:
	find ./ -name '*~' | xargs rm -f	
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst
	rm -rf build

reallyclean: clean
	$(MAKE) -C $(STD_PERIPH_LIB) clean
