#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncWebSocket.h>

#define PWM_OUT_PIN 32
#define PWM_OUT_PIN_2 33


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 30000);

int temperatura = 0;
int tempAuto = 0;
int atualTemp;
bool controlAuto = false;

const char *mainSSID = "Elements";
const char *mainPassword = "Elements@@2024!";

void notifyClients() {
  ws.textAll(String(tempAuto));
}

void updateTemperatureBox() {
  notifyClients();
}

void handleIncreaseTemperature(AsyncWebServerRequest *request) {
  if (temperatura < 100) {
    temperatura += 10;
    updateTemperatureBox();
  }
  Serial.println("Temperatura atual: " + String(temperatura));
  request->send(200, "text/plain", "");
}

void handleDecreaseTemperature(AsyncWebServerRequest *request) {
  if (temperatura > 0) {
    temperatura -= 10;
    updateTemperatureBox();
  }
  Serial.println("Temperatura atual: " + String(temperatura));
  request->send(200, "text/plain", "");
}



void handleAutoTemperature() {
  if (tempAuto == 0) {
    tempAuto = 100;
  } else {
    tempAuto = 0;
  }
  Serial.println("TempAuto atual: " + String(tempAuto));
  updateTemperatureBox();
}

void handleCicloCircadiano(AsyncWebServerRequest *request) {
  controlAuto = !controlAuto;
  Serial.println("Ciclo Circadiano ativado: " + String(controlAuto));
  
  String response = (controlAuto) ? "true" : "false";
  request->send(200, "text/plain", response);
}

void connectToWiFi() {
  WiFi.begin(mainSSID, mainPassword);
  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED && attempt < 10) {
    delay(500);
    Serial.println("Tentando conectar ao WiFi principal...");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao WiFi principal!");
    Serial.println("IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("Falha ao conectar ao WiFi principal.");
  }
}

void setup() {
  WiFi.setHostname("luminaria-elements");
  ledcSetup(0, 5000, 10);
  ledcSetup(1, 5000, 10);
  ledcAttachPin(PWM_OUT_PIN, 0);
  ledcAttachPin(PWM_OUT_PIN_2, 1);



  Serial.begin(115200);

  // Desconectar de qualquer rede anterior
  WiFi.disconnect(true);

  connectToWiFi();

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    // WebSocket message handling (if needed)
  });

  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
    html += "<style>body {font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0;}";
    html += ".container {width: 80%; margin: auto; text-align: center;}";
    html += "h2 {text-align: center; color: #333;}";
    html += "button {font-size: 20px; margin: 10px; padding: 10px 20px; width: 150px; cursor: pointer; border: none; border-radius: 5px;}";
    html += "#quenteBtn {background-color: #FFD700; color: #333;}";
    html += "#frioBtn {background-color: #ADD8E6; color: #333;}";
    html += "#cicloBtn {font-size: 16px; margin: 10px; padding: 10px 20px; width: 150px; cursor: pointer; border: none; border-radius: 5px; background-color: ";
    html += (controlAuto) ? "#800080;" : "#808080; color: #fff;}";
    html += ".quente-info, .frio-info, .temp-auto-info {margin: 10px; padding: 10px; width: 150px; display: inline-block;}";
    html += ".quente-info {background-color: #FFD700;}";
    html += ".frio-info {background-color: #ADD8E6;}";
    html += ".temp-auto-info {background-color: #8A2BE2;}";
    html += "</style></head><body><div class='container'>";
    html += "<h2>Lumin&aacute;ria Ventura</h2>";
    html += "<button onclick='decreaseTemperature()' id='quenteBtn' " + String((controlAuto) ? "disabled" : "") + ">Quente</button>";
    html += "<button onclick='increaseTemperature()' id='frioBtn' " + String((controlAuto) ? "disabled" : "") + ">Frio</button>";

    //html += "<div class='quente-info' id='quenteInfo'>Quente (" + String(100 - temperatura) + "%)</div>";
    //html += "<div class='frio-info' id='frioInfo'>Frio (" + String(temperatura) + "%)</div>";
    html += "<button onclick='toggleCiclo()' id='cicloBtn'>Ciclo Circadiano</button>";
    html += "<script>";
    html += "function decreaseTemperature() { fetch('/decreaseTemperature'); }";
    html += "function increaseTemperature() { fetch('/increaseTemperature'); }";
    html += "function toggleCiclo() { fetch('/toggleCiclo').then(() => location.reload()); }";
    html += "</script>";
    html += "</div></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/decreaseTemperature", HTTP_GET, handleDecreaseTemperature);
  server.on("/increaseTemperature", HTTP_GET, handleIncreaseTemperature);
  server.on("/toggleCiclo", HTTP_GET, handleCicloCircadiano);

  server.begin();
}

void loop() {
  atualTemp = (controlAuto) ? tempAuto : temperatura;
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  tempAuto = map(currentSecond,0,60,0,100);
  if(controlAuto == false) atualTemp = temperatura;
  else atualTemp = tempAuto;

  int pwmValue = map(atualTemp, 0, 100, 0, 255);
  int pwmValue2 = map(atualTemp, 100, 0, 0, 255);

  ledcWrite(0, pwmValue);
  ledcWrite(1, pwmValue2);
  notifyClients();

}

