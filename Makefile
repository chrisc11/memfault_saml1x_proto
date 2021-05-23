ROOT_DIR := $(abspath .)

VPATH = $(ROOT_DIR)

BUILD_DIRNAME ?= build
BUILD_DIR  := $(ROOT_DIR)/$(BUILD_DIRNAME)
Q ?= @

COMPILER ?= arm-none-eabi-gcc

HAL_DIR := $(ROOT_DIR)/hal
HPL_DIR := $(ROOT_DIR)/hpl

SRC_FILES +=  \
  $(ROOT_DIR)/atmel_start.c \
  $(ROOT_DIR)/Device_Startup/startup_saml10d14a.c \
  $(ROOT_DIR)/Device_Startup/system_saml10d14a.c \
  $(ROOT_DIR)/driver_init.c \
  $(ROOT_DIR)/examples/driver_examples.c \
  $(ROOT_DIR)/main.c \
  $(HAL_DIR)/src/hal_atomic.c \
  $(HAL_DIR)/src/hal_delay.c \
  $(HAL_DIR)/src/hal_gpio.c \
  $(HAL_DIR)/src/hal_init.c \
  $(HAL_DIR)/src/hal_io.c \
  $(HAL_DIR)/src/hal_sleep.c \
  $(HAL_DIR)/utils/src/utils_assert.c \
  $(HAL_DIR)/utils/src/utils_event.c \
  $(HAL_DIR)/utils/src/utils_list.c \
  $(HAL_DIR)/utils/src/utils_syscalls.c \
  $(HPL_DIR)/core/hpl_core_m23_base.c \
  $(HPL_DIR)/core/hpl_init.c \
  $(HPL_DIR)/dmac/hpl_dmac.c \
  $(HPL_DIR)/gclk/hpl_gclk.c \
  $(HPL_DIR)/mclk/hpl_mclk.c \
  $(HPL_DIR)/osc32kctrl/hpl_osc32kctrl.c \
  $(HPL_DIR)/oscctrl/hpl_oscctrl.c \
  $(HPL_DIR)/pm/hpl_pm.c \
  $(HPL_DIR)/systick/hpl_systick.c

INCLUDE_PATHS += \
  $(ROOT_DIR)/Packs/arm/CMSIS/5.4.0/CMSIS/Core/Include \
  $(ROOT_DIR)/examples \
  $(ROOT_DIR)/hal/include \
  $(ROOT_DIR)/hal/utils/include \
  $(ROOT_DIR)/hpl/core \
  $(ROOT_DIR)/hpl/dmac \
  $(ROOT_DIR)/hpl/gclk \
  $(ROOT_DIR)/hpl/mclk \
  $(ROOT_DIR)/hpl/osc32kctrl \
  $(ROOT_DIR)/hpl/oscctrl \
  $(ROOT_DIR)/hpl/pm \
  $(ROOT_DIR)/hpl/port \
  $(ROOT_DIR)/hpl/systick \
  $(ROOT_DIR)/hri \
  $(ROOT_DIR)/Config \
  $(ROOT_DIR) \
  $(ROOT_DIR)/Packs/atmel/SAML10_DFP/1.0.158/include


#
# Pick up Memfault!
#

MEMFAULT_COMPONENTS := core util panics
MEMFAULT_SDK_DIR ?= memfault-firmware-sdk
MEMFAULT_SDK_ROOT = $(ROOT_DIR)/third_party/memfault/$(MEMFAULT_SDK_DIR)
MEMFAULT_PORT_ROOT := $(ROOT_DIR)/third_party/memfault
include $(MEMFAULT_SDK_ROOT)/makefiles/MemfaultWorker.mk

SRC_FILES += \
  $(MEMFAULT_COMPONENTS_SRCS) \
  $(MEMFAULT_SDK_ROOT)/ports/panics/src/memfault_platform_ram_backed_coredump.c \
  $(MEMFAULT_PORT_ROOT)/memfault_platform_port.c

INCLUDE_PATHS +=  \
  $(MEMFAULT_COMPONENTS_INC_FOLDERS) \
  $(MEMFAULT_SDK_ROOT)/ports/include \
  $(MEMFAULT_PORT_ROOT)

CFLAGS += -Wl,--build-id

# https://community.memfault.com/t/reproducible-firmware-builds-interrupt/112/12
SRC_FILES := $(sort $(SRC_FILES))

INCLUDES = $(foreach d, $(INCLUDE_PATHS), -I$d)

# https://community.memfault.com/t/reproducible-firmware-builds-interrupt/112/12
SRC_FILES := $(sort $(SRC_FILES))

CPU_CFLAGS := \
  -D__SAML10D14A__ \
  -mthumb \
  -mcpu=cortex-m23

CFLAGS += \
  $(CPU_CFLAGS) \
  -g3 \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -Os \
  -fdebug-prefix-map=$(ROOT_DIR)=. \
  -std=gnu99 \
  -Wbad-function-cast

# Some versions of GCC generate false postivies for this version
CFLAGS += \
  -Wno-bad-function-cast \
  -Wno-unused

# Disable error flags that result in issues in atmel SDK
CFLAGS += \
  -Wno-bad-function-cast \
  -Wno-unused


TARGET ?= memfault_proto
LDSCRIPT = Device_Startup/saml10d14a_flash.ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections,-Map,$(BUILD_DIR)/$(TARGET).map
LDFLAGS += --specs=nano.specs

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

all: $(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES)  -o $@ -Wl,-lc -Wl,-lgcc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

# Rebuild all objects anytime Makefile is edited
# $(OBJ_FILES): Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $*.c"
	@mkdir -p $(dir $@)
	$(Q) cd $(ROOT_DIR) && $(COMPILER) $(DEP_CFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $*.c
