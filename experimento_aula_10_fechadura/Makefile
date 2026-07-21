# Makefile - Testes isolados da Fechadura Eletronica (PCS3732 - Aula 10)
# Requer: pigpio (sudo apt-get install pigpio libpigpio-dev)
# Uso:
#   make            -> compila todos os testes
#   make teste_lcd  -> compila apenas um alvo
#   make clean      -> remove binarios
# Execute os binarios com sudo (pigpio exige acesso a /dev/mem):
#   sudo ./build/teste_teclado

CXX      := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17
LDFLAGS  := -lpigpio -lrt -lpthread

SRC_DIR   := src
BUILD_DIR := build

TARGETS := teste_teclado teste_lcd_i2c teste_sensor teste_buzzer

all: $(addprefix $(BUILD_DIR)/,$(TARGETS))

$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)
	@echo "[OK] Compilado: $@"

clean:
	rm -rf $(BUILD_DIR)
	@echo "[OK] Limpeza concluida."

.PHONY: all clean
