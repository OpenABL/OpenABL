################################################################################
#
# FLAME GPU Script for CUDA 7.5
#
# Copyright 2016 University of Sheffield.  All rights reserved.
#
# Authors : Dr Mozhgan Kabiri Chimeh, Dr Paul Richmond
# Contact : {m.kabiri-chimeh,p.richmond}@sheffield.ac.uk
#
# NOTICE TO USER:
#
# University of Sheffield retain all intellectual property and
# proprietary rights in and to this software and related documentation.
# Any use, reproduction, disclosure, or distribution of this software
# and related documentation without an express license agreement from
# University of Sheffield is strictly prohibited.
#
# For terms of licence agreement please attached licence or view licence
# on www.flamegpu.com website.
#
################################################################################
# USAGE : make help
################################################################################
#
# Makefile project only supported on Linux Platforms
#
################################################################################

# FlameGPU root directory
FLAMEGPU_DIR ?= ../..

# Folders containing FLAMEGPU example files and templates
BIN_DIR := $(FLAMEGPU_DIR)/bin/x64/
IDIR := $(FLAMEGPU_DIR)/include/
# For now this will only work for x64 linux. 32x is uninportant as deprecated in CUDA 8.0 Other systems are currently not possible to test.
LDIR := $(FLAMEGPU_DIR)/lib/x86_64-linux-gnu/
TEMPLATE := $(FLAMEGPU_DIR)/FLAMEGPU/templates/
XSD_SCHEMA := $(FLAMEGPU_DIR)/FLAMEGPU/schemas/

INPUT_DATA:=iterations/0.xml

SRC_ := model/
SRC_VIZ := visualisation/
SRC_CUDA := dynamic/

OPENGL_FLAGS := -lglut -lGLEW -lGLU -lGL
FLAMELIB := -I $(IDIR) -I $(SRC_) -I $(SRC_CUDA) -I $(SRC_VIZ) -I $(IDIR)GL/

################################################################################
#Generating Dynamic Code from FLAMEGPU Templates

XML_MODEL:=$(SRC_)XMLModelFile.xml

XML_Validate:
	xmllint --noout $(XML_MODEL) --schema $(XSD_SCHEMA)XMMLGPU.xsd

XSLTPREP: XML_Validate
XSLTPREP:
	xsltproc $(TEMPLATE)header.xslt  $(XML_MODEL)> $(SRC_CUDA)header.h
	xsltproc $(TEMPLATE)FLAMEGPU_kernals.xslt $(XML_MODEL) > $(SRC_CUDA)FLAMEGPU_kernals.cu
	xsltproc $(TEMPLATE)io.xslt $(XML_MODEL) > $(SRC_CUDA)io.cu
	xsltproc $(TEMPLATE)simulation.xslt $(XML_MODEL) > $(SRC_CUDA)simulation.cu
	xsltproc $(TEMPLATE)main.xslt $(XML_MODEL) > $(SRC_CUDA)main.cu
	xsltproc $(TEMPLATE)visualisation.xslt $(XML_MODEL) > $(SRC_CUDA)visualisation.cu

################################################################################

# Location of the CUDA Toolkit
CUDA_PATH ?= "/usr/local/cuda"

# architecture
HOST_ARCH   := $(shell uname -m)
TARGET_ARCH ?= $(HOST_ARCH)
ifneq (,$(filter $(TARGET_ARCH),x86_64 aarch64 ppc64le))
    TARGET_SIZE := 64
else ifeq ($(TARGET_ARCH),armv7l)
    TARGET_SIZE := 32
else
    $(error ERROR - unsupported value $(TARGET_ARCH) for TARGET_ARCH!)
endif
ifneq ($(TARGET_ARCH),$(HOST_ARCH))
    ifeq (,$(filter $(HOST_ARCH)-$(TARGET_ARCH),aarch64-armv7l x86_64-armv7l x86_64-aarch64 x86_64-ppc64le))
        $(error ERROR - cross compiling from $(HOST_ARCH) to $(TARGET_ARCH) is not supported!)
    endif
endif

# operating system
HOST_OS   := $(shell uname -s 2>/dev/null | tr "[:upper:]" "[:lower:]")
TARGET_OS ?= $(HOST_OS)
ifeq (,$(filter $(TARGET_OS),linux darwin qnx android))
    $(error ERROR - unsupported value $(TARGET_OS) for TARGET_OS!)
endif

# host compiler
ifeq ($(TARGET_OS),darwin)
    ifeq ($(shell expr `xcodebuild -version | grep -i xcode | awk '{print $$2}' | cut -d'.' -f1` \>= 5),1)
        HOST_COMPILER ?= clang++
    endif
else ifneq ($(TARGET_ARCH),$(HOST_ARCH))
    ifeq ($(HOST_ARCH)-$(TARGET_ARCH),x86_64-armv7l)
        ifeq ($(TARGET_OS),linux)
            HOST_COMPILER ?= arm-linux-gnueabihf-g++
        else ifeq ($(TARGET_OS),qnx)
            ifeq ($(QNX_HOST),)
                $(error ERROR - QNX_HOST must be passed to the QNX host toolchain)
            endif
            ifeq ($(QNX_TARGET),)
                $(error ERROR - QNX_TARGET must be passed to the QNX target toolchain)
            endif
            export QNX_HOST
            export QNX_TARGET
            HOST_COMPILER ?= $(QNX_HOST)/usr/bin/arm-unknown-nto-qnx6.6.0eabi-g++
        else ifeq ($(TARGET_OS),android)
            HOST_COMPILER ?= arm-linux-androideabi-g++
        endif
    else ifeq ($(TARGET_ARCH),aarch64)
        ifeq ($(TARGET_OS), linux)
            HOST_COMPILER ?= aarch64-linux-gnu-g++
        else ifeq ($(TARGET_OS), android)
            HOST_COMPILER ?= aarch64-linux-android-g++
        endif
    else ifeq ($(TARGET_ARCH),ppc64le)
        HOST_COMPILER ?= powerpc64le-linux-gnu-g++
    endif
endif
HOST_COMPILER ?= g++
NVCC          := $(CUDA_PATH)/bin/nvcc -ccbin $(HOST_COMPILER)

# internal flags
NVCCFLAGS   := -m${TARGET_SIZE}
CCFLAGS     :=
LDFLAGS     := -L$(LDIR)

# build flags
ifeq ($(TARGET_OS),darwin)
    LDFLAGS += -rpath $(CUDA_PATH)/lib
    CCFLAGS += -arch $(HOST_ARCH)
else ifeq ($(HOST_ARCH)-$(TARGET_ARCH)-$(TARGET_OS),x86_64-armv7l-linux)
    LDFLAGS += --dynamic-linker=/lib/ld-linux-armhf.so.3
    CCFLAGS += -mfloat-abi=hard
else ifeq ($(TARGET_OS),android)
    LDFLAGS += -pie
    CCFLAGS += -fpie -fpic -fexceptions
endif

ifneq ($(TARGET_ARCH),$(HOST_ARCH))
    ifeq ($(TARGET_ARCH)-$(TARGET_OS),armv7l-linux)
        ifneq ($(TARGET_FS),)
            GCCVERSIONLTEQ46 := $(shell expr `$(HOST_COMPILER) -dumpversion` \<= 4.6)
            ifeq ($(GCCVERSIONLTEQ46),1)
                CCFLAGS += --sysroot=$(TARGET_FS)
            endif
            LDFLAGS += --sysroot=$(TARGET_FS)
            LDFLAGS += -rpath-link=$(TARGET_FS)/lib
            LDFLAGS += -rpath-link=$(TARGET_FS)/usr/lib
            LDFLAGS += -rpath-link=$(TARGET_FS)/usr/lib/arm-linux-gnueabihf
        endif
    endif
endif


# Debug build flags
ifeq ($(dbg),1)
      NVCCFLAGS += -g -G
      Mode_TYPE := Debug
else
      Mode_TYPE := Release
endif


ALL_CCFLAGS :=
ALL_CCFLAGS += $(NVCCFLAGS)
ALL_CCFLAGS += $(EXTRA_NVCCFLAGS)
ALL_CCFLAGS += $(addprefix -Xcompiler ,$(CCFLAGS))
ALL_CCFLAGS += $(addprefix -Xcompiler ,$(EXTRA_CCFLAGS))

ALL_LDFLAGS :=
ALL_LDFLAGS += $(ALL_CCFLAGS)
ALL_LDFLAGS += $(addprefix -Xlinker ,$(LDFLAGS))
ALL_LDFLAGS += $(addprefix -Xlinker ,$(EXTRA_LDFLAGS))

# Common includes and paths for CUDA
INCLUDES  := -I../../common/inc

# Common includes for OPENGL
LIBRARIES :=

################################################################################

SAMPLE_ENABLED := 1

# Gencode arguments
SMS ?= 30 35 37 50 52

ifeq ($(SMS),)
$(info >>> WARNING - no SM architectures have been specified - waiving sample <<<)
SAMPLE_ENABLED := 0
endif

ifeq ($(GENCODE_FLAGS),)
# Generate SASS code for each SM architecture listed in $(SMS)
$(foreach sm,$(SMS),$(eval GENCODE_FLAGS += -gencode arch=compute_$(sm),code=sm_$(sm)))

# Generate PTX code from the highest SM architecture in $(SMS) to guarantee forward-compatibility
HIGHEST_SM := $(lastword $(sort $(SMS)))
ifneq ($(HIGHEST_SM),)
GENCODE_FLAGS += -gencode arch=compute_$(HIGHEST_SM),code=compute_$(HIGHEST_SM)
endif
endif

ifeq ($(SAMPLE_ENABLED),0)
EXEC ?= @echo "[@]"
endif

################################################################################

build: XSLTPREP main

build_vis: XSLTPREP main_vis

check.deps:
ifeq ($(SAMPLE_ENABLED),0)
	@echo "Sample will be waived due to the above missing dependencies"
else
	@echo "Sample is ready - all dependencies have been met"
endif

io.o: $(SRC_CUDA)io.cu
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(DEBUG) $(GENCODE_FLAGS) $(FLAMELIB) -o $@ -c $<

simulation.o: $(SRC_CUDA)simulation.cu
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(DEBUG) $(GENCODE_FLAGS) $(FLAMELIB) -o $@ -c $<

visualisation.o: $(SRC_CUDA)visualisation.cu
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(DEBUG) $(GENCODE_FLAGS) $(FLAMELIB) -o $@ -c -DVISUALISATION $<

main.o: $(SRC_CUDA)main.cu
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(DEBUG) $(GENCODE_FLAGS) $(FLAMELIB) -o $@ -c $<

main_vis.o: $(SRC_CUDA)main.cu
	$(EXEC) $(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(DEBUG) $(GENCODE_FLAGS) $(FLAMELIB) -o $@ -c -DVISUALISATION $<

main: io.o simulation.o main.o
	$(EXEC) $(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) -o $@ $+ $(LIBRARIES)

main_vis: io.o simulation.o main_vis.o visualisation.o
	$(EXEC) $(NVCC) $(ALL_LDFLAGS) $(GENCODE_FLAGS) $(OPENGL_FLAGS) -o $@ $+ $(LIBRARIES) \
		&& mv main_vis main

.PHONY: clean
clean:
	rm -f *.o
	rm -rf $(SRC_CUDA)*

