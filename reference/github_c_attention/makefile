CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build
TARGET := $(BUILD_DIR)/eagleeye_c_sim
SRCS := src/attention.c src/attention_scheduler.c src/attention_score.c \
        src/ball_challenger_gate.c \
        src/c_config.c src/decision_snapshot.c src/director_state.c \
        src/eagleeye_types.c src/enhance_policy.c src/fake_data.c \
        src/fovea_stats.c src/main.c src/motion_predictor.c \
        src/platform_io_sim.c src/player_attention_features.c src/risk.c src/roi_smoother.c \
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
