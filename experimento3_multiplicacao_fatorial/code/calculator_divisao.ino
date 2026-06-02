#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_CALCULADORA";
const char* password = "12345678";

WebServer server(80);

const int LED_PINS[4] = {6, 7, 8, 9};

void writeLeds(uint8_t value) {
  value &= 0x0F;

  for (int i = 0; i < 4; i++) {
    int bitValue = (value >> i) & 1;
    digitalWrite(LED_PINS[i], bitValue ? HIGH : LOW);
  }
}

String bin4(uint8_t value) {
  value &= 0x0F;
  String s = "";

  for (int i = 3; i >= 0; i--) {
    s += ((value >> i) & 1) ? "1" : "0";
  }

  return s;
}

bool parseBin4(String text, uint8_t &value) {
  text.trim();

  if (text.length() != 4) {
    return false;
  }

  value = 0;

  for (int i = 0; i < 4; i++) {
    char c = text.charAt(i);

    if (c != '0' && c != '1') {
      return false;
    }

    value = (value << 1) | (c - '0');
  }

  value &= 0x0F;
  return true;
}

int8_t signed4(uint8_t value) {
  value &= 0x0F;

  if (value & 0x08) {
    return value - 16;
  }

  return value;
}

bool overflowAdd(uint8_t a, uint8_t b, uint8_t result) {
  bool signA = a & 0x08;
  bool signB = b & 0x08;
  bool signR = result & 0x08;

  return (signA == signB) && (signR != signA);
}

bool overflowSub(uint8_t a, uint8_t b, uint8_t result) {
  bool signA = a & 0x08;
  bool signB = b & 0x08;
  bool signR = result & 0x08;

  return (signA != signB) && (signR != signA);
}

int16_t multiply4(uint8_t a, uint8_t b) {
  return (int16_t)signed4(a) * (int16_t)signed4(b);
}

bool divide4(uint8_t a, uint8_t b, int16_t &quotient, int16_t &remainder) {
  int8_t valA = signed4(a);
  int8_t valB = signed4(b);

  if (valB == 0) {
    return false;
  }

  quotient = valA / valB;
  remainder = valA % valB;

  return true;
}

bool factorial4(uint8_t a, uint16_t &exactResult) {
  int8_t value = signed4(a);

  if (value < 0) {
    return false;
  }

  exactResult = 1;

  for (int8_t i = 2; i <= value; i++) {
    exactResult *= i;
  }

  return true;
}

bool fitsSigned4(int16_t value) {
  return value >= -8 && value <= 7;
}

uint32_t benchmarkAdd(uint8_t bits, uint32_t iterations, volatile int32_t &checksum) {
  int32_t maxValue;

  if (bits == 32) {
    maxValue = 2147483647;
  } else {
    maxValue = (1L << (bits - 1)) - 1;
  }

  volatile int32_t a = maxValue;
  volatile int32_t b = 1;

  uint32_t start = micros();

  for (uint32_t i = 0; i < iterations; i++) {
    checksum += a + b + (int32_t)i;
  }

  uint32_t elapsed = micros() - start;
  return elapsed;
}

uint32_t benchmarkSub(uint8_t bits, uint32_t iterations, volatile int32_t &checksum) {
  int32_t maxValue;

  if (bits == 32) {
    maxValue = 2147483647;
  } else {
    maxValue = (1L << (bits - 1)) - 1;
  }

  volatile int32_t a = maxValue;
  volatile int32_t b = 1;

  uint32_t start = micros();

  for (uint32_t i = 0; i < iterations; i++) {
    checksum += a - b - (int32_t)i;
  }

  uint32_t elapsed = micros() - start;
  return elapsed;
}

uint32_t benchmarkMul(uint8_t bits, uint32_t iterations, volatile int32_t &checksum) {
  int32_t a;
  int32_t b;

  if (bits == 4) {
    a = 7;
    b = -3;
  } else if (bits == 8) {
    a = 127;
    b = -3;
  } else if (bits == 16) {
    a = 32767;
    b = -3;
  } else {
    a = 46340;
    b = -3;
  }

  volatile int32_t va = a;
  volatile int32_t vb = b;

  uint32_t start = micros();

  for (uint32_t i = 0; i < iterations; i++) {
    checksum += va * vb + (int32_t)i;
  }

  uint32_t elapsed = micros() - start;
  return elapsed;
}

uint32_t benchmarkDiv(uint8_t bits, uint32_t iterations, volatile int32_t &checksum) {
  int32_t a;
  int32_t b;

  if (bits == 4) {
    a = 7;
    b = -2;
  } else if (bits == 8) {
    a = 127;
    b = -3;
  } else if (bits == 16) {
    a = 32767;
    b = -7;
  } else {
    a = 2147483647;
    b = -11;
  }

  volatile int32_t va = a;
  volatile int32_t vb = b;

  uint32_t start = micros();

  for (uint32_t i = 0; i < iterations; i++) {
    checksum += va / vb + (int32_t)i;
  }

  uint32_t elapsed = micros() - start;
  return elapsed;
}

String benchmarkJsonForBits(uint8_t bits, uint32_t iterations) {
  volatile int32_t checksum = 0;

  uint32_t addTime = benchmarkAdd(bits, iterations, checksum);
  uint32_t subTime = benchmarkSub(bits, iterations, checksum);
  uint32_t mulTime = benchmarkMul(bits, iterations, checksum);
  uint32_t divTime = benchmarkDiv(bits, iterations, checksum);

  String json = "{";
  json += "\"bits\":" + String(bits) + ",";
  json += "\"iterations\":" + String(iterations) + ",";
  json += "\"addTotalUs\":" + String(addTime) + ",";
  json += "\"subTotalUs\":" + String(subTime) + ",";
  json += "\"mulTotalUs\":" + String(mulTime) + ",";
  json += "\"divTotalUs\":" + String(divTime) + ",";
  json += "\"addAvgUs\":" + String((double)addTime / iterations, 6) + ",";
  json += "\"subAvgUs\":" + String((double)subTime / iterations, 6) + ",";
  json += "\"mulAvgUs\":" + String((double)mulTime / iterations, 6) + ",";
  json += "\"divAvgUs\":" + String((double)divTime / iterations, 6) + ",";
  json += "\"checksum\":" + String((int32_t)checksum);
  json += "}";

  return json;
}

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Calculadora 4 bits</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 30px;
      background: #f3f3f3;
    }

    .card {
      max-width: 720px;
      padding: 20px;
      border-radius: 10px;
      background: white;
      box-shadow: 0 0 10px rgba(0,0,0,0.2);
    }

    input, select, button {
      width: 100%;
      box-sizing: border-box;
      font-size: 18px;
      padding: 8px;
      margin: 6px 0 14px 0;
    }

    button {
      cursor: pointer;
    }

    pre {
      background: #eeeeee;
      padding: 12px;
      border-radius: 6px;
      font-size: 16px;
      overflow-x: auto;
    }

    .ok {
      color: green;
      font-weight: bold;
    }

    .overflow {
      color: red;
      font-weight: bold;
    }

    table {
      border-collapse: collapse;
      width: 100%;
      margin-top: 12px;
    }

    th, td {
      border: 1px solid #999;
      padding: 6px;
      text-align: center;
    }

    th {
      background: #dddddd;
    }
  </style>
</head>

<body>
  <div class="card">
    <h2>Calculadora binária de 4 bits</h2>
    <p>Representação: complemento de dois. Faixa dos LEDs: -8 a +7.</p>

    <label>Operando A:</label>
    <input id="a" maxlength="4" value="0011">

    <label>Operando B:</label>
    <input id="b" maxlength="4" value="0010">

    <label>Operação:</label>
    <select id="op">
      <option value="add">SOMA: A + B</option>
      <option value="sub">SUB: A - B</option>
      <option value="mul">MULT: A x B</option>
      <option value="div">DIV: A / B</option>
      <option value="fact">FAT: A!</option>
    </select>

    <button onclick="calculate()">Calcular</button>
    <button onclick="runBenchmark()">Benchmark por tamanho de operando</button>

    <h3>Resultado</h3>
    <div id="output">Aguardando entrada...</div>

    <h3>Benchmark</h3>
    <div id="benchmarkOutput">Benchmark ainda não executado.</div>
  </div>

  <script>
    function isBin4(x) {
      return /^[01]{4}$/.test(x);
    }

    async function calculate() {
      const a = document.getElementById("a").value.trim();
      const b = document.getElementById("b").value.trim();
      const op = document.getElementById("op").value;

      if (!isBin4(a) || !isBin4(b)) {
        document.getElementById("output").innerHTML =
          "<p class='overflow'>Erro: use exatamente 4 bits em cada operando.</p>";
        return;
      }

      const params = new URLSearchParams();
      params.append("a", a);
      params.append("b", b);
      params.append("op", op);

      const response = await fetch("/calc?" + params.toString());
      const data = await response.json();

      if (data.error) {
        document.getElementById("output").innerHTML =
          "<p class='overflow'>Erro: " + data.error + "</p>";
        return;
      }

      const overflowText = data.overflow ? "SIM" : "NAO";
      const statusClass = data.overflow ? "overflow" : "ok";
      const statusText = data.overflow ? "OVERFLOW DETECTADO" : "OK";

      document.getElementById("output").innerHTML = `
        <pre>
A = ${data.aBin} (${data.aSigned})
${data.usesB ? `B = ${data.bBin} (${data.bSigned})` : "B = ignorado nesta operacao"}
Operacao = ${data.operation}

Resultado nos LEDs = ${data.resultBin}
Resultado interpretado = ${data.resultSigned}
Resultado exato = ${data.exactResult}
Resto = ${data.remainder}
Tempo de calculo = ${data.elapsedMicros} us

Overflow = ${overflowText}
        </pre>

        <p class="${statusClass}">${statusText}</p>
      `;
    }

    async function runBenchmark() {
      document.getElementById("benchmarkOutput").innerHTML =
        "<p>Executando benchmark no ESP32...</p>";

      const response = await fetch("/bench");
      const data = await response.json();

      let html = "";
      html += "<table>";
      html += "<tr>";
      html += "<th>Bits</th>";
      html += "<th>Iterações</th>";
      html += "<th>Soma média (us)</th>";
      html += "<th>Sub média (us)</th>";
      html += "<th>Mult média (us)</th>";
      html += "<th>Div média (us)</th>";
      html += "</tr>";

      for (const row of data.results) {
        html += "<tr>";
        html += "<td>" + row.bits + "</td>";
        html += "<td>" + row.iterations + "</td>";
        html += "<td>" + row.addAvgUs + "</td>";
        html += "<td>" + row.subAvgUs + "</td>";
        html += "<td>" + row.mulAvgUs + "</td>";
        html += "<td>" + row.divAvgUs + "</td>";
        html += "</tr>";
      }

      html += "</table>";
      html += "<p>Observação: os valores são medidos no ESP32 usando micros().</p>";

      document.getElementById("benchmarkOutput").innerHTML = html;
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleBench() {
  const uint32_t iterations = 10000;

  String json = "{";
  json += "\"results\":[";
  json += benchmarkJsonForBits(4, iterations);
  json += ",";
  json += benchmarkJsonForBits(8, iterations);
  json += ",";
  json += benchmarkJsonForBits(16, iterations);
  json += ",";
  json += benchmarkJsonForBits(32, iterations);
  json += "]";
  json += "}";

  server.send(200, "application/json", json);
}

void handleCalc() {
  if (!server.hasArg("a") || !server.hasArg("b") || !server.hasArg("op")) {
    server.send(400, "application/json", "{\"error\":\"Parametros ausentes\"}");
    return;
  }

  String aText = server.arg("a");
  String bText = server.arg("b");
  String op = server.arg("op");

  uint8_t a;
  uint8_t b;

  if (!parseBin4(aText, a) || !parseBin4(bText, b)) {
    server.send(400, "application/json", "{\"error\":\"Operandos invalidos\"}");
    return;
  }

  uint8_t result = 0;
  bool overflow = false;
  String operationText;
  int32_t exactResult = 0;
  int32_t remainder = 0;
  bool usesB = true;

  // Aritmetica implementada no ESP32, nao no Javascript.
  uint32_t startMicros = micros();

  if (op == "add") {
    exactResult = signed4(a) + signed4(b);
    result = exactResult & 0x0F;
    overflow = overflowAdd(a, b, result);
    operationText = "A + B";

  } else if (op == "sub") {
    exactResult = signed4(a) - signed4(b);
    result = exactResult & 0x0F;
    overflow = overflowSub(a, b, result);
    operationText = "A - B";

  } else if (op == "mul") {
    exactResult = multiply4(a, b);
    result = exactResult & 0x0F;
    overflow = !fitsSigned4(exactResult);
    operationText = "A x B";

  } else if (op == "div") {
    int16_t quotient16;
    int16_t remainder16;

    if (!divide4(a, b, quotient16, remainder16)) {
      server.send(400, "application/json", "{\"error\":\"Divisao por zero\"}");
      return;
    }

    exactResult = quotient16;
    remainder = remainder16;
    result = exactResult & 0x0F;
    overflow = !fitsSigned4(exactResult);
    operationText = "A / B";

  } else if (op == "fact") {
    uint16_t factorialResult;

    if (!factorial4(a, factorialResult)) {
      server.send(400, "application/json", "{\"error\":\"Fatorial aceita apenas A entre 0 e 7\"}");
      return;
    }

    usesB = false;
    exactResult = factorialResult;
    result = exactResult & 0x0F;
    overflow = !fitsSigned4(exactResult);
    operationText = "A!";

  } else {
    server.send(400, "application/json", "{\"error\":\"Operacao invalida\"}");
    return;
  }

  uint32_t elapsedMicros = micros() - startMicros;

  writeLeds(result);

  Serial.println();
  Serial.println("Nova operacao:");
  Serial.print("A = ");
  Serial.print(bin4(a));
  Serial.print(" (");
  Serial.print(signed4(a));
  Serial.println(")");

  if (usesB) {
    Serial.print("B = ");
    Serial.print(bin4(b));
    Serial.print(" (");
    Serial.print(signed4(b));
    Serial.println(")");
  } else {
    Serial.println("B = ignorado nesta operacao");
  }

  Serial.print("Operacao: ");
  Serial.println(operationText);

  Serial.print("Resultado LEDs = ");
  Serial.print(bin4(result));
  Serial.print(" (");
  Serial.print(signed4(result));
  Serial.println(")");

  Serial.print("Resultado exato: ");
  Serial.println(exactResult);

  Serial.print("Resto: ");
  Serial.println(remainder);

  Serial.print("Overflow: ");
  Serial.println(overflow ? "SIM" : "NAO");

  Serial.print("Tempo de calculo: ");
  Serial.print(elapsedMicros);
  Serial.println(" us");

  String json = "{";
  json += "\"aBin\":\"" + bin4(a) + "\",";
  json += "\"bBin\":\"" + bin4(b) + "\",";
  json += "\"resultBin\":\"" + bin4(result) + "\",";
  json += "\"aSigned\":" + String(signed4(a)) + ",";
  json += "\"bSigned\":" + String(signed4(b)) + ",";
  json += "\"resultSigned\":" + String(signed4(result)) + ",";
  json += "\"exactResult\":" + String(exactResult) + ",";
  json += "\"remainder\":" + String(remainder) + ",";
  json += "\"elapsedMicros\":" + String(elapsedMicros) + ",";
  json += "\"operation\":\"" + operationText + "\",";
  json += "\"usesB\":" + String(usesB ? "true" : "false") + ",";
  json += "\"overflow\":" + String(overflow ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Calculadora 4 bits iniciada.");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/calc", handleCalc);
  server.on("/bench", handleBench);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}