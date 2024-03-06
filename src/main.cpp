#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 60 * 60);  // Ajuste para o fuso horário de Brasília (GMT-3)

AsyncWebServer server(80);

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

void setup() {
  WiFi.setHostname("luminaria-elements");
  Serial.begin(115200);

  WiFi.disconnect(true);
  connectToWiFi();

  // Inicialize o cliente NTP
  timeClient.begin();

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Código HTML com os botões e a imagem abaixo
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
    html += "<label class='switch'><input type='checkbox'><span class='slider round'></span></label>";
    html += "<label class='switch-label' style='font-size: 16px;'>Modo Automático</label>";
    html += "<label class='switch'><input type='checkbox'><span class='slider round'></span></label>";
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
    html += "function setLight(hour) {";
    html += "console.log('Botão pressionado para o horário:', hour);";
    html += "}";
    html += "</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
});



  // Adicione o endpoint para obter o estado atual
  server.on("/getState", HTTP_GET, [](AsyncWebServerRequest *request) {
    String state = "{\"time\":\"" + timeClient.getFormattedTime() + "\"}";
    request->send(200, "application/json", state);
  });

  server.begin();
}

void loop() {
  // Atualize o cliente NTP
  timeClient.update();

  // Adicione a lógica do ciclo circadiano aqui
  // Adicione outras lógicas necessárias para o ciclo circadiano
}
