#include <pigpio.h>
#include <iostream>
#include <cstdint>

// --- Mapeamento de pinos (numeração BCM) ------------------------------------
static const int ROWS[4] = {5, 6, 13, 19};   // linhas: saídas
static const int COLS[4] = {12, 16, 20, 21};  // colunas: entradas c/ pull-up

// Layout físico das teclas do teclado matricial 4x4
static const char KEYMAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static const unsigned DEBOUNCE_MS = 30;  // janela de estabilização (anti-bounce)

// Inicializa os GPIOs do teclado
bool initTeclado() {
    if (gpioInitialise() < 0) {
        std::cerr << "[ERRO] Falha ao inicializar pigpio.\n";
        return false;
    }
    for (int r = 0; r < 4; ++r) {
        gpioSetMode(ROWS[r], PI_OUTPUT);
        gpioWrite(ROWS[r], 1);  // linhas em nível alto (inativas)
    }
    for (int c = 0; c < 4; ++c) {
        gpioSetMode(COLS[c], PI_INPUT);
        gpioSetPullUpDown(COLS[c], PI_PUD_UP);  // pull-up interno evita flutuação
    }
    return true;
}

// Varre a matriz e retorna a tecla pressionada, ou '\0' se nenhuma.
// Aplica debounce por confirmação temporal.
char scanTeclado() {
    for (int r = 0; r < 4; ++r) {
        gpioWrite(ROWS[r], 0);            // ativa a linha atual (nível baixo)
        for (int c = 0; c < 4; ++c) {
            if (gpioRead(COLS[c]) == 0) { // coluna puxada para baixo -> tecla
                gpioDelay(DEBOUNCE_MS * 1000);        // espera estabilizar (us)
                if (gpioRead(COLS[c]) == 0) {         // ainda pressionada?
                    char tecla = KEYMAP[r][c];
                    // aguarda a soltura para gerar UM único evento
                    while (gpioRead(COLS[c]) == 0) gpioDelay(1000);
                    gpioWrite(ROWS[r], 1);
                    return tecla;
                }
            }
        }
        gpioWrite(ROWS[r], 1);            // desativa a linha
    }
    return '\0';
}

int main() {
    if (!initTeclado()) return 1;

    std::cout << "=== TESTE ISOLADO: TECLADO MATRICIAL 4x4 ===\n";
    std::cout << "Pressione teclas (ESPERADO: 1 evento por toque, sem bouncing).\n";
    std::cout << "Digite 'D' para encerrar.\n\n";

    while (true) {
        char t = scanTeclado();
        if (t != '\0') {
            std::cout << "[EVENTO] Tecla capturada: '" << t << "'\n";
            if (t == 'D') break;
        }
        gpioDelay(5000);  // 5 ms entre varreduras (não-bloqueante p/ o sistema)
    }

    std::cout << "\n[OK] Teste do teclado finalizado.\n";
    gpioTerminate();
    return 0;
}
