/*
  Semáforo com NeoPixel built-in, LDR e botão de pedestre.
  Utilizamos IA Generativa para auxiliar no uso da biblioteca NeoPixel.
*/

#include <Adafruit_NeoPixel.h>

// =====================================================
// Pinos
// =====================================================
#define LED_PIN 8
#define NUMPIXELS 1

#define LDR_PIN 0
#define BUTTON_PIN 2

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// =====================================================
// Tempos do semáforo
// =====================================================
const unsigned long GREEN_TIME_MS = 3000;
const unsigned long YELLOW_TIME_MS = 1000;
const unsigned long RED_TIME_MS = 4000;

// Modo noturno: pisca amarelo a cada 1 segundo
const unsigned long NIGHT_BLINK_MS = 1000;

// Debounce do botão
const unsigned long DEBOUNCE_MS = 50;

// Limiar do LDR
int LOW_LIGHT_THRESHOLD = 1500;

// Montagem sugerida:
// 3.3V -> resistor 10k -> GPIO0/ADC -> LDR -> GND
// Nesse caso, baixa luminosidade tende a gerar ADC maior.
bool lowLightWhenAdcIsHigh = true;

// =====================================================
// Estados
// =====================================================
enum TrafficState {
  GREEN,
  YELLOW,
  RED
};

TrafficState currentState = RED;

unsigned long stateStartMs = 0;
unsigned long lastBlinkMs = 0;

bool nightMode = false;
bool previousNightMode = false;
bool yellowBlinkOn = false;

volatile bool buttonInterruptFlag = false;
volatile unsigned long lastInterruptMs = 0;

bool pedestrianRequest = false;

// =====================================================
// NeoPixel
// =====================================================
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void ledGreen() {
  setColor(0, 255, 0);
}

void ledYellow() {
  setColor(255, 255, 0);
}

void ledRed() {
  setColor(255, 0, 0);
}

void ledOff() {
  setColor(0, 0, 0);
}

// =====================================================
// Interrupção com debounce
// =====================================================
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long now = millis();

  if (now - lastInterruptMs > DEBOUNCE_MS) {
    buttonInterruptFlag = true;
    lastInterruptMs = now;
  }
}

// =====================================================
// Botão de pedestre
// =====================================================
void updateButton() {
  if (buttonInterruptFlag) {
    buttonInterruptFlag = false;

    if (digitalRead(BUTTON_PIN) == LOW) {
      pedestrianRequest = true;
      Serial.println("Solicitacao de travessia registrada.");
    }
  }
}

// =====================================================
// LDR e modo noturno
// =====================================================
void updateLdr() {
  int ldrRaw = analogRead(LDR_PIN);

  previousNightMode = nightMode;

  if (lowLightWhenAdcIsHigh) {
    nightMode = ldrRaw >= LOW_LIGHT_THRESHOLD;
  } else {
    nightMode = ldrRaw <= LOW_LIGHT_THRESHOLD;
  }

  // Ao sair do modo noturno, volta sempre para vermelho por segurança
  if (previousNightMode == true && nightMode == false) {
    currentState = RED;
    stateStartMs = millis();
    pedestrianRequest = false;

    Serial.println("Saida do modo noturno: voltando para vermelho por seguranca.");
  }

  Serial.print("LDR: ");
  Serial.print(ldrRaw);
  Serial.print(" | Modo noturno: ");
  Serial.println(nightMode ? "SIM" : "NAO");
}

// =====================================================
// Modo noturno
// =====================================================
void updateNightMode() {
  unsigned long now = millis();

  if (now - lastBlinkMs >= NIGHT_BLINK_MS) {
    lastBlinkMs = now;
    yellowBlinkOn = !yellowBlinkOn;

    if (yellowBlinkOn) {
      ledYellow();
    } else {
      ledOff();
    }
  }
}

// =====================================================
// Semáforo normal
// =====================================================
void updateTrafficLight() {
  unsigned long now = millis();
  unsigned long elapsed = now - stateStartMs;

  switch (currentState) {
    case GREEN:
      ledGreen();

      if (pedestrianRequest || elapsed >= GREEN_TIME_MS) {
        pedestrianRequest = false;
        currentState = YELLOW;
        stateStartMs = now;
      }
      break;

    case YELLOW:
      ledYellow();

      if (elapsed >= YELLOW_TIME_MS) {
        currentState = RED;
        stateStartMs = now;
      }
      break;

    case RED:
      ledRed();

      if (elapsed >= RED_TIME_MS) {
        currentState = GREEN;
        stateStartMs = now;
      }
      break;
  }
}

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);

  pixel.begin();
  pixel.setBrightness(50);
  ledOff();

  pinMode(LDR_PIN, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  currentState = RED;
  stateStartMs = millis();

  Serial.println("Semaforo iniciado em vermelho.");
}

// =====================================================
// Loop
// =====================================================
void loop() {
  updateButton();
  updateLdr();

  if (nightMode) {
    updateNightMode();
  } else {
    updateTrafficLight();
  }

  delay(100);
}