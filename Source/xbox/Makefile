XBE_TITLE = flurry
GEN_XISO = $(XBE_TITLE).iso

SRCS =  $(wildcard $(CURDIR)/*.cpp) $(wildcard $(CURDIR)/*.c) \
        $(wildcard $(CURDIR)/../core/*.cpp)

SHADER_OBJS = combiner.inl vshader.inl

NXDK_DIR ?= $(CURDIR)/../../../nxdk
NXDK_SDL = y
NXDK_CXX = y

%.inl: %.ps | fp20compiler
	@echo "[ FP20     ] $@"
	$(FP20COMPILER) $< > $@

include $(NXDK_DIR)/Makefile

