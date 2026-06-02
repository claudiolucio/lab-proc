#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

const char* ssid = "ESP32_CALCULADORA_TESTES";
const char* password = "12345678";

WebServer server(80);

const uint8_t SAMPLE_COUNT = 5;

volatile uint64_t sink64 = 0;

enum OperationKind {
  OP_INVALID,
  OP_MUL,
  OP_FACT
};

struct OperationResult {
  bool valid;
  String exactText;
  uint64_t bitsValue;
  bool overflow;
};

struct TestCase {
  const char* name;
  const char* op;
  uint8_t bits;
  int64_t a;
  int64_t b;
  bool expectValid;
  int64_t expectExact;
  bool expectOverflow;
};

struct BenchmarkCase {
  uint8_t bits;
  int64_t mulA;
  int64_t mulB;
  int64_t factA;
};

uint64_t maskForBits(uint8_t bits) {
  if (bits >= 64) {
    return 0xFFFFFFFFFFFFFFFFULL;
  }

  return (1ULL << bits) - 1ULL;
}

int64_t minSigned(uint8_t bits) {
  if (bits >= 64) {
    return INT64_MIN;
  }

  return -(1LL << (bits - 1));
}

int64_t maxSigned(uint8_t bits) {
  if (bits >= 64) {
    return INT64_MAX;
  }

  return (1LL << (bits - 1)) - 1LL;
}

bool validBitWidth(uint8_t bits) {
  return bits == 4 || bits == 8 || bits == 12 || bits == 16 || bits == 32 || bits == 64;
}

OperationKind operationKindFromString(const String &op) {
  if (op == "mul") {
    return OP_MUL;
  }

  if (op == "fact") {
    return OP_FACT;
  }

  return OP_INVALID;
}

String int64ToString(int64_t value) {
  char buffer[24];
  snprintf(buffer, sizeof(buffer), "%lld", (long long)value);
  return String(buffer);
}

String uint64ToStringValue(uint64_t value) {
  if (value == 0) {
    return "0";
  }

  char buffer[24];
  int index = sizeof(buffer) - 1;
  buffer[index] = '\0';

  while (value > 0 && index > 0) {
    index--;
    buffer[index] = '0' + (int)(value % 10);
    value /= 10;
  }

  return String(&buffer[index]);
}

int64_t parseInt64(String text) {
  text.trim();
  return strtoll(text.c_str(), NULL, 10);
}

uint64_t absMagnitude(int64_t value) {
  if (value >= 0) {
    return (uint64_t)value;
  }

  return (uint64_t)(-(value + 1)) + 1ULL;
}

String multiplyPositiveDecimalStrings(String a, String b) {
  const int maxDigits = 48;
  uint16_t digits[maxDigits];

  for (int i = 0; i < maxDigits; i++) {
    digits[i] = 0;
  }

  for (int i = a.length() - 1; i >= 0; i--) {
    for (int j = b.length() - 1; j >= 0; j--) {
      int pos = (a.length() - 1 - i) + (b.length() - 1 - j);
      digits[pos] += (a.charAt(i) - '0') * (b.charAt(j) - '0');
    }
  }

  for (int i = 0; i < maxDigits - 1; i++) {
    digits[i + 1] += digits[i] / 10;
    digits[i] %= 10;
  }

  int top = maxDigits - 1;

  while (top > 0 && digits[top] == 0) {
    top--;
  }

  String result = "";

  for (int i = top; i >= 0; i--) {
    result += char('0' + digits[i]);
  }

  return result;
}

String multiplyDecimalText(int64_t a, int64_t b) {
  bool negative = (a < 0) != (b < 0);
  String result = multiplyPositiveDecimalStrings(
    uint64ToStringValue(absMagnitude(a)),
    uint64ToStringValue(absMagnitude(b))
  );

  if (negative && result != "0") {
    result = "-" + result;
  }

  return result;
}

bool multiplicationOverflowsSignedN(int64_t a, int64_t b, uint8_t bits) {
  int64_t minValue = minSigned(bits);
  int64_t maxValue = maxSigned(bits);

  if (a == 0 || b == 0) {
    return false;
  }

  if (a > 0) {
    if (b > 0) {
      return a > maxValue / b;
    }

    return b < minValue / a;
  }

  if (b > 0) {
    return a < minValue / b;
  }

  if (a == INT64_MIN || b == INT64_MIN) {
    return true;
  }

  return -a > maxValue / -b;
}

bool int64FitsSignedN(int64_t value, uint8_t bits) {
  return value >= minSigned(bits) && value <= maxSigned(bits);
}

uint64_t multiplyModuloBits(int64_t a, int64_t b, uint8_t bits) {
  uint64_t mask = maskForBits(bits);
  uint64_t multiplicand = (uint64_t)a & mask;
  uint64_t multiplier = (uint64_t)b & mask;
  uint64_t result = 0;

  for (uint8_t i = 0; i < bits; i++) {
    if (multiplier & 1ULL) {
      result = (result + multiplicand) & mask;
    }

    multiplicand = (multiplicand << 1) & mask;
    multiplier >>= 1;
  }

  return result;
}

String binN(uint64_t value, uint8_t bits) {
  uint64_t masked = value & maskForBits(bits);
  String s = "";

  for (int16_t i = bits - 1; i >= 0; i--) {
    s += ((masked >> i) & 1ULL) ? "1" : "0";
  }

  return s;
}

int64_t factorialIterative(int64_t n) {
  if (n <= 1) {
    return 1;
  }

  int64_t result = 1;

  for (int64_t i = 2; i <= n; i++) {
    result *= i;
  }

  return result;
}

bool factorialValue(int64_t value, int64_t &result) {
  if (value < 0 || value > 20) {
    return false;
  }

  result = factorialIterative(value);
  return true;
}

OperationResult calculateOperation(OperationKind op, uint8_t bits, int64_t a, int64_t b) {
  OperationResult r;
  r.valid = true;
  r.exactText = "0";
  r.bitsValue = 0;
  r.overflow = false;

  if (!validBitWidth(bits)) {
    r.valid = false;
    return r;
  }

  if (a < minSigned(bits) || a > maxSigned(bits) || b < minSigned(bits) || b > maxSigned(bits)) {
    r.valid = false;
    return r;
  }

  if (op == OP_MUL) {
    r.exactText = multiplyDecimalText(a, b);
    r.bitsValue = multiplyModuloBits(a, b, bits);
    r.overflow = multiplicationOverflowsSignedN(a, b, bits);
  } else if (op == OP_FACT) {
    int64_t factorialResult;

    if (!factorialValue(a, factorialResult)) {
      r.valid = false;
      return r;
    }

    r.exactText = int64ToString(factorialResult);
    r.bitsValue = (uint64_t)factorialResult & maskForBits(bits);
    r.overflow = !int64FitsSignedN(factorialResult, bits);
  } else {
    r.valid = false;
    return r;
  }

  return r;
}

OperationResult calculateOperation(const String &op, uint8_t bits, int64_t a, int64_t b) {
  return calculateOperation(operationKindFromString(op), bits, a, b);
}

float measureMultiplicationMicros(int64_t a, int64_t b, uint8_t bits) {
  uint32_t start = micros();

  sink64 = multiplyModuloBits(a, b, bits);

  uint32_t elapsed = micros() - start;
  return (float)elapsed;
}

float measureFactorialMicros(int64_t a) {
  uint32_t start = micros();

  sink64 = (uint64_t)factorialIterative(a);

  uint32_t elapsed = micros() - start;
  return (float)elapsed;
}

float measureOperationMicros(const String &op, uint8_t bits, int64_t a, int64_t b) {
  if (operationKindFromString(op) == OP_MUL) {
    return measureMultiplicationMicros(a, b, bits);
  }

  return measureFactorialMicros(a);
}

void appendStatsJson(String &json, const char* op, uint8_t bits, int64_t a, int64_t b) {
  float samples[SAMPLE_COUNT];
  float sum = 0.0;

  OperationKind kind = operationKindFromString(String(op));

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    if (kind == OP_MUL) {
      samples[i] = measureMultiplicationMicros(a, b, bits);
    } else {
      samples[i] = measureFactorialMicros(a);
    }

    sum += samples[i];
    delay(5);
  }

  float mean = sum / SAMPLE_COUNT;
  float variance = 0.0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    float delta = samples[i] - mean;
    variance += delta * delta;
  }

  float stddev = sqrt(variance / (SAMPLE_COUNT - 1));
  OperationResult r = calculateOperation(String(op), bits, a, b);

  json += "{";
  json += "\"op\":\"" + String(op) + "\",";
  json += "\"bits\":" + String(bits) + ",";
  json += "\"a\":\"" + int64ToString(a) + "\",";
  json += "\"b\":\"" + int64ToString(b) + "\",";
  json += "\"exact\":\"" + r.exactText + "\",";
  json += "\"resultBits\":\"" + binN(r.bitsValue, bits) + "\",";
  json += "\"overflow\":" + String(r.overflow ? "true" : "false") + ",";
  json += "\"meanMicros\":" + String(mean, 4) + ",";
  json += "\"stddevMicros\":" + String(stddev, 4) + ",";
  json += "\"samples\":[";

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    if (i > 0) {
      json += ",";
    }

    json += String(samples[i], 4);
  }

  json += "]}";
}

const TestCase TESTS[] = {
  {"mul_4_ok", "mul", 4, 3, 2, true, 6, false},
  {"mul_4_overflow", "mul", 4, 7, 2, true, 14, true},
  {"mul_4_negativo", "mul", 4, -1, 3, true, -3, false},
  {"mul_8_ok", "mul", 8, 12, 10, true, 120, false},
  {"mul_8_overflow", "mul", 8, 100, 3, true, 300, true},
  {"fact_4_ok", "fact", 4, 3, 0, true, 6, false},
  {"fact_4_overflow", "fact", 4, 4, 0, true, 24, true},
  {"fact_8_ok", "fact", 8, 5, 0, true, 120, false},
  {"fact_8_overflow", "fact", 8, 6, 0, true, 720, true},
  {"fact_negativo_invalido", "fact", 4, -1, 0, false, 0, false}
};

const uint8_t TEST_COUNT = sizeof(TESTS) / sizeof(TESTS[0]);

const BenchmarkCase BENCHMARK_CASES[] = {
  {4, 7, 7, 7},
  {8, 127, 127, 127},
  {12, 2047, 2047, 2047},
  {16, 32767, 32767, 32767},
  {32, 2147483647LL, 2147483647LL, 32767},
  {64, 9223372036854775807LL, 9223372036854775807LL, 0}
};

const uint8_t BENCHMARK_CASE_COUNT = sizeof(BENCHMARK_CASES) / sizeof(BENCHMARK_CASES[0]);

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Benchmark ESP32</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 24px; background: #f3f3f3; color: #202020; }
    .wrap { max-width: 1120px; margin: 0 auto; }
    .panel { background: #fff; border: 1px solid #d8d8d8; border-radius: 8px; padding: 16px; margin-bottom: 16px; }
    .grid { display: grid; grid-template-columns: repeat(5, minmax(120px, 1fr)); gap: 10px; align-items: end; }
    label { display: block; font-size: 13px; margin-bottom: 4px; }
    input, select, button { width: 100%; box-sizing: border-box; font-size: 16px; padding: 8px; }
    button { cursor: pointer; }
    table { width: 100%; border-collapse: collapse; background: white; }
    th, td { border: 1px solid #d0d0d0; padding: 6px; text-align: left; font-size: 13px; }
    th { background: #eeeeee; }
    pre { white-space: pre-wrap; background: #eeeeee; padding: 10px; border-radius: 6px; }
    .ok { color: #116611; font-weight: bold; }
    .fail { color: #aa1111; font-weight: bold; }
  </style>
</head>
<body>
  <div class="wrap">
    <h2>Testes e benchmark de multiplicacao/fatorial no ESP32</h2>

    <div class="panel">
      <h3>Calculo manual</h3>
      <div class="grid">
        <div>
          <label>Bits</label>
          <select id="bits">
            <option>4</option>
            <option>8</option>
            <option>12</option>
            <option>16</option>
            <option>32</option>
            <option>64</option>
          </select>
        </div>
        <div>
          <label>Operacao</label>
          <select id="op">
            <option value="mul">Multiplicacao</option>
            <option value="fact">Fatorial</option>
          </select>
        </div>
        <div>
          <label>A decimal</label>
          <input id="a" type="text" value="3">
        </div>
        <div>
          <label>B decimal</label>
          <input id="b" type="text" value="2">
        </div>
        <button onclick="calculate()">Calcular</button>
      </div>
      <pre id="manual">Aguardando calculo...</pre>
    </div>

    <div class="panel">
      <h3>Casos planejados</h3>
      <button onclick="runTests()">Executar testes planejados</button>
      <div id="tests"></div>
    </div>

    <div class="panel">
      <h3>Benchmark experimental</h3>
      <button onclick="runBench()">Executar benchmark</button>
      <p>Cada largura usa uma multiplicacao por soma/deslocamento e um fatorial representativos, com 5 amostras medidas no ESP32.</p>
      <div id="bench"></div>
    </div>
  </div>

  <script>
    async function getJson(path) {
      const response = await fetch(path);
      return response.json();
    }

    async function calculate() {
      const params = new URLSearchParams();
      params.append("bits", document.getElementById("bits").value);
      params.append("op", document.getElementById("op").value);
      params.append("a", document.getElementById("a").value);
      params.append("b", document.getElementById("b").value);

      const data = await getJson("/calc?" + params.toString());

      if (data.error) {
        document.getElementById("manual").textContent = "Erro: " + data.error;
        return;
      }

      document.getElementById("manual").textContent =
        "bits = " + data.bits + "\n" +
        "operacao = " + data.op + "\n" +
        "A = " + data.a + "\n" +
        "B = " + (data.op === "fact" ? "ignorado" : data.b) + "\n" +
        "resultado exato = " + data.exact + "\n" +
        "resultado em bits = " + data.resultBits + "\n" +
        "overflow = " + (data.overflow ? "SIM" : "NAO") + "\n" +
        "tempo medio = " + data.meanMicros + " us";
    }

    async function runTests() {
      const data = await getJson("/tests");
      let html = "<table><thead><tr><th>Caso</th><th>Op</th><th>Bits</th><th>A</th><th>B</th><th>Exato</th><th>Bits</th><th>Overflow</th><th>Status</th></tr></thead><tbody>";

      data.tests.forEach((row) => {
        html += "<tr>" +
          "<td>" + row.name + "</td>" +
          "<td>" + row.op + "</td>" +
          "<td>" + row.bits + "</td>" +
          "<td>" + row.a + "</td>" +
          "<td>" + (row.op === "fact" ? "-" : row.b) + "</td>" +
          "<td>" + (row.valid ? row.exact : "invalido") + "</td>" +
          "<td>" + (row.valid ? row.resultBits : "-") + "</td>" +
          "<td>" + (row.valid ? (row.overflow ? "SIM" : "NAO") : "-") + "</td>" +
          "<td class='" + (row.pass ? "ok" : "fail") + "'>" + (row.pass ? "PASSOU" : "FALHOU") + "</td>" +
          "</tr>";
      });

      html += "</tbody></table>";
      document.getElementById("tests").innerHTML = html;
    }

    async function runBench() {
      document.getElementById("bench").textContent = "Executando...";
      const data = await getJson("/bench");
      let html = "<table><thead><tr><th>Op</th><th>Bits</th><th>A</th><th>B</th><th>Exato</th><th>Bits</th><th>Overflow</th><th>Media us</th><th>Desvio us</th><th>Amostras us</th></tr></thead><tbody>";
      let csv = "op,bits,a,b,exact,resultBits,overflow,meanMicros,stddevMicros,samples\n";

      data.results.forEach((row) => {
        const sampleText = row.samples.join(" | ");
        html += "<tr>" +
          "<td>" + row.op + "</td>" +
          "<td>" + row.bits + "</td>" +
          "<td>" + row.a + "</td>" +
          "<td>" + (row.op === "fact" ? "-" : row.b) + "</td>" +
          "<td>" + row.exact + "</td>" +
          "<td>" + row.resultBits + "</td>" +
          "<td>" + (row.overflow ? "SIM" : "NAO") + "</td>" +
          "<td>" + row.meanMicros + "</td>" +
          "<td>" + row.stddevMicros + "</td>" +
          "<td>" + sampleText + "</td>" +
          "</tr>";
        csv += [row.op, row.bits, row.a, row.b, row.exact, row.resultBits, row.overflow, row.meanMicros, row.stddevMicros, row.samples.join("|")].join(",") + "\n";
      });

      html += "</tbody></table><h4>CSV para o relatorio</h4><pre>" + csv + "</pre>";
      document.getElementById("bench").innerHTML = html;
    }
  </script>
</body>
</html>
)rawliteral";

void sendJsonError(const char* message) {
  String json = "{\"error\":\"";
  json += message;
  json += "\"}";
  server.send(400, "application/json", json);
}

void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleCalc() {
  if (!server.hasArg("bits") || !server.hasArg("op") || !server.hasArg("a") || !server.hasArg("b")) {
    sendJsonError("Parametros ausentes");
    return;
  }

  uint8_t bits = server.arg("bits").toInt();
  String op = server.arg("op");
  int64_t a = parseInt64(server.arg("a"));
  int64_t b = parseInt64(server.arg("b"));
  OperationResult r = calculateOperation(op, bits, a, b);

  if (!r.valid) {
    sendJsonError("Operacao, largura ou operandos invalidos");
    return;
  }

  float meanMicros = measureOperationMicros(op, bits, a, b);

  String json = "{";
  json += "\"op\":\"" + op + "\",";
  json += "\"bits\":" + String(bits) + ",";
  json += "\"a\":\"" + int64ToString(a) + "\",";
  json += "\"b\":\"" + int64ToString(b) + "\",";
  json += "\"exact\":\"" + r.exactText + "\",";
  json += "\"resultBits\":\"" + binN(r.bitsValue, bits) + "\",";
  json += "\"overflow\":" + String(r.overflow ? "true" : "false") + ",";
  json += "\"meanMicros\":" + String(meanMicros, 4);
  json += "}";

  server.send(200, "application/json", json);
}

void handleTests() {
  String json = "{\"tests\":[";

  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    const TestCase &tc = TESTS[i];
    OperationResult r = calculateOperation(String(tc.op), tc.bits, tc.a, tc.b);
    bool pass = r.valid == tc.expectValid;

  if (tc.expectValid) {
      pass = pass && r.exactText == int64ToString(tc.expectExact) && r.overflow == tc.expectOverflow;
    }

    if (i > 0) {
      json += ",";
    }

    json += "{";
    json += "\"name\":\"" + String(tc.name) + "\",";
    json += "\"op\":\"" + String(tc.op) + "\",";
    json += "\"bits\":" + String(tc.bits) + ",";
    json += "\"a\":\"" + int64ToString(tc.a) + "\",";
    json += "\"b\":\"" + int64ToString(tc.b) + "\",";
    json += "\"valid\":" + String(r.valid ? "true" : "false") + ",";
    json += "\"exact\":\"" + r.exactText + "\",";
    json += "\"resultBits\":\"" + binN(r.bitsValue, tc.bits) + "\",";
    json += "\"overflow\":" + String(r.overflow ? "true" : "false") + ",";
    json += "\"pass\":" + String(pass ? "true" : "false");
    json += "}";
  }

  json += "]}";
  server.send(200, "application/json", json);
}

void handleBench() {
  String json = "{\"sampleCount\":" + String(SAMPLE_COUNT) + ",";
  json += "\"results\":[";
  bool first = true;

  for (uint8_t i = 0; i < BENCHMARK_CASE_COUNT; i++) {
    const BenchmarkCase &bc = BENCHMARK_CASES[i];

    if (!first) {
      json += ",";
    }

    appendStatsJson(json, "mul", bc.bits, bc.mulA, bc.mulB);
    first = false;

    json += ",";
    appendStatsJson(json, "fact", bc.bits, bc.factA, 0);
  }

  json += "]}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Benchmark multiplicacao/fatorial iniciado.");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/calc", handleCalc);
  server.on("/tests", handleTests);
  server.on("/bench", handleBench);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}
