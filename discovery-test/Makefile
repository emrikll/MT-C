# put your *.o targets here, make should handle the rest!
SRCS = $(wildcard src/*.c)
SRCS += device/startup_stm32f0xx.s # add startup file to build

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJ_NAME=discovery-test

# Location of the Libraries folder from the STM32F0xx Standard Peripheral Library
STD_PERIPH_LIB=libraries

# Location of the linker scripts
LDSCRIPT_INC=device/ldscripts

# Location of FreeRTOS
FREERTOS_LIB=$(STD_PERIPH_LIB)/FreeRTOS-Kernel

TRACERECORDER_LIB = $(STD_PERIPH_LIB)/TraceRecorder

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

CFLAGS  = -Wall -g -std=c99 -Os  
CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=build/$(PROJ_NAME).map
#CFLAGS += --specs=nosys.specs
CFLAGS += -specs=rdimon.specs
CFLAGS += -lc -lrdimon
#CFLAGS += -specs=nano.specs -u _printf_float -specs=rdimon.specs -lc -lrdimon


###################################################

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS += -I inc -I $(STD_PERIPH_LIB) -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc
CFLAGS += -include $(STD_PERIPH_LIB)/stm32f0xx_conf.h
CFLAGS += -I Include/
CFLAGS += -I $(FREERTOS_LIB)/portable/GCC/ARM_CM0
CFLAGS += -I $(FREERTOS_LIB)/include
CFLAGS += -I /usr/include
CFLAGS += -I $(TRACERECORDER_LIB)/config
CFLAGS += -I $(TRACERECORDER_LIB)/include

SRCS += $(FREERTOS_LIB)/tasks.c
SRCS += $(FREERTOS_LIB)/list.c
SRCS += $(FREERTOS_LIB)/croutine.c
SRCS += $(FREERTOS_LIB)/event_groups.c
SRCS += $(FREERTOS_LIB)/queue.c
SRCS += $(FREERTOS_LIB)/stream_buffer.c
SRCS += $(FREERTOS_LIB)/portable/GCC/ARM_CM0/port.c
SRCS += $(FREERTOS_LIB)/portable/MemMang/heap_4.c

SRCS += $(TRACERECORDER_LIB)/trcAssert.c
SRCS += $(TRACERECORDER_LIB)/trcCounter.c
SRCS += $(TRACERECORDER_LIB)/trcDependency.c
SRCS += $(TRACERECORDER_LIB)/trcDiagnostics.c
SRCS += $(TRACERECORDER_LIB)/trcEntryTable.c
SRCS += $(TRACERECORDER_LIB)/trcError.c
SRCS += $(TRACERECORDER_LIB)/trcEvent.c
SRCS += $(TRACERECORDER_LIB)/trcEventBuffer.c
SRCS += $(TRACERECORDER_LIB)/trcExtension.c
SRCS += $(TRACERECORDER_LIB)/trcHardwarePort.c
SRCS += $(TRACERECORDER_LIB)/trcHeap.c
SRCS += $(TRACERECORDER_LIB)/trcInternalEventBuffer.c
SRCS += $(TRACERECORDER_LIB)/trcInterval.c
SRCS += $(TRACERECORDER_LIB)/trcISR.c
SRCS += $(TRACERECORDER_LIB)/trcKernelPort.c
SRCS += $(TRACERECORDER_LIB)/trcMultiCoreEventBuffer.c
SRCS += $(TRACERECORDER_LIB)/trcObject.c
SRCS += $(TRACERECORDER_LIB)/trcPrint.c
SRCS += $(TRACERECORDER_LIB)/trcRunnable.c
SRCS += $(TRACERECORDER_LIB)/trcSnapshotRecorder.c
SRCS += $(TRACERECORDER_LIB)/trcStackMonitor.c
SRCS += $(TRACERECORDER_LIB)/trcStateMachine.c
SRCS += $(TRACERECORDER_LIB)/trcStaticBuffer.c
SRCS += $(TRACERECORDER_LIB)/trcStreamingRecorder.c
SRCS += $(TRACERECORDER_LIB)/trcStreamPort.c
SRCS += $(TRACERECORDER_LIB)/trcString.c
SRCS += $(TRACERECORDER_LIB)/trcTask.c
SRCS += $(TRACERECORDER_LIB)/trcTimestamp.c

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C $(STD_PERIPH_LIB)

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	mkdir build
	$(CC) $(CFLAGS) $^ -o build/$@ -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld
	$(OBJCOPY) -O ihex build/$(PROJ_NAME).elf build/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary build/$(PROJ_NAME).elf build/$(PROJ_NAME).bin
	$(OBJDUMP) -St build/$(PROJ_NAME).elf > build/$(PROJ_NAME).lst
	$(SIZE) build/$(PROJ_NAME).elf
	
program: build/$(PROJ_NAME).bin
	$(OPENOCD) -f ./openocd.cfg -f $(OPENOCD_PROC_FILE) -c "stm_flash `pwd`/build/$(PROJ_NAME).bin"

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
