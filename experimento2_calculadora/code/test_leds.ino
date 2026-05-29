const int LED_PINS[4] = {9, 8, 7, 6};

void writeLeds(uint8_t value) {
  value &= 0x0F;

  for (int i = 0; i < 4; i++) {
    int bitValue = (value >> i) & 1;
    digitalWrite(LED_PINS[i], bitValue ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  Serial.println("Teste dos LEDs iniciado.");
}

void loop() {
  for (uint8_t value = 0; value < 16; value++) {
    Serial.print("Valor decimal: ");
    Serial.print(value);
    Serial.print(" | Binario: ");

    for (int i = 3; i >= 0; i--) {
      Serial.print((value >> i) & 1);
    }

    Serial.println();

    writeLeds(value);
    delay(800);
  }
}