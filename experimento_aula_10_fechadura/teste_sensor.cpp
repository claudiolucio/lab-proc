#include <pigpio.h>
#include <iostream>

static const int TRIG = 23;
static const int ECHO = 24;

// Distância (cm) abaixo da qual consideramos a porta FECHADA
static const double LIMIAR_FECHADO_CM = 8.0;

// Dispara o TRIG e mede o tempo de eco; retorna distância em cm (-1 se timeout)
double medirDistanciaCm() {
    gpioWrite(TRIG, 0);
    gpioDelay(2);
    gpioWrite(TRIG, 1);
    gpioDelay(10);       // pulso de 10 us
    gpioWrite(TRIG, 0);

    // aguarda subida do ECHO (com timeout ~30 ms)
    uint32_t start = gpioTick();
    while (gpioRead(ECHO) == 0) {
        if (gpioTick() - start > 30000) return -1.0;
    }
    uint32_t t0 = gpioTick();

    // aguarda descida do ECHO
    while (gpioRead(ECHO) == 1) {
        if (gpioTick() - t0 > 30000) return -1.0;
    }
    uint32_t t1 = gpioTick();

    double dur_us = static_cast<double>(t1 - t0);
    return (dur_us * 0.0343) / 2.0;  // velocidade do som ~343 m/s
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "[ERRO] Falha ao inicializar pigpio.\n";
        return 1;
    }

    gpioSetMode(TRIG, PI_OUTPUT);
    gpioSetMode(ECHO, PI_INPUT);
    gpioWrite(TRIG, 0);
    gpioDelay(500000);  // estabiliza o sensor

    std::cout << "=== TESTE ISOLADO: SENSOR DE ESTADO (HC-SR04) ===\n";
    std::cout << "Aproxime/afaste um obstaculo para simular Fechada/Aberta.\n";
    std::cout << "Ctrl+C para encerrar.\n\n";

    while (true) {
        double d = medirDistanciaCm();
        if (d < 0) {
            std::cout << "[ERRO] Timeout na leitura (verifique fiacao/echo).\n";
        } else {
            const char* estado = (d <= LIMIAR_FECHADO_CM) ? "FECHADA" : "ABERTA";
            std::cout << "[LEITURA] " << d << " cm -> Porta " << estado << "\n";
        }
        gpioDelay(300000);  // 300 ms entre leituras
    }

    gpioTerminate();
    return 0;
}
