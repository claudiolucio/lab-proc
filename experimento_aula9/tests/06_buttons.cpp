#include "config.hpp"

#include <wiringPi.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {

volatile std::sig_atomic_t running = 1;

void stop_program(int) {
    running = 0;
}

void wait_for_release(int pin) {
    while (running && digitalRead(pin) == LOW) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config::BUTTON_DEBOUNCE_MS)
    );
}

}  // namespace

int main() {
    std::signal(SIGINT, stop_program);

    std::cout << "Programa de teste dos botões iniciado.\n";

    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Erro ao inicializar wiringPi.\n";
        return 1;
    }

    pinMode(config::BUTTON_UP_GPIO, INPUT);
    pinMode(config::BUTTON_DOWN_GPIO, INPUT);

    pullUpDnControl(config::BUTTON_UP_GPIO, PUD_UP);
    pullUpDnControl(config::BUTTON_DOWN_GPIO, PUD_UP);

    std::cout << "Pressione os botões. Use Ctrl+C para encerrar.\n";

    while (running) {
        if (digitalRead(config::BUTTON_UP_GPIO) == LOW) {
            std::cout << "Botão + pressionado\n";
            wait_for_release(config::BUTTON_UP_GPIO);
        }

        if (digitalRead(config::BUTTON_DOWN_GPIO) == LOW) {
            std::cout << "Botão - pressionado\n";
            wait_for_release(config::BUTTON_DOWN_GPIO);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout << "Teste encerrado.\n";
    return 0;
}
