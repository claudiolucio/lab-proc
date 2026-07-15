#pragma once

namespace config {

// Numeração BCM, isto é, os números lógicos das GPIOs.
inline constexpr unsigned LED_GPIO = 18;
inline constexpr unsigned SERVO_GPIO = 12;
inline constexpr unsigned BUZZER_GPIO = 23;
inline constexpr unsigned BUTTON_UP_GPIO = 5;
inline constexpr unsigned BUTTON_DOWN_GPIO = 6;

inline constexpr int DEFAULT_BPM = 60;
inline constexpr int MIN_BPM = 30;
inline constexpr int MAX_BPM = 240;
inline constexpr int BPM_STEP = 5;

inline constexpr unsigned SERVO_LEFT_US = 1100;
inline constexpr unsigned SERVO_CENTER_US = 1500;
inline constexpr unsigned SERVO_RIGHT_US = 1900;

inline constexpr unsigned BUZZER_FREQUENCY_HZ = 1000;
inline constexpr unsigned BEEP_DURATION_MS = 80;

inline constexpr unsigned BUTTON_DEBOUNCE_MS = 30;

}  // namespace config
