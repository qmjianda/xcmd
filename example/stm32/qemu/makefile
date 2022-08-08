# DECLARATIONS

# Directory where QEMU_STM32 ARM executable is located - used for running
# program in emulator.
QEMU_ARM_DIR ?= qemu_stm32/arm-softmmu/


# Declare command line tools - assume these are in the path
CC	  = arm-none-eabi-gcc
LD	  = arm-none-eabi-ld
AS	  = arm-none-eabi-as
CP	  = arm-none-eabi-objcopy
OD	  = arm-none-eabi-objdump

# Declare command line flags
CORE_CFLAGS = -I./ -I$(CORE_SRC) -I$(DEVICE_SRC) -I$(STD_PERIPH)/inc -Idemos/common -fno-common -O0 -g -mcpu=cortex-m3 -mthumb 
CFLAGS  = $(CORE_CFLAGS) -c 
CFLAGS_LINK = -Wl,-Tdemos/main.ld -nostartfiles $(CORE_CFLAGS) --specs=nano.specs -specs=nosys.specs
ASFLAGS = -mcpu=cortex-m3 -mthumb -g
LDFLAGS = -Tdemos/main.ld
CPFLAGS = -Obinary
ODFLAGS	= -S

# Declare library source paths
SRC = $(realpath .)
CORE_SRC = $(SRC)/libraries/CMSIS/CM3/CoreSupport
DEVICE_SRC = $(SRC)/libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
STD_PERIPH = $(SRC)/libraries/STM32F10x_StdPeriph_Driver
STD_PERIPH_SRC = $(STD_PERIPH)/src

# List common and system library source files
# (i.e. for accessing STM32/Cortex M3 hardware) 
COMMON_FILES = $(CORE_SRC)/core_cm3.c
COMMON_FILES += $(DEVICE_SRC)/system_stm32f10x.c
COMMON_FILES += $(DEVICE_SRC)/startup/gcc_ride7/startup_stm32f10x_md.s
COMMON_FILES += demos/common/stm32_p103.c
COMMON_FILES += demos/common/myprintk.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_rcc.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_gpio.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_usart.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_exti.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_adc.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_tim.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_rtc.c
COMMON_FILES += $(STD_PERIPH_SRC)/stm32f10x_dac.c
COMMON_FILES += $(STD_PERIPH_SRC)/misc.c
#xcmd file
COMMON_FILES += $(SRC)/../../../src/xcmd.c
COMMON_FILES += $(SRC)/../../../src/xcmd_default_cmds.c
COMMON_FILES += $(SRC)/../../../src/xcmd_default_keys.c
COMMON_FILES += $(SRC)/../../../extensions/ex_cmds/ex_cmds.c
COMMON_FILES += $(SRC)/../../../extensions/ex_keys/ex_keys.c
COMMON_FILES += $(SRC)/../../../extensions/test/test.c
XCMD_INC = -I$(SRC)/../../../extensions/ex_cmds/ -I$(SRC)/../../../extensions/ex_keys/ -I$(SRC)/../../../extensions/test -I$(SRC)/../../../inc/

# List all demos
DEMOS =  main


# List all demo folders
DEMO_FOLDERS = $(addprefix demos/,$(DEMOS))

# List FreeRTOS resources
FREE_RTOS_SRC = $(SRC)/libraries/FreeRTOS
FREE_RTOS_SRC_FILES = $(FREE_RTOS_SRC)/croutine.c $(FREE_RTOS_SRC)/list.c $(FREE_RTOS_SRC)/queue.c $(FREE_RTOS_SRC)/tasks.c $(FREE_RTOS_SRC)/croutine.c $(FREE_RTOS_SRC)/stream_buffer.c $(FREE_RTOS_SRC)/portable/GCC/ARM_CM3/port.c
FREE_RTOS_INC = $(FREE_RTOS_SRC)/include/
FREE_RTOS_PORT_INC = $(FREE_RTOS_SRC)/portable/GCC/ARM_CM3/

# List path to demo build output files
OUTPUT_FILES = $(addsuffix /main,$(DEMO_FOLDERS))
ELF_FILES	= $(addsuffix .elf,$(OUTPUT_FILES))
LIST_FILES	= $(addsuffix .list,$(OUTPUT_FILES))
BIN_FILES	= $(addsuffix .bin,$(OUTPUT_FILES))

# Declare target names for each demo
ALL_TARGETS = $(addsuffix _ALL,$(DEMOS))
PROG_TARGETS = $(addsuffix _PROG,$(DEMOS))

QEMU_RUN_TARGETS = $(addsuffix _QEMURUN,$(DEMOS))
QEMU_RUN_PTY_TARGETS = $(addsuffix _QEMURUN_PTY,$(DEMOS))
QEMU_RUN_TEL_TARGETS = $(addsuffix _QEMURUN_TEL,$(DEMOS))
QEMU_DBG_TARGETS = $(addsuffix _QEMUDBG,$(DEMOS))
QEMU_DBG_PTY_TARGETS = $(addsuffix _QEMUDBG_PTY,$(DEMOS))
QEMU_DBG_TEL_TARGETS = $(addsuffix _QEMUDBG_TEL,$(DEMOS))



# TARGETS - See README for descriptions of the targets

# Generic targets
.PHONY: clean

all: $(ALL_TARGETS)

clean:
	find . -type f -name "*.o" -exec rm {} \;
	find . -type f -name "*.elf" -exec rm {} \;
	find . -type f -name "*.bin" -exec rm {} \;
	find . -type f -name "*.list" -exec rm {} \; 
	rm -f DAC_OUT_PUT1.txt DAC_OUT_PUT2.txt

# Compile targets (builds all output files)
$(ALL_TARGETS): %_ALL : demos/%/main.elf demos/%/main.bin demos/%/main.list

# QEMU run targets
$(QEMU_RUN_TARGETS): %_QEMURUN : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -kernel demos/$*/main.bin

$(QEMU_RUN_PTY_TARGETS): %_QEMURUN_PTY : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -kernel demos/$*/main.bin -serial pty
	
$(QEMU_RUN_TEL_TARGETS): %_QEMURUN_TEL : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -kernel demos/$*/main.bin -serial tcp::7777,server

# QEMU debug targets
$(QEMU_DBG_TARGETS): %_QEMUDBG : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -gdb tcp::3333 -S -kernel demos/$*/main.bin

$(QEMU_DBG_PTY_TARGETS): %_QEMUDBG_PTY : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -gdb tcp::3333 -S -kernel demos/$*/main.bin -serial pty
	
$(QEMU_DBG_TEL_TARGETS): %_QEMUDBG_TEL : %_ALL
	-killall -q qemu-system-arm
	$(QEMU_ARM_DIR)qemu-system-arm -M stm32-p103 -gdb tcp::3333 -S -kernel demos/$*/main.bin -serial tcp::7777,server
	
# Note: Use this command to run QEMU in low-level debug mode:
#    qemu-system-arm -cpu cortex-m3 -M stm32-p103 -nographic -singlestep -kernel main.bin -d in_asm,out_asm,exec,cpu,int,op,op_opt

# Compile targets to build individual files
$(LIST_FILES): %.list : %.elf
	$(OD) $(ODFLAGS) $< > $@

$(BIN_FILES): %.bin : %.elf
	$(CP) $(CPFLAGS) $< $@

# Targets to build individual demos
demos/main/main.elf: demos/main/main.c
demos/main/main.elf: $(COMMON_FILES)
	$(CC) $(CFLAGS_LINK) $(XCMD_INC) -O0 -Idemos/main/ -o $@ $^
