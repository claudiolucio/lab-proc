#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
    using Clock = std::chrono::steady_clock;

    constexpr int beats = 60;
    constexpr int bpm = 60;

    const auto period = std::chrono::duration<double>(
        60.0 / static_cast<double>(bpm)
    );

    const auto start = Clock::now();
    auto next_beat = start;

    std::ofstream csv("data/jitter_cpp.csv");
    if (!csv) {
        std::cerr << "Não foi possível criar data/jitter_cpp.csv\n";
        return 1;
    }

    csv << "batimento,tempo_esperado_s,tempo_real_s,erro_ms\n";

    for (int beat = 1; beat <= beats; ++beat) {
        std::this_thread::sleep_until(next_beat);
        const auto actual = Clock::now();

        const double expected_s =
            std::chrono::duration<double>(next_beat - start).count();
        const double actual_s =
            std::chrono::duration<double>(actual - start).count();
        const double error_ms =
            std::chrono::duration<double, std::milli>(
                actual - next_beat
            ).count();

        csv << beat << ','
            << expected_s << ','
            << actual_s << ','
            << error_ms << '\n';

        std::cout << "Batimento " << std::setw(2) << beat
                  << " | erro: " << std::fixed << std::setprecision(3)
                  << error_ms << " ms\n";

        next_beat += std::chrono::duration_cast<Clock::duration>(period);
    }

    std::cout << "Arquivo criado: data/jitter_cpp.csv\n";
    return 0;
}
