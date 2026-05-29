#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_CALCULADORA_UM";
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

int8_t signedOnes4(uint8_t value) {
  value &= 0x0F;

  if ((value & 0x08) == 0) {
    return value;
  }

  return -((~value) & 0x0F);
}

String signedOnes4Text(uint8_t value) {
  value &= 0x0F;

  if (value == 0x00) {
    return "+0";
  }

  if (value == 0x0F) {
    return "-0";
  }

  return String(signedOnes4(value));
}

uint8_t onesComplement(uint8_t value) {
  return (~value) & 0x0F;
}

uint8_t addOnes4(uint8_t a, uint8_t b) {
  uint8_t sum = (a & 0x0F) + (b & 0x0F);
  uint8_t result = sum & 0x0F;

  if (sum & 0x10) {
    result = (result + 1) & 0x0F;
  }

  return result;
}

bool overflowAddOnes4(uint8_t a, uint8_t b, uint8_t result) {
  bool signA = a & 0x08;
  bool signB = b & 0x08;
  bool signR = result & 0x08;

  return (signA == signB) && (signR != signA);
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
      max-width: 560px;
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
    }

    .ok {
      color: green;
      font-weight: bold;
    }

    .overflow {
      color: red;
      font-weight: bold;
    }
  </style>
</head>

<body>
  <div class="card">
    <h2>Calculadora binaria de 4 bits</h2>
    <p>Representacao: complemento de um. Faixa: -7 a +7.</p>
    <p>Observacao: 0000 representa +0 e 1111 representa -0.</p>

    <label>Operando A:</label>
    <input id="a" maxlength="4" value="0011">

    <label>Operando B:</label>
    <input id="b" maxlength="4" value="0010">

    <label>Operacao:</label>
    <select id="op">
      <option value="add">SOMA: A + B</option>
      <option value="sub">SUB: A - B</option>
    </select>

    <button onclick="calculate()">Calcular</button>

    <h3>Resultado</h3>
    <div id="output">Aguardando entrada...</div>
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
A = ${data.aBin} (${data.aSignedText})
B = ${data.bBin} (${data.bSignedText})
Operacao = ${data.operation}

Resultado nos LEDs = ${data.resultBin}
Resultado interpretado = ${data.resultSignedText}

Overflow = ${overflowText}
        </pre>

        <p class="${statusClass}">${statusText}</p>
      `;
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", PAGE);
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

  uint8_t result;
  bool overflow;
  String operationText;

  // Aritmetica em complemento de um com carry de retorno.
  if (op == "add") {
    result = addOnes4(a, b);
    overflow = overflowAddOnes4(a, b, result);
    operationText = "A + B";
  } else if (op == "sub") {
    uint8_t negB = onesComplement(b);
    result = addOnes4(a, negB);
    overflow = overflowAddOnes4(a, negB, result);
    operationText = "A - B";
  } else {
    server.send(400, "application/json", "{\"error\":\"Operacao invalida\"}");
    return;
  }

  writeLeds(result);

  Serial.println();
  Serial.println("Nova operacao:");
  Serial.print("A = ");
  Serial.print(bin4(a));
  Serial.print(" (");
  Serial.print(signedOnes4Text(a));
  Serial.println(")");

  Serial.print("B = ");
  Serial.print(bin4(b));
  Serial.print(" (");
  Serial.print(signedOnes4Text(b));
  Serial.println(")");

  Serial.print("Operacao: ");
  Serial.println(operationText);

  Serial.print("Resultado = ");
  Serial.print(bin4(result));
  Serial.print(" (");
  Serial.print(signedOnes4Text(result));
  Serial.println(")");

  Serial.print("Overflow: ");
  Serial.println(overflow ? "SIM" : "NAO");

  String json = "{";
  json += "\"aBin\":\"" + bin4(a) + "\",";
  json += "\"bBin\":\"" + bin4(b) + "\",";
  json += "\"resultBin\":\"" + bin4(result) + "\",";
  json += "\"aSigned\":" + String(signedOnes4(a)) + ",";
  json += "\"bSigned\":" + String(signedOnes4(b)) + ",";
  json += "\"resultSigned\":" + String(signedOnes4(result)) + ",";
  json += "\"aSignedText\":\"" + signedOnes4Text(a) + "\",";
  json += "\"bSignedText\":\"" + signedOnes4Text(b) + "\",";
  json += "\"resultSignedText\":\"" + signedOnes4Text(result) + "\",";
  json += "\"operation\":\"" + operationText + "\",";
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
  Serial.println("Calculadora 4 bits em complemento de um iniciada.");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/calc", handleCalc);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}
