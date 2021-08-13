# =============================================================================
# custom settings
STM32Fx   = STM32F767xx
LIB_PATH  = ../../one-third-hal
PRJ_PATH  = . $(LIB_PATH)/core $(LIB_PATH)/device
LIBS      = -lc -lm -lnosys
OPT       = -Og
ASM_SRC   = $(LIB_PATH)/startups/startup_stm32f767xx.s
LD_SCRIPT = $(LIB_PATH)/lds/STM32F767ZITx_FLASH.ld
PRJ_OUT   = bin
PRJ_NAME  = $(STM32Fx)-binary

PRJ_INCH =  \
    $(LIB_PATH)/core \
    $(LIB_PATH)/core/config \
    $(LIB_PATH)/core/iic \
    $(LIB_PATH)/core/math \
    $(LIB_PATH)/core/spi \
    $(LIB_PATH)/core/uart \
    $(LIB_PATH)/device

# ------------------------------------------
ifneq (,$(findstring STM32F7,$(STM32Fx)))
    CPU        = -mcpu=cortex-m4
    FPU        = -mfpu=fpv4-sp-d16
    FLOAT-ABI  = -mfloat-abi=hard
    LIB_SHARE  = f7-share
    LIB_HAL    = f7-v1.16.0
    STM32Family= STM32F7xx
	HAL_INCH   = $(LIB_PATH)/${LIB_HAL}/inc/Legacy
else
    $(info not a STM32F7xx family device, you need to revise the Makefile)
endif

# =============================================================================
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CC) -x assembler-with-cpp
CP = $(CROSS_COMPILE)objcopy
SZ = $(CROSS_COMPILE)size
AR = $(CROSS_COMPILE)ar

# =============================================================================
MCU     = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
C_DEFS  = -D$(STM32Fx) -DUSE_HAL_DRIVER
C_DEFS += -DFIRMWARE=\"$(shell basename ${PWD})\"
C_DEFS += -DPRJ_GIT_CMT=\"$(shell git rev-parse --short HEAD)\"
C_DEFS += -DPRJ_GIT_BRH=\"$(shell git rev-parse --abbrev-ref HEAD)\"
C_DEFS += -DPRJ_GIT_VER=\"$(shell git describe --abbrev=7 --dirty --always --tags)\"
C_DEFS += -DLIB_GIT_CMT=\"$(shell git rev-parse --short HEAD)\"
C_DEFS += -DLIB_GIT_BRH=\"$(shell git rev-parse --abbrev-ref HEAD)\"
C_DEFS += -DLIB_GIT_VER=\"$(shell git describe --abbrev=7 --dirty --always --tags)\"

# =============================================================================
HAL_PATH += $(LIB_PATH)/${LIB_HAL}
HAL_OUT  = $(LIB_PATH)/$(STM32Fx)-hal
HAL_DIRS:= $(shell find $(HAL_PATH) -maxdepth 10 -type d)
HAL_SRCS = $(foreach dir,$(HAL_DIRS),$(wildcard $(dir)/*.c))
HAL_OBJS = $(addprefix $(HAL_OUT)/,$(notdir $(HAL_SRCS:.c=.o)))
vpath %.c $(sort $(dir $(HAL_SRCS)))

# =============================================================================
PRJ_PATH += $(LIB_PATH)/${LIB_SHARE}
PRJ_DIRS:= $(shell find $(PRJ_PATH) -maxdepth 10 -type d)
PRJ_SRCS = $(foreach dir,$(PRJ_DIRS),$(wildcard $(dir)/*.c))
PRJ_OBJS = $(addprefix $(PRJ_OUT)/,$(notdir $(PRJ_SRCS:.c=.o)))
vpath %.c $(sort $(dir $(PRJ_SRCS)))
PRJ_OBJS+= $(addprefix $(PRJ_OUT)/,$(notdir $(ASM_SRC:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SRC)))

HAL_INCH += \
    $(LIB_PATH)/CMSIS/Include \
    $(LIB_PATH)/CMSIS/Device/ST/${STM32Family}/Include \
    $(LIB_PATH)/${LIB_HAL}/inc \
    $(LIB_PATH)/${LIB_SHARE} \
    ./inc
HAL_INC  = $(HAL_INCH:%=-I%)

PRJ_INCs = $(shell find -L $(PRJ_INCH) -name '*.h' -exec dirname {} \; | uniq)
PRJ_INC  = $(PRJ_INCs:%=-I%)
PRJ_INC += ${HAL_INC}

# =============================================================================
CFLAGS   = $(MCU) $(C_DEFS) $(OPT)
CFLAGS  += -Werror=unused-parameter
CFLAGS  += -Werror=unused-variable
CFLAGS  += -Werror=unused-function
CFLAGS  += -Wall -fdata-sections -ffunction-sections
CFLAGS  += -MMD -MP -MF"$(@:%.o=%.d)" -g -gdwarf-2
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LD_SCRIPT) $(LIBS)
LDFLAGS += -Wl,-Map=$(PRJ_OUT)/$(PRJ_NAME).map,--cref -Wl,--gc-sections
LDFLAGS += -l$(STM32Fx) -L$(HAL_OUT)/

# =============================================================================
.PHONY:
# -----------------
all: release
	$(SZ) $(PRJ_OUT)/$(PRJ_NAME).elf
release: CFLAGS+= -D MAKE_TYPE=\"RELEASE\" $(PRJ_INC)
release: $(PRJ_OUT)/$(PRJ_NAME).elf
$(PRJ_OUT)/%.o: %.c Makefile | $(PRJ_OUT)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(PRJ_OUT)/$(notdir $(<:.c=.lst)) $< -o $@

$(PRJ_OUT)/%.o: %.s Makefile | $(PRJ_OUT)
	$(AS) -c $(CFLAGS) $< -o $@

$(PRJ_OUT)/$(PRJ_NAME).elf: $(PRJ_OBJS) 
	$(CC) $(PRJ_OBJS) $(LDFLAGS) -o $(PRJ_OUT)/$(PRJ_NAME).elf
	$(CP) -O ihex $(PRJ_OUT)/$(PRJ_NAME).elf $(PRJ_OUT)/$(PRJ_NAME).hex
	$(CP) -O binary -S $(PRJ_OUT)/$(PRJ_NAME).elf $(PRJ_OUT)/$(PRJ_NAME).bin

$(PRJ_OUT):
	mkdir -p $@

# -----------------
hal: CFLAGS+=$(HAL_INC)
hal: $(HAL_OUT)/lib$(STM32Fx).a
$(HAL_OUT)/%.o: %.c Makefile | $(HAL_OUT)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(HAL_OUT)/$(notdir $(<:.c=.lst)) $< -o $@

$(HAL_OUT)/lib$(STM32Fx).a: $(HAL_OBJS) 
	$(AR) -r $@ $(HAL_OBJS)

$(HAL_OUT):
	mkdir -p $@

hal-clean:
	rm -rf $(PRJ_OUT)
	rm -rf $(HAL_OUT)

# -----------------
download:
	st-flash write $(PRJ_OUT)/$(PRJ_NAME).bin 0x8000000
	st-flash reset

clean:
	rm -rf $(PRJ_OUT)

erase:
	st-flash erase

# -----------------
debug: CFLAGS+= -D __DEBUG $(PRJ_INC)
debug: CFLAGS+= -D MAKE_TYPE=\"DEBUG\"
debug: $(PRJ_OUT)/$(PRJ_NAME).elf
	$(SZ) $(PRJ_OUT)/$(PRJ_NAME).elf

-include $(wildcard $(PRJ_OUT)/*.d)
-include $(wildcard $(HAL_OUT)/*.d)
