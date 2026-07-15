#include "config.hpp"
#include "gpio_guard.hpp"

#include <pigpio.h>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    try {
        GpioGuard gpio;
        gpioSetMode(config::BUZZER_GPIO, PI_OUTPUT);
        gpioSetPWMrange(config::BUZZER_GPIO, 100);

        for (unsigned frequency : {262u, 330u, 392u, 523u, 784u, 1000u}) {
            const int actual = gpioSetPWMfrequency(
                config::BUZZER_GPIO,
                frequency
            );

            std::cout << "Tom solicitado: " << frequency
                      << " Hz | aplicado: " << actual << " Hz\n";

            gpioPWM(config::BUZZER_GPIO, 50);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            gpioPWM(config::BUZZER_GPIO, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
