EXAMPLE := main
HAS_VISUALISATION := 1
CUSTOM_VISUALISATION := 0
FLAMEGPU_ROOT := $(FLAMEGPU_DIR)/
EXAMPLE_BIN_DIR := ./bin
EXAMPLE_BUILD_DIR := ./build

include $(FLAMEGPU_ROOT)tools/common.mk

# Copy binary to a predictable location.
# The default paths are architecture dependent.
console-ext: console
	cp $(TARGET_CONSOLE) main

visualisation-ext: visualisation
	cp $(TARGET_VISUALISATION) main
