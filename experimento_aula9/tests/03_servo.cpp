#include "config.hpp"
#include "gpio_guard.hpp"

#include <pigpio.h>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    try {
        GpioGuard gpio;
        gpioSetMode(config::SERVO_GPIO, PI_OUTPUT);

        const unsigned positions[] = {
            config::SERVO_LEFT_US,
            config::SERVO_CENTER_US,
            config::SERVO_RIGHT_US,
            config::SERVO_CENTER_US
        };

        for (unsigned pulse : positions) {
            std::cout << "Pulso do servo: " << pulse << " us\n";
            gpioServo(config::SERVO_GPIO, pulse);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        gpioServo(config::SERVO_GPIO, 0);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
