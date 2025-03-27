V=1
SOURCE_DIR=..
BUILD_DIR=build
CXXFLAGS += -DFLURRY_TINY_TEXTURES

include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

all: flurry-tiny3d.z64
.PHONY: all

OBJS = \
    $(BUILD_DIR)/n64-libdragon-tiny3d/src/main.o  \
    $(BUILD_DIR)/core/Default.o \
    $(BUILD_DIR)/core/Gl_saver.o \
    $(BUILD_DIR)/core/Particle.o \
    $(BUILD_DIR)/core/Smoke.o    \
    $(BUILD_DIR)/core/Spark.o    \
    $(BUILD_DIR)/core/Star.o     \
    $(BUILD_DIR)/core/Std.o      \
    $(BUILD_DIR)/core/Texture.o

CXXFLAGS += -DFLURRY_TINY_TEXTURES


flurry-tiny3d.z64: N64_ROM_TITLE="Flurry (Tiny3D)"

$(BUILD_DIR)/flurry-tiny3d.elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)
