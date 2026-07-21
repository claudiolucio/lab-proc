#include <pigpio.h>
#include <iostream>

static const int BUZZER = 18;  // GPIO18 suporta PWM

// Emite um tom por certa duração usando PWM (buzzer passivo).
// NOTA: gpioDelay aqui é aceitável no TESTE ISOLADO; na INTEGRAÇÃO,
// o buzzer é temporizado por máquina de estados (não-bloqueante).
void tom(unsigned freqHz, unsigned duracaoMs) {
    gpioSetPWMfrequency(BUZZER, freqHz);
    gpioPWM(BUZZER, 128);            // duty 50%
    gpioDelay(duracaoMs * 1000);
    gpioPWM(BUZZER, 0);             // silencia
}

void bipeSucesso() {
    std::cout << "[SUCESSO] Bipe curto (1000 Hz, 150 ms)\n";
    tom(1000, 150);
}

void bipeFalha() {
    std::cout << "[FALHA] Bipe longo (400 Hz, 600 ms)\n";
    tom(400, 600);
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "[ERRO] Falha ao inicializar pigpio.\n";
        return 1;
    }
    gpioSetMode(BUZZER, PI_OUTPUT);

    std::cout << "=== TESTE ISOLADO: BUZZER ===\n";
    std::cout << "Sequencia: sucesso -> pausa -> falha.\n\n";

    bipeSucesso();
    gpioDelay(500000);
    bipeFalha();

    std::cout << "\n[OK] Teste do buzzer finalizado.\n";
    gpioPWM(BUZZER, 0);
    gpioTerminate();
    return 0;
}
