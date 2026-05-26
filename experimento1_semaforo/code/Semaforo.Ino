/*
  Utilizamos IA Generativa para auxiliar no uso da biblioteca Neopixel
*/
#include <Adafruit_NeoPixel.h>

#define LED_PIN   8 
#define NUMPIXELS 1

// Define the LED type and pin
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// the setup function runs once when you press reset or power the board
void setup() {
  pixel.begin();            // Initialize the NeoPixel
  pixel.setBrightness(50);
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

  // 3. YELLOW (Red + Green)
  pixel.setPixelColor(0, pixel.Color(255, 255, 0));
  pixel.show();
  delay(1000);
}
