#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_CALCULADORA";
const char* password = "12345678";

WebServer server(80);

void handleRoot() {
  String html = "";
  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head><meta charset='UTF-8'><title>ESP32</title></head>";
  html += "<body>";
  html += "<h1>ESP32 WebServer funcionando</h1>";
  html += "<p>Modo Access Point ativo.</p>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("Access Point iniciado.");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}