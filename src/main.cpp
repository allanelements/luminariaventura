
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define PWM_OUT_PIN 32
#define PWM_OUT_PIN_2 33

int currentHour;
int currentMinute;
int currentSecond;

int pwmValue = 0;
int pwmValue2 = 0;
bool onOffState = 1;
bool modoAutomaticoState = 1;
int selectedHour = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 60 * 60);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void connectToWiFi() {
  WiFi.begin("Elements", "Elements@@2024!");
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

void actionLed() {
  timeClient.update();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
  currentSecond = timeClient.getSeconds();
  ledcWrite(0, pwmValue);
  ledcWrite(1, pwmValue2);
}

void handleWebSocketMessage(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len) {
  DynamicJsonDocument jsonDoc(256);
  deserializeJson(jsonDoc, data, len);

  Serial.println("Mensagem WebSocket recebida:");

  if (jsonDoc.containsKey("onOffState")) {
    onOffState = jsonDoc["onOffState"];
    Serial.print("onOffState atualizado para: ");
    Serial.println(onOffState);
  }

  if (jsonDoc.containsKey("modoAutomaticoState")) {
    modoAutomaticoState = jsonDoc["modoAutomaticoState"];
    Serial.print("modoAutomaticoState atualizado para: ");
    Serial.println(modoAutomaticoState);
  }

  if (jsonDoc.containsKey("selectedHour")) {
    selectedHour = jsonDoc["selectedHour"];
    Serial.print("selectedHour atualizado para: ");
    Serial.println(selectedHour);
  }
}


void handleRoot(AsyncWebServerRequest *request) {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
    html += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.2.0/css/all.min.css'>";
    html += "<style>body {font-family: Arial, sans-serif; background-color: #ffffff; margin: 0; padding: 0; position: relative;}";
    html += ".container {width: 90%; margin: auto; text-align: center;}";
    html += ".header {padding: 5px 0; margin-bottom: 5px; text-align: center;}";
    html += ".logo {width: 50px; height: 50px; cursor: pointer; margin-bottom: 25px; }";
    html += "img {width: 100%; height: auto; max-width: 80%; margin: 20px auto; display: block;}";
    html += ".switch-container {display: flex; flex-direction: column; align-items: center;}";
    html += ".switch-label {margin-bottom: 10px; font-size: 16px;}";
    html += ".switch {position: relative; display: inline-block; width: 60px; height: 34px;}";
    html += ".switch input {display: none;}";
    html += ".slider {position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}";
    html += ".slider:before {position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}";
    html += "input:checked + .slider {background-color: #2196F3;}";
    html += "input:focus + .slider {box-shadow: 0 0 1px #2196F3;}";
    html += "input:checked + .slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}";
    html += ".slider.round {border-radius: 34px;}";
    html += ".slider.round:before {border-radius: 50%;}";
    html += ".button-container {margin-top: 20px; display: flex; flex-wrap: wrap; justify-content: center;}";
    html += ".button-container h2 {width: 100%; text-align: center; margin-bottom: 10px; font-size: 18px;}";
    html += ".button-container button {margin: 5px; padding: 10px; font-size: 16px; flex: 0 0 calc(33.333% - 10px); max-width: calc(33.333% - 10px);}";
    html += ".responsive-image {width: 100%; height: auto; max-width: 80%; margin: 20px auto; display: block;}";
    html += ".NT-footer-icones {background-color: #000; color: #fff; padding: 20px 0; text-align: center;}";
    html += "</style></head><body>";
    html += "<div class='header'><img class='logo' src='https://elements.com.br/cdn/shop/files/Group_1.png?v=1702562143&width=50' onclick='window.location.href=\"https://elements.com.br/\"' alt='Logo'></div>";
    html += "<div class='container'>";
    html += "<h2 style='font-size: 24px;'>Luminária Solare</h2>";
    html += "<div class='switch-container'>";

    html += "<label class='switch-label' style='font-size: 16px;'>ON/OFF</label>";
    html += "<label class='switch'><input id='onOffSwitch' type='checkbox' onchange='toggleOnOff(this)'><span class='slider round'></span></label>";
    html += "<label class='switch-label' style='font-size: 16px;'>Modo Automático</label>";
    html += "<label class='switch'><input id='modoAutomaticoSwitch' type='checkbox' onchange='toggleModoAutomatico(this)'><span class='slider round'></span></label>";

    
    html += "</div>";
    html += "<div class='button-container'>";
    html += "<h2>Escolha a luz do horário que deseja:</h2>";
    html += "<button onclick='setLight(8)'>8:00hrs</button>";
    html += "<button onclick='setLight(10)'>10:00hrs</button>";
    html += "<button onclick='setLight(12)'>12:00hrs</button>";
    html += "<button onclick='setLight(14)'>14:00hrs</button>";
    html += "<button onclick='setLight(16)'>16:00hrs</button>";
    html += "<button onclick='setLight(18)'>18:00hrs</button>";
    html += "</div>";
    html += "<img class='responsive-image' src='https://dcuatro.com/wp-content/uploads/2020/08/Articulo_alejandro_imagen.jpg' alt='Imagem'>";
    html += "</div>";
    html += "<div class='NT-footer-icones'>";
    // Restante do código para os ícones
    html += "</div>";
    
    
    
   html += "<script>";
html += "var ws = new WebSocket('ws://' + window.location.hostname + '/ws');";
html += "ws.onopen = function(event) {";
html += "  console.log('Conexão WebSocket estabelecida');";
html += "};";
html += "function setLight(hour) {";
html += "  console.log('Botão pressionado para o horário:', hour);";
html += "  var message = { selectedHour: hour };";
html += "  ws.send(JSON.stringify(message));";
html += "}";
html += "function toggleOnOff(checkbox) {";
html += "  var onOffValue = checkbox.checked ? 1 : 0;";
html += "  var message = { onOffState: onOffValue };";
html += "  ws.send(JSON.stringify(message));";
html += "}";
html += "function toggleModoAutomatico(checkbox) {";
html += "  var modoAutomaticoValue = checkbox.checked ? 1 : 0;";
html += "  var message = { modoAutomaticoState: modoAutomaticoValue };";
html += "  ws.send(JSON.stringify(message));";
html += "}";

// Script para atualizar os switches com base nas variáveis globais
html += "document.addEventListener('DOMContentLoaded', function() {";
html += "  var onOffSwitch = document.getElementById('onOffSwitch');";
html += "  var modoAutomaticoSwitch = document.getElementById('modoAutomaticoSwitch');";
html += "  onOffSwitch.checked = " + String(onOffState ? "true" : "false") + ";";
html += "  modoAutomaticoSwitch.checked = " + String(modoAutomaticoState ? "true" : "false") + ";";
html += "});";

html += "</script>";

html += "</body></html>";
request->send(200, "text/html", html);

}


void setup() {
  WiFi.setHostname("luminaria-elements");
  Serial.begin(115200);
  pinMode(PWM_OUT_PIN, OUTPUT);
  pinMode(PWM_OUT_PIN_2, OUTPUT);
  ledcAttachPin(PWM_OUT_PIN, 0);
  ledcAttachPin(PWM_OUT_PIN_2, 1);
  ledcSetup(0, 12000, 10);
  ledcSetup(1, 12000, 10);
  
  connectToWiFi();

  // Configuração do WebSocket
  ws.onEvent(handleWebSocketMessage);
  server.addHandler(&ws);

  // Inicia o servidor
  server.begin();

  // Inicia o cliente NTP
  timeClient.begin();
  timeClient.setTimeOffset(-10800); // Horário de Brasília (UTC -3)

  // Adiciona o handler para /globals.js
  server.on("/globals.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Envie as variáveis globais JavaScript para o cliente
    String response = "var onOffState = " + String(onOffState) + ";\n";
    response += "var modoAutomaticoState = " + String(modoAutomaticoState) + ";\n";
    response += "var selectedHour = " + String(selectedHour) + ";";
    request->send(200, "text/javascript", response);
  });

  // Handler para a raiz da página
  server.on("/", HTTP_GET, handleRoot);
}


void loop() {

   // Verifica se as variáveis foram alteradas
  static bool prevOnOffState = false;
  static bool prevModoAutomaticoState = false;

  if (onOffState != prevOnOffState || modoAutomaticoState != prevModoAutomaticoState) {
    // Atualiza as variáveis de estado anterior
    prevOnOffState = onOffState;
    prevModoAutomaticoState = modoAutomaticoState;

    // Cria e envia a mensagem WebSocket para informar o cliente sobre a mudança
    DynamicJsonDocument jsonDoc(128);
    jsonDoc["onOffState"] = onOffState;
    jsonDoc["modoAutomaticoState"] = modoAutomaticoState;
    String jsonData;
    serializeJson(jsonDoc, jsonData);
    ws.textAll(jsonData);
  }
  
  if (onOffState) {
    pwmValue = 255;
  } else {
    pwmValue = 0;
    pwmValue2 = 0;
  }

  if (modoAutomaticoState) {
    pwmValue2 = 255;
  } else {
    pwmValue = 0;
    pwmValue2 = 0;
  }
  actionLed();
   //Serial.print("ON/OFF State: " + String(onOffState));
  //Serial.print("  -  Modo Automático State: " + String(modoAutomaticoState));
  //Serial.println(" -  Selected Hour: " + String(selectedHour));
}
