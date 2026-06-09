#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "driver/ledc.h"
#include "esp_err.h"

// ============================
// CONFIGURAÇÕES DE PINOS
// ============================

#define LED_PIN    7
#define SERVO_PIN  5

// ============================
// CONFIGURAÇÕES DO ACCESS POINT
// ============================

const char* AP_SSID = "ESP32C3-PWM-LAB";
const char* AP_PASS = "12345678";  // mínimo 8 caracteres

WebServer server(80);

// ============================
// CONFIGURAÇÕES LEDC / PWM
// ============================

// No ESP32-C3, usar LEDC_LOW_SPEED_MODE.
const ledc_mode_t PWM_MODE = LEDC_LOW_SPEED_MODE;

// Timer e canal do LED
const ledc_timer_t   LED_TIMER   = LEDC_TIMER_0;
const ledc_channel_t LED_CHANNEL = LEDC_CHANNEL_0;

// Timer e canal do servo
const ledc_timer_t   SERVO_TIMER   = LEDC_TIMER_1;
const ledc_channel_t SERVO_CHANNEL = LEDC_CHANNEL_1;

// Resolução do LED: 10 bits = 0..1023.
// Boa para testar várias frequências sem estourar a combinação freq/resolução.
const uint8_t LED_RES_BITS = 10;

// Resolução do servo: 14 bits = 0..16383.
// Mais resolução melhora o ajuste da largura de pulso.
const uint8_t SERVO_RES_BITS = 14;

// Frequência inicial do LED
uint32_t ledFreqHz = 5000;

// Estado atual dos controles
int ledPercent = 0;
int servoAngle = 90;

// Servo comum: 50 Hz => período de 20 ms.
const uint32_t SERVO_FREQ_HZ = 50;
const uint32_t SERVO_PERIOD_US = 1000000UL / SERVO_FREQ_HZ;

// Ajuste conforme o servo real.
// Comece com 1000..2000 us para evitar forçar fim de curso.
const uint32_t SERVO_MIN_US = 1000;
const uint32_t SERVO_MAX_US = 2000;

// ============================
// FUNÇÕES AUXILIARES
// ============================

int clampInt(int value, int minValue, int maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

uint32_t dutyMax(uint8_t resolutionBits) {
  return (1UL << resolutionBits) - 1UL;
}

void printEspError(const char* context, esp_err_t err) {
  if (err != ESP_OK) {
    Serial.printf("[ERRO] %s: %s\n", context, esp_err_to_name(err));
  }
}

uint32_t servoPulseUsFromAngle(int angle) {
  angle = clampInt(angle, 0, 180);

  return SERVO_MIN_US +
         ((uint32_t)angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180UL;
}

uint32_t servoDutyFromAngle(int angle) {
  uint32_t pulseUs = servoPulseUsFromAngle(angle);

  return ((uint64_t)pulseUs * dutyMax(SERVO_RES_BITS)) / SERVO_PERIOD_US;
}

uint32_t ledDutyFromPercent(int percent) {
  percent = clampInt(percent, 0, 100);

  return ((uint64_t)percent * dutyMax(LED_RES_BITS)) / 100UL;
}

// ============================
// CONFIGURAÇÃO DO PWM
// ============================

void configureLedcTimer(ledc_timer_t timer,
                        uint32_t freqHz,
                        uint8_t resolutionBits) {
  ledc_timer_config_t timerConfig;
  memset(&timerConfig, 0, sizeof(timerConfig));

  timerConfig.speed_mode = PWM_MODE;
  timerConfig.timer_num = timer;
  timerConfig.duty_resolution = (ledc_timer_bit_t)resolutionBits;
  timerConfig.freq_hz = freqHz;
  timerConfig.clk_cfg = LEDC_AUTO_CLK;

  esp_err_t err = ledc_timer_config(&timerConfig);
  printEspError("ledc_timer_config", err);
}

void configureLedcChannel(ledc_channel_t channel,
                          ledc_timer_t timer,
                          int gpio,
                          uint32_t initialDuty) {
  ledc_channel_config_t channelConfig;
  memset(&channelConfig, 0, sizeof(channelConfig));

  channelConfig.gpio_num = gpio;
  channelConfig.speed_mode = PWM_MODE;
  channelConfig.channel = channel;
  channelConfig.intr_type = LEDC_INTR_DISABLE;
  channelConfig.timer_sel = timer;
  channelConfig.duty = initialDuty;
  channelConfig.hpoint = 0;

  esp_err_t err = ledc_channel_config(&channelConfig);
  printEspError("ledc_channel_config", err);
}

void applyLedPWM() {
  uint32_t duty = ledDutyFromPercent(ledPercent);

  esp_err_t err = ledc_set_duty(PWM_MODE, LED_CHANNEL, duty);
  printEspError("ledc_set_duty LED", err);

  err = ledc_update_duty(PWM_MODE, LED_CHANNEL);
  printEspError("ledc_update_duty LED", err);

  Serial.printf("[LED] brilho=%d%% freq=%lu Hz duty=%lu/%lu\n",
                ledPercent,
                (unsigned long)ledFreqHz,
                (unsigned long)duty,
                (unsigned long)dutyMax(LED_RES_BITS));
}

void applyServoPWM() {
  uint32_t pulseUs = servoPulseUsFromAngle(servoAngle);
  uint32_t duty = servoDutyFromAngle(servoAngle);

  esp_err_t err = ledc_set_duty(PWM_MODE, SERVO_CHANNEL, duty);
  printEspError("ledc_set_duty SERVO", err);

  err = ledc_update_duty(PWM_MODE, SERVO_CHANNEL);
  printEspError("ledc_update_duty SERVO", err);

  Serial.printf("[SERVO] angulo=%d graus pulso=%lu us duty=%lu/%lu\n",
                servoAngle,
                (unsigned long)pulseUs,
                (unsigned long)duty,
                (unsigned long)dutyMax(SERVO_RES_BITS));
}

bool setLedFrequency(uint32_t freqHz) {
  // Limites práticos para este experimento com resolução de 10 bits.
if (freqHz < 1) freqHz = 1;
if (freqHz > 20000) freqHz = 20000;

  esp_err_t err = ledc_set_freq(PWM_MODE, LED_TIMER, freqHz);

  if (err == ESP_OK) {
    ledFreqHz = ledc_get_freq(PWM_MODE, LED_TIMER);
    applyLedPWM();

    Serial.printf("[LED] frequência alterada para %lu Hz\n",
                  (unsigned long)ledFreqHz);

    return true;
  }

  printEspError("ledc_set_freq LED", err);
  return false;
}

void setupPWM() {
  configureLedcTimer(LED_TIMER, ledFreqHz, LED_RES_BITS);
  configureLedcChannel(LED_CHANNEL,
                       LED_TIMER,
                       LED_PIN,
                       ledDutyFromPercent(ledPercent));

  configureLedcTimer(SERVO_TIMER, SERVO_FREQ_HZ, SERVO_RES_BITS);
  configureLedcChannel(SERVO_CHANNEL,
                       SERVO_TIMER,
                       SERVO_PIN,
                       servoDutyFromAngle(servoAngle));

  applyLedPWM();
  applyServoPWM();
}

// ============================
// INTERFACE WEB
// ============================

String htmlPage() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">

  <title>ESP32-C3 PWM Lab</title>

  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 24px;
      background: #f5f5f5;
      color: #222;
    }

    .card {
      max-width: 620px;
      margin: auto;
      background: white;
      padding: 22px;
      border-radius: 12px;
      box-shadow: 0 2px 12px rgba(0,0,0,0.12);
    }

    h1 {
      font-size: 1.35rem;
      margin-top: 0;
    }

    label {
      display: block;
      margin-top: 22px;
      font-weight: bold;
    }

    input[type=range] {
      width: 100%;
    }

    input[type=number] {
      width: 120px;
      padding: 6px;
      font-size: 1rem;
    }

    .value {
      font-family: monospace;
      background: #eee;
      padding: 3px 6px;
      border-radius: 4px;
    }

    .status {
      margin-top: 22px;
      padding: 12px;
      background: #eef;
      border-radius: 8px;
      font-family: monospace;
    }

    .hint {
      font-size: 0.9rem;
      color: #555;
      margin-top: 6px;
    }
  </style>
</head>

<body>
  <div class="card">
    <h1>ESP32-C3 — Controle PWM de LED e Servo</h1>

    <label for="led">
      Brilho do LED:
      <span id="ledVal" class="value">--</span>
    </label>
    <input
      id="led"
      type="range"
      min="0"
      max="100"
      step="1"
      value="{{LED}}"
      oninput="sendUpdate()"
    >

    <label for="freq">
      Frequência do LED:
      <span id="freqVal" class="value">--</span>
    </label>
    <input
      id="freq"
      type="number"
      min="1"
      max="20000"
      step="1"
      value="{{FREQ}}"
      oninput="sendUpdate()"
    >
    <div class="hint">
      Sugestões de teste: 100, 500, 1000, 5000, 10000 Hz.
    </div>

    <label for="angle">
      Posição do servo:
      <span id="angleVal" class="value">--</span>
    </label>
    <input
      id="angle"
      type="range"
      min="0"
      max="180"
      step="1"
      value="{{ANGLE}}"
      oninput="sendUpdate()"
    >

    <div id="status" class="status">
      Aguardando comandos...
    </div>
  </div>

  <script>
    let debounceTimer = null;

    function byId(id) {
      return document.getElementById(id);
    }

    function updateLabels() {
      byId("ledVal").textContent = byId("led").value + "%";
      byId("freqVal").textContent = byId("freq").value + " Hz";
      byId("angleVal").textContent = byId("angle").value + "°";
    }

    function sendUpdate() {
      updateLabels();

      clearTimeout(debounceTimer);

      debounceTimer = setTimeout(() => {
        const led = byId("led").value;
        const freq = byId("freq").value;
        const angle = byId("angle").value;

        fetch(`/set?led=${led}&freq=${freq}&angle=${angle}`)
          .then(response => response.json())
          .then(data => {
            byId("status").textContent =
              `LED=${data.led}% | freq=${data.freq} Hz | servo=${data.angle}° | pulso=${data.servoPulseUs} us`;
          })
          .catch(() => {
            byId("status").textContent = "Falha na requisição HTTP.";
          });
      }, 100);
    }

    window.addEventListener("load", () => {
      updateLabels();
      fetch("/status")
        .then(response => response.json())
        .then(data => {
          byId("status").textContent =
            `LED=${data.led}% | freq=${data.freq} Hz | servo=${data.angle}° | pulso=${data.servoPulseUs} us`;
        });
    });
  </script>
</body>
</html>
)rawliteral";

  page.replace("{{LED}}", String(ledPercent));
  page.replace("{{FREQ}}", String(ledFreqHz));
  page.replace("{{ANGLE}}", String(servoAngle));

  return page;
}

String statusJson() {
  String json = "{";
  json += "\"led\":" + String(ledPercent) + ",";
  json += "\"freq\":" + String(ledFreqHz) + ",";
  json += "\"angle\":" + String(servoAngle) + ",";
  json += "\"servoPulseUs\":" + String(servoPulseUsFromAngle(servoAngle));
  json += "}";

  return json;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleStatus() {
  server.send(200, "application/json", statusJson());
}

void handleSet() {
  bool ok = true;

  if (server.hasArg("led")) {
    ledPercent = clampInt(server.arg("led").toInt(), 0, 100);
    applyLedPWM();
  }

  if (server.hasArg("freq")) {
    uint32_t requestedFreq = (uint32_t)server.arg("freq").toInt();

    if (!setLedFrequency(requestedFreq)) {
      ok = false;
    }
  }

  if (server.hasArg("angle")) {
    servoAngle = clampInt(server.arg("angle").toInt(), 0, 180);
    applyServoPWM();
  }

  server.send(ok ? 200 : 400, "application/json", statusJson());
}

void handleNotFound() {
  server.send(404, "text/plain", "Rota não encontrada.");
}

// ============================
// SETUP E LOOP
// ============================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Inicializando PWM...");
  setupPWM();

  Serial.println("Inicializando Access Point...");
  WiFi.mode(WIFI_AP);

  bool apOk = WiFi.softAP(AP_SSID, AP_PASS);

  if (!apOk) {
    Serial.println("[ERRO] Falha ao iniciar Wi-Fi AP.");
  } else {
    Serial.println("[OK] Access Point iniciado.");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Senha: ");
    Serial.println(AP_PASS);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[OK] Webserver iniciado.");
}

void loop() {
  // Não usar delays longos aqui.
  // O PWM é gerado por hardware; o loop fica livre para atender HTTP.
  server.handleClient();
  yield();
}