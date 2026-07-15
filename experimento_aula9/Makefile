CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
LDLIBS := -lpigpio -lwiringPi -lrt -lpthread

BIN_DIR := bin

GPIO_PROGRAMS := \
	metronome \
	01_led_pwm \
	02_led_frequency \
	03_servo \
	04_buzzer_passive \
	05_buzzer_active \
	06_buttons

all: $(addprefix $(BIN_DIR)/,$(GPIO_PROGRAMS)) $(BIN_DIR)/07_timing

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/metronome: src/metronome.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/01_led_pwm: tests/01_led_pwm.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/02_led_frequency: tests/02_led_frequency.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/03_servo: tests/03_servo.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/04_buzzer_passive: tests/04_buzzer_passive.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/05_buzzer_active: tests/05_buzzer_active.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/06_buttons: tests/06_buttons.cpp include/config.hpp include/gpio_guard.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(BIN_DIR)/07_timing: tests/07_timing.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR) data/*.csv

.PHONY: all clean
