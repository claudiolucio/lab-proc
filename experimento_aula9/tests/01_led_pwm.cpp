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
        gpioSetPWMfrequency(config::LED_GPIO, 1000);

        for (unsigned duty : {0u, 25u, 50u, 75u, 100u}) {
            std::cout << "Duty cycle: " << duty << "%\n";
            gpioPWM(config::LED_GPIO, duty);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        gpioPWM(config::LED_GPIO, 0);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
