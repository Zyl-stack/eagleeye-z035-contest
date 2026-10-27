CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build
TARGET := $(BUILD_DIR)/eagleeye_c_sim
SRCS := src/attention.c src/attention_scheduler.c src/attention_score.c \
        src/eagleeye_types.c src/fake_data.c src/main.c \
        src/motion_predictor.c src/platform_io_sim.c src/risk.c \
        src/state_machine.c src/transaction.c
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all run clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
