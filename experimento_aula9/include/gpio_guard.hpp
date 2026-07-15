#pragma once

#include <pigpio.h>
#include <stdexcept>

class GpioGuard {
public:
    GpioGuard() {
        if (gpioInitialise() < 0) {
            throw std::runtime_error(
                "Falha ao inicializar pigpio. Execute com sudo ou verifique a instalação."
            );
        }
    }

    GpioGuard(const GpioGuard&) = delete;
    GpioGuard& operator=(const GpioGuard&) = delete;

    ~GpioGuard() {
        gpioTerminate();
    }
};
