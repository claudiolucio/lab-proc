#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
// ESP32-C3 Super Mini
// =====================================================
#define LDR_PIN 0
#define BUTTON_PIN 2

#define RGB_BUILTIN_PIN 8
#define NUM_PIXELS 1

Adafruit_NeoPixel rgbLed(NUM_PIXELS, RGB_BUILTIN_PIN, NEO_GRB + NEO_KHZ800);

// =====================================================
// Wi-Fi local: ESP32 como Access Point
// =====================================================
const char* ssid = "ESP32-C3-LDR";
const char* password = "12345678";

WebServer server(80);

// =====================================================
// Parâmetros
// =====================================================
const unsigned long LDR_READ_INTERVAL_MS = 1000;
const unsigned long YELLOW_BLINK_PERIOD_MS = 2000;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long RED_PRIORITY_TIME_MS = 3000;

int LOW_LIGHT_THRESHOLD = 1500;
bool lowLightWhenAdcIsHigh = true;

// =====================================================
// Estado global
// =====================================================
int ldrRaw = 0;
int ldrMilliVolts = 0;
bool lowLight = false;

unsigned long lastLdrReadMs = 0;

volatile bool buttonInterruptFlag = false;
volatile unsigned long lastInterruptMs = 0;

unsigned long redUntilMs = 0;

// =====================================================
// Interrupção do botão SOS
// =====================================================
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long now = millis();

  if (now - lastInterruptMs > DEBOUNCE_MS) {
    buttonInterruptFlag = true;
    lastInterruptMs = now;
  }
}

// =====================================================
// Controle do NeoPixel built-in
// =====================================================
void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  rgbLed.setPixelColor(0, rgbLed.Color(r, g, b));
  rgbLed.show();
}

void ledOff() {
  setRgb(0, 0, 0);
}

void ledYellow() {
  setRgb(255, 180, 0);
}

void ledRed() {
  setRgb(255, 0, 0);
}

void updateRgbLed() {
  unsigned long now = millis();

  // Prioridade máxima: botão SOS pressionado
  if (now < redUntilMs) {
    ledRed();
    return;
  }

  // Condição normal: baixa luminosidade
  if (!lowLight) {
    ledOff();
    return;
  }

  bool yellowOn = ((now / (YELLOW_BLINK_PERIOD_MS / 2)) % 2) == 0;

  if (yellowOn) {
    ledYellow();
  } else {
    ledOff();
  }
}

// =====================================================
// Tratamento do botão SOS
// =====================================================
void updateButton() {
  if (buttonInterruptFlag) {
    buttonInterruptFlag = false;

    // Confirma se o botão ainda está pressionado
    if (digitalRead(BUTTON_PIN) == LOW) {
      redUntilMs = millis() + RED_PRIORITY_TIME_MS;

      Serial.println("Botao SOS pressionado: NeoPixel vermelho por 3 segundos.");
    }
  }
}

// =====================================================
// Leitura do LDR via ADC
// =====================================================
void updateLdrReading() {
  unsigned long now = millis();

  if (now - lastLdrReadMs >= LDR_READ_INTERVAL_MS) {
    lastLdrReadMs = now;

    ldrRaw = analogRead(LDR_PIN);
    ldrMilliVolts = analogReadMilliVolts(LDR_PIN);

    if (lowLightWhenAdcIsHigh) {
      lowLight = (ldrRaw >= LOW_LIGHT_THRESHOLD);
    } else {
      lowLight = (ldrRaw <= LOW_LIGHT_THRESHOLD);
    }

    Serial.print("LDR raw: ");
    Serial.print(4096 - ldrRaw);
    Serial.print(" | mV: ");
    Serial.print(ldrMilliVolts);
    Serial.print(" | baixa luminosidade: ");
    Serial.println(lowLight ? "SIM" : "NAO");
  }
}

// =====================================================
// Interface Web
// =====================================================
String makeHtmlPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32-C3 LDR + SOS</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 32px;
      background-color: #f4f4f4;
    }

    .card {
      background-color: white;
      max-width: 560px;
      padding: 20px;
      border-radius: 8px;
      border: 1px solid #ccc;
    }

    .value {
      font-size: 1.4em;
      font-weight: bold;
    }

    .normal {
      color: green;
    }

    .alert {
      color: darkorange;
    }

    .danger {
      color: red;
    }
  </style>
</head>
<body>
  <h1>ESP32-C3 Super Mini - LDR + SOS</h1>

  <div class="card">
    <p>Valor ADC do LDR: <span id="ldrRaw" class="value">---</span></p>
    <p>Tensão aproximada: <span id="ldrMv" class="value">---</span> mV</p>
    <p>Limiar configurado: <span id="threshold" class="value">---</span></p>
    <p>Baixa luminosidade: <span id="lowLight" class="value">---</span></p>
    <p>Botão SOS ativo: <span id="sosActive" class="value">---</span></p>
    <p>LED RGB built-in: <span id="ledState" class="value">---</span></p>
  </div>

  <script>
    async function updateStatus() {
      try {
        const response = await fetch('/status');
        const data = await response.json();

        document.getElementById('ldrRaw').textContent = data.ldrRaw;
        document.getElementById('ldrMv').textContent = data.ldrMilliVolts;
        document.getElementById('threshold').textContent = data.threshold;

        const lowLightElement = document.getElementById('lowLight');
        lowLightElement.textContent = data.lowLight ? 'SIM' : 'NAO';
        lowLightElement.className = data.lowLight ? 'value alert' : 'value normal';

        const sosElement = document.getElementById('sosActive');
        sosElement.textContent = data.sosActive ? 'SIM' : 'NAO';
        sosElement.className = data.sosActive ? 'value danger' : 'value normal';

        const ledElement = document.getElementById('ledState');

        if (data.sosActive) {
          ledElement.textContent = 'VERMELHO - SOS';
          ledElement.className = 'value danger';
        } else if (data.lowLight) {
          ledElement.textContent = 'AMARELO PISCANDO';
          ledElement.className = 'value alert';
        } else {
          ledElement.textContent = 'APAGADO';
          ledElement.className = 'value normal';
        }

      } catch (error) {
        console.log(error);
      }
    }

    setInterval(updateStatus, 1000);
    updateStatus();
  </script>
</body>
</html>
)rawliteral";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", makeHtmlPage());
}

void handleStatus() {
  bool redPriorityActive = millis() < redUntilMs;

  String json = "{";
  json += "\"ldrRaw\":" + String(4096 - ldrRaw) + ",";
  json += "\"ldrMilliVolts\":" + String(ldrMilliVolts) + ",";
  json += "\"threshold\":" + String(LOW_LIGHT_THRESHOLD) + ",";
  json += "\"lowLight\":" + String(lowLight ? "true" : "false") + ",";
  json += "\"sosActive\":" + String(redPriorityActive ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  rgbLed.begin();
  rgbLed.setBrightness(40);
  ledOff();

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Access Point iniciado.");
  Serial.print("Rede Wi-Fi: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("Acesse no navegador: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("Webserver iniciado.");
}

// =====================================================
// Loop principal
// =====================================================
void loop() {
  server.handleClient();

  updateButton();
  updateLdrReading();
  updateRgbLed();
}