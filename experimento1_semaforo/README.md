> 🏷️ **AI Attribution Notice**  
> *This documentation layout, instructions, and code configurations were structured and generated with the assistance of Generative AI tools.*

--- 

# ESP32-C3 Built-in NeoPixel Guide

Quick setup documentation to control the onboard addressable RGB LED using the Arduino IDE.

---

## 🛠️ Prerequisites

1. **Arduino IDE** installed (v2.0 or higher recommended).
2. **ESP32 Board Package** installed.
   * For a detailed step-by-step tutorial on how to install and configure the board package for the C3, follow the guide on [Random Nerd Tutorials - Getting Started with ESP32-C3 Super Mini](https://randomnerdtutorials.com/getting-started-esp32-c3-super-mini/).

---

## 📦 Step 1: Install the Library

1. Open the Arduino IDE.
2. Navigate to **Sketch** > **Include Library** > **Manage Libraries...**
3. Search for **Adafruit NeoPixel**.
4. Locate the library by **Adafruit** and click **Install**.

---

## 💻 Step 2: Source Code

Create a new sketch in your Arduino IDE, delete any default code, and paste the snippet below:

```cpp
/**
 * ESP32-C3 Built-in NeoPixel Color Cycle
 * Sequence: Red -> Green -> Yellow -> OFF
 * 
 * [Generated with AI Assistance]
 */

#include <Adafruit_NeoPixel.h>

// COMMON ESP32-C3 ONBOARD PINS:
// - ESP32-C3 Super Mini / Beetle: GPIO 8
// - Espressif DevKitM-1: GPIO 2
// - Seeed Studio XIAO C3: GPIO 20
#define LED_PIN   8 
#define NUMPIXELS 1

// Initialize the NeoPixel object
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixel.begin();            // Start the NeoPixel hardware
  pixel.setBrightness(50);  // Set safe brightness limit (0-255)
}

void loop() {
  // 1. GREEN 3s
  pixel.setPixelColor(0, pixel.Color(0, 255, 0));
  pixel.show();
  delay(3000);

  // 2. RED 4s
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));
  pixel.show();
  delay(4000);

  // 3. YELLOW (Red + Green) for 1s
  pixel.setPixelColor(0, pixel.Color(255, 255, 0));
  pixel.show();
  delay(1000);
}

```

---

## 🔌 Step 3: Board Configuration & Upload

1. Connect the ESP32-C3 board to your computer using a **data-capable USB-C cable**.
2. Set the board target: **Tools** > **Board** > **esp32** > select **ESP32C3 Dev Module** (or your specific model).
3. Set the communication port: **Tools** > **Port** > select the COM port matching your board.
4. Click the **Upload** arrow icon (`Ctrl + U`) to compile and flash the firmware.

---



