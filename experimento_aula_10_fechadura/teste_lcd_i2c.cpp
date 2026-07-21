#include <pigpio.h>
#include <iostream>
#include <cstdint>
#include <string>

// --- Configuração I2C -------------------------------------------------------
static const int  I2C_BUS  = 1;      // /dev/i2c-1 no RPi3
static const int  LCD_ADDR = 0x27;   // ajuste conforme i2cdetect -y 1

// Bits de controle do backpack PCF8574 -> HD44780
static const uint8_t LCD_BACKLIGHT = 0x08;
static const uint8_t ENABLE        = 0x04;  // bit Enable
static const uint8_t MODE_CMD      = 0x00;  // RS = 0 (comando)
static const uint8_t MODE_DATA     = 0x01;  // RS = 1 (dado)

static int handle = -1;

// Pulso no pino Enable para o HD44780 "travar" o nibble
void lcdPulseEnable(uint8_t data) {
    i2cWriteByte(handle, data | ENABLE | LCD_BACKLIGHT);
    gpioDelay(1);       // > 450 ns
    i2cWriteByte(handle, (data & ~ENABLE) | LCD_BACKLIGHT);
    gpioDelay(50);      // tempo de comando
}

// Escreve um nibble (4 bits altos) com o modo (comando/dado)
void lcdWriteNibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    i2cWriteByte(handle, data);
    lcdPulseEnable(data);
}

// Escreve um byte completo em dois nibbles (modo 4 bits)
void lcdWriteByte(uint8_t value, uint8_t mode) {
    lcdWriteNibble(value & 0xF0, mode);          // nibble alto
    lcdWriteNibble((value << 4) & 0xF0, mode);   // nibble baixo
}

void lcdCommand(uint8_t cmd) { lcdWriteByte(cmd, MODE_CMD); }
void lcdData(uint8_t d)      { lcdWriteByte(d,  MODE_DATA); }

// Sequência de inicialização do HD44780 em modo 4 bits
void lcdInit() {
    gpioDelay(50000);
    lcdWriteNibble(0x30, MODE_CMD); gpioDelay(4500);
    lcdWriteNibble(0x30, MODE_CMD); gpioDelay(150);
    lcdWriteNibble(0x30, MODE_CMD);
    lcdWriteNibble(0x20, MODE_CMD);   // seta modo 4 bits
    lcdCommand(0x28);  // 4 bits, 2 linhas, 5x8
    lcdCommand(0x0C);  // display on, cursor off
    lcdCommand(0x06);  // incremento automático
    lcdCommand(0x01);  // limpa
    gpioDelay(2000);
}

// Posiciona o cursor (linha 0 ou 1)
void lcdSetCursor(int col, int row) {
    static const uint8_t offset[2] = {0x00, 0x40};
    lcdCommand(0x80 | (col + offset[row]));
}

// Imprime uma string
void lcdPrint(const std::string& s) {
    for (char c : s) lcdData(static_cast<uint8_t>(c));
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "[ERRO] Falha ao inicializar pigpio.\n";
        return 1;
    }

    handle = i2cOpen(I2C_BUS, LCD_ADDR, 0);
    if (handle < 0) {
        std::cerr << "[ERRO] Nao foi possivel abrir o LCD em 0x"
                  << std::hex << LCD_ADDR
                  << ". Rode: i2cdetect -y 1\n";
        gpioTerminate();
        return 1;
    }

    std::cout << "=== TESTE ISOLADO: LCD 16x2 via I2C (0x"
              << std::hex << LCD_ADDR << ") ===\n";

    lcdInit();
    lcdSetCursor(0, 0);
    lcdPrint("Hello World");
    lcdSetCursor(0, 1);
    lcdPrint("STATUS: OK");

    std::cout << "[OK] 'Hello World' enviado ao display.\n";
    std::cout << "Verifique visualmente as duas linhas do LCD.\n";

    i2cClose(handle);
    gpioTerminate();
    return 0;
}
