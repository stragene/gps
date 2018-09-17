#工程的名称及最后生成文件的名字
TARGET = GPS_GPRS
#DEBUG编译选项
DEBUG = 1
BUILD_DIR = build
OPT = -Og
#源代码目录 
SOURCES_DIR = \
cmsis/source \
stdperiph/source \
user/source \
FreeRTOS
# 固件库
PERIFLIB_PATH = 
# 编译路径
BUILD_DIR = build
# C源文件
C_SOURCES=$(shell find ./ -name '*.c')  
# 汇编源文件
ASM_SOURCES=$(shell find ./ -name '*.s')  
# 固件库
PERIFLIB_SOURCES = 
#设定临时性环境变量
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
#######################################
# 编译选项
#######################################
# cpu
CPU = -mcpu=cortex-m4
#浮点运算单元类型
FPU = -mfpu=fpv4-sp-d16
# 浮点运算单元接口
FLOAT-ABI = -mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
# 汇编宏定义
AS_DEFS = 
# C宏定义
C_DEFS = \
-DUSE_STDPERIPH_DRIVER
# 汇编头文件路径
AS_INCLUDES =  
# C头文件路径 
C_INCLUDES = \
-Icmsis/include \
-Istdperiph/include \
-IFreeRTOS/include \
-IFreeRTOS/Source/portable/GCC/ARM_CM4_MPU \
-Ilwip/include\
-Iuser/include
# ASFLAGS
ASFLAGS = 
# CFLAGS
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
# 是否生成DEBUG信息
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
# 生成依赖信息
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

#######################################
# 链接选项
#######################################
# 链接脚本
LDSCRIPT = STM32_FLASH.ld
# 库文件 
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

#默认命令 
all:$(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
