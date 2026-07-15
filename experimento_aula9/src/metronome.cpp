#include "config.hpp"
#include "gpio_guard.hpp"

#include <pigpio.h>
#include <wiringPi.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>

namespace {

using Clock = std::chrono::steady_clock;

std::atomic<bool> running{true};
std::atomic<int> bpm{config::DEFAULT_BPM};

void handle_signal(int) {
    running.store(false);
}

void configure_outputs() {
    gpioSetMode(config::LED_GPIO, PI_OUTPUT);
    gpioSetMode(config::SERVO_GPIO, PI_OUTPUT);
    gpioSetMode(config::BUZZER_GPIO, PI_OUTPUT);

    gpioWrite(config::LED_GPIO, PI_LOW);
    gpioWrite(config::BUZZER_GPIO, PI_LOW);
    gpioServo(config::SERVO_GPIO, config::SERVO_CENTER_US);
}

bool configure_buttons() {
    if (wiringPiSetupGpio() == -1) {
        return false;
    }

    pinMode(config::BUTTON_UP_GPIO, INPUT);
    pinMode(config::BUTTON_DOWN_GPIO, INPUT);

    pullUpDnControl(config::BUTTON_UP_GPIO, PUD_UP);
    pullUpDnControl(config::BUTTON_DOWN_GPIO, PUD_UP);

    return true;
}

void wait_for_release(int pin) {
    while (running.load() && digitalRead(pin) == LOW) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config::BUTTON_DEBOUNCE_MS)
    );
}

void button_loop() {
    while (running.load()) {
        if (digitalRead(config::BUTTON_UP_GPIO) == LOW) {
            int current = bpm.load();
            bpm.store(std::min(
                config::MAX_BPM,
                current + config::BPM_STEP
            ));

            std::cout << "\nBPM alterado para " << bpm.load() << '\n';
            wait_for_release(config::BUTTON_UP_GPIO);
        }

        if (digitalRead(config::BUTTON_DOWN_GPIO) == LOW) {
            int current = bpm.load();
            bpm.store(std::max(
                config::MIN_BPM,
                current - config::BPM_STEP
            ));

            std::cout << "\nBPM alterado para " << bpm.load() << '\n';
            wait_for_release(config::BUTTON_DOWN_GPIO);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void beep() {
    gpioSetPWMfrequency(
        config::BUZZER_GPIO,
        config::BUZZER_FREQUENCY_HZ
    );
    gpioSetPWMrange(config::BUZZER_GPIO, 100);
    gpioPWM(config::BUZZER_GPIO, 50);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config::BEEP_DURATION_MS)
    );

    gpioPWM(config::BUZZER_GPIO, 0);
}

void cleanup_outputs() {
    gpioWrite(config::LED_GPIO, PI_LOW);
    gpioPWM(config::BUZZER_GPIO, 0);
    gpioServo(config::SERVO_GPIO, 0);
}

}  // namespace

int main() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    try {
        GpioGuard gpio;
        configure_outputs();

        if (!configure_buttons()) {
            std::cerr << "Erro ao inicializar wiringPi para os botões.\n";
            return 1;
        }

        std::thread buttons(button_loop);

        bool left = true;
        auto next_beat = Clock::now();

        std::cout << "Metrônomo iniciado em "
                  << bpm.load()
                  << " BPM. Use Ctrl+C para encerrar.\n";

        while (running.load()) {
            const int current_bpm = bpm.load();
            const auto period = std::chrono::duration<double>(
                60.0 / static_cast<double>(current_bpm)
            );

            const auto scheduled = next_beat;
            const auto actual = Clock::now();
            const auto error_us =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    actual - scheduled
                ).count();

            gpioServo(
                config::SERVO_GPIO,
                left ? config::SERVO_LEFT_US : config::SERVO_RIGHT_US
            );
            left = !left;

            gpioWrite(config::LED_GPIO, PI_HIGH);
            beep();
            gpioWrite(config::LED_GPIO, PI_LOW);

            std::cout << "BPM: " << std::setw(3) << current_bpm
                      << " | período: " << std::fixed << std::setprecision(3)
                      << period.count() << " s"
                      << " | erro: " << error_us / 1000.0 << " ms\n";

            next_beat += std::chrono::duration_cast<Clock::duration>(period);

            const auto now = Clock::now();
            if (next_beat > now) {
                std::this_thread::sleep_until(next_beat);
            } else {
                const auto late_us =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        now - next_beat
                    ).count();

                std::cerr << "Prazo perdido por "
                          << late_us / 1000.0
                          << " ms\n";

                next_beat = Clock::now();
            }
        }

        if (buttons.joinable()) {
            buttons.join();
        }

        cleanup_outputs();
        std::cout << "Metrônomo encerrado.\n";
        return 0;

    } catch (const std::exception& error) {
        running.store(false);
        std::cerr << "Erro: " << error.what() << '\n';
        return 1;
    }
}
