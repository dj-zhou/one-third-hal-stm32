# =============================================================================
# custom settings
STM32Fx    = STM32F4
PROJ_NAME  = f407zg-binary
CODE_PATH  = .
LIB_CODE   = ../../one-third-hal
CODE_PATH += $(LIB_CODE)/CMSIS
CODE_PATH += $(LIB_CODE)/core
CODE_PATH += $(LIB_CODE)/f4-share
CODE_PATH += $(LIB_CODE)/f4-v1.25.1
C_DEFS     = -DSTM32F407xx -DUSE_HAL_DRIVER
ASM_SRC    = $(LIB_CODE)/startups/startup_stm32f407xx.s
LD_SCRIPT  = $(LIB_CODE)/lds/STM32F407ZGTx_FLASH.ld
LIBDIR     =
LIBS       = -lc -lm -lnosys
OUTPATH    = bin
OPT        = -Og
CSTD       = -std=gnu99

# =============================================================================
ifeq ($(STM32Fx), STM32F4)
CPU       = -mcpu=cortex-m4
FPU       = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard
MCU       = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
endif

# =============================================================================
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CC) -x assembler-with-cpp
CP = $(CROSS_COMPILE)objcopy
SZ = $(CROSS_COMPILE)size

# =============================================================================
C_DEFS += -D FIRMWARE=\"$(shell basename ${PWD})\"
C_DEFS += -D PRJ_GIT_CMT=\"$(shell git rev-parse --short HEAD)\"
C_DEFS += -D PRJ_GIT_BRH=\"$(shell git rev-parse --abbrev-ref HEAD)\"
C_DEFS += -D PRJ_GIT_VER=\"$(shell git describe --abbrev=7 --dirty --always --tags)\"
C_DEFS += -D LIB_GIT_CMT=\"$(shell git rev-parse --short HEAD)\"
C_DEFS += -D LIB_GIT_BRH=\"$(shell git rev-parse --abbrev-ref HEAD)\"
C_DEFS += -D LIB_GIT_VER=\"$(shell git describe --abbrev=7 --dirty --always --tags)\"
$(info )
$(info ------------------------------------)
$(info C_DEFS:)
$(info  $(C_DEFS))

# =============================================================================
DIRS := $(shell find $(CODE_PATH) -maxdepth 10 -type d)
SRCS  = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
$(info )
$(info ------------------------------------)
$(info source files:)
$(info  $(SRCS))

OBJS  = $(addprefix $(OUTPATH)/,$(notdir $(SRCS:.c=.o)))
vpath %.c $(sort $(dir $(SRCS)))
OBJS += $(addprefix $(OUTPATH)/,$(notdir $(ASM_SRC:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SRC)))
$(info )
$(info ------------------------------------)
$(info object files:)
$(info  $(OBJS))

#collect '.h' files in INC
INCH = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.h))
INC  = $(shell find -L $(INCH) -name '*.h' -exec dirname {} \; | uniq)
$(info )
$(info ------------------------------------)
$(info headers to include:)
$(info  $(INC:%=-I%))

$(info )
$(info ====================================)

# =============================================================================
CFLAGS   = $(CSTD) $(MCU) $(C_DEFS) $(INC:%=-I%) $(OPT)
CFLAGS  += -Werror=unused-parameter
CFLAGS  += -Werror=unused-variable
CFLAGS  += -Werror=unused-function
CFLAGS  += -Wall -fdata-sections -ffunction-sections
CFLAGS  += -MMD -MP -MF"$(@:%.o=%.d)" -g -gdwarf-2
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LD_SCRIPT) $(LIBDIR) $(LIBS)
LDFLAGS += -Wl,-Map=$(OUTPATH)/$(PROJ_NAME).map,--cref -Wl,--gc-sections

# =============================================================================
.PHONY:
all: release
	$(SZ) $(OUTPATH)/$(PROJ_NAME).elf

release: CFLAGS+= -D MAKE_TYPE=\"RELEASE\"
release: $(OUTPATH)/$(PROJ_NAME).elf
$(OUTPATH)/%.o: %.c Makefile | $(OUTPATH)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUTPATH)/$(notdir $(<:.c=.lst)) $< -o $@

$(OUTPATH)/%.o: %.s Makefile | $(OUTPATH)
	$(AS) -c $(CFLAGS) $< -o $@

$(OUTPATH)/$(PROJ_NAME).elf: $(OBJS) Makefile
	$(CC) $(OBJS) $(LDFLAGS) -o $(OUTPATH)/$(PROJ_NAME).elf
	$(CP) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(CP) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin

$(OUTPATH):
	mkdir $@

download:
	st-flash write $(OUTPATH)/$(PROJ_NAME).bin 0x8000000

clean:
	rm -f *.o
	rm -f $(OUTPATH)/*

erase:
	st-flash erase

# -----------------------------------------------------------------------------
debug: CFLAGS+= -D __DEBUG
debug: CFLAGS+= -D MAKE_TYPE=\"DEBUG\"
debug: $(OUTPATH)/$(PROJ_NAME).elf

# dependencies
-include $(wildcard $(OUTPATH)/*.d)
