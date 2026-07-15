#include "config.hpp"
#include "gpio_guard.hpp"

#include <pigpio.h>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    try {
        GpioGuard gpio;
        gpioSetMode(config::LED_GPIO, PI_OUTPUT);
        gpioSetPWMrange(config::LED_GPIO, 100);
        gpioPWM(config::LED_GPIO, 50);

        for (unsigned frequency : {1u, 5u, 10u, 50u, 100u, 1000u}) {
            const int actual = gpioSetPWMfrequency(
                config::LED_GPIO,
                frequency
            );

            std::cout << "Solicitada: " << frequency
                      << " Hz | aplicada: " << actual
                      << " Hz | duty: 50%\n";

            std::this_thread::sleep_for(std::chrono::seconds(4));
        }

        gpioPWM(config::LED_GPIO, 0);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
