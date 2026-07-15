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

        for (int i = 0; i < 5; ++i) {
            std::cout << "Bip " << i + 1 << '\n';
            gpioWrite(config::BUZZER_GPIO, PI_HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            gpioWrite(config::BUZZER_GPIO, PI_LOW);
            std::this_thread::sleep_for(std::chrono::milliseconds(900));
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
