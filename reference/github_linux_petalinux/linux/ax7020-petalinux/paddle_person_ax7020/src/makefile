CXX ?= arm-linux-gnueabihf-g++
PADDLE_LITE_DIR ?= ../paddle_lite
INTEGRATION_DIR ?= ../../integration/linux_ov5640_picodet
TARGET ?= ../paddle_person
VDMA_PROBE_TARGET ?= ../vdma_probe
CXXFLAGS ?= -O3 -std=c++14 -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fno-omit-frame-pointer
CPPFLAGS += -I$(PADDLE_LITE_DIR)/include -I$(INTEGRATION_DIR)/src -I.
LDFLAGS += -static -Wl,--gc-sections -Wl,-z,noexecstack
LDLIBS += $(PADDLE_LITE_DIR)/lib/libpaddle_api_light_bundled.a -lgomp -lpthread -ldl -lm

SOURCES := paddle_person.cpp $(INTEGRATION_DIR)/src/eagleeye_hw.cpp
HEADERS := $(INTEGRATION_DIR)/src/eagleeye_hw.hpp \
	$(INTEGRATION_DIR)/src/frame_layout.h \
	$(INTEGRATION_DIR)/reference_baremetal/ov5640.c

PROBE_SOURCES := $(INTEGRATION_DIR)/src/vdma_probe.cpp

.PHONY: all clean
all: $(TARGET)

vdma_probe: $(VDMA_PROBE_TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS) $(LDLIBS)

$(VDMA_PROBE_TARGET): $(PROBE_SOURCES) $(INTEGRATION_DIR)/src/frame_layout.h \
	$(INTEGRATION_DIR)/reference_baremetal/ov5640.c
	$(CXX) -I$(INTEGRATION_DIR)/src $(CXXFLAGS) $(PROBE_SOURCES) -o $@ \
		-static -Wl,-z,noexecstack -lpthread

clean:
	rm -f $(TARGET) $(VDMA_PROBE_TARGET)
