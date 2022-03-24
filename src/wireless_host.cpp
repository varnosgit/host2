#include "Arduino.h"
#include "wireless_host.h"
#include "host_control.h"
#include "zones.h"

//const char* ssid = "HUAWEI-2.4G-g4ci"; const char* password = "db7nf4dk";
//const char* ssid = "artin123"; const char* password = "Smartartin123";
// const char* ssid = "Varnos5"; const char* password = "toolesag";
const char* ssid = "Hanisa"; const char* password = "1qaz!QAZ";
String hostname = "HVAC Server";

IPAddress localIp;
extern uint8_t pair_request_flag;
extern systemZone zones[];
extern uint8_t modeStat, fanStat;
extern bool ledState;
extern const int ledPin;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void wifi_initial(void)
{
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int con_counter = 0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(2000);
    con_counter++;
    Serial.println("Connecting to WiFi..");
    if (con_counter > 4) ESP.restart();
  }
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  if(!MDNS.begin("hvac")) 
  {
     Serial.println("Error starting mDNS");
  }

  Serial.println(WiFi.localIP()); //display_log_print("IP: " + WiFi.localIP().toString());
  localIp = WiFi.localIP();
}

void setup_webpages(void)
{
    //  Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
        // Route for root / web page
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
      // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
    // Route to load script.js file
  server.on("/sct_home.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sct_home.js", "text/javascript");
  });

      // Route for root / web page
  server.on("/addzone.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/addzone.html", String(), false, processor);
  });
        // Route for root / web page
  server.on("/myzone.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/myzone.html", String(), false, processor);
  });
            // Route for root / web page
  server.on("/device.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/device.html", String(), false, processor);
  });
          // Route for root / web page
  server.on("/vent.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/vent.html", String(), false, processor);
  });
          // Route for root / web page
  server.on("/termo.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/termo.html", String(), false, processor);
  });
  server.begin();
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.print("processor: "); Serial.println(var);
  return String();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) 
{
  //String zoneNames;
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      ws.text(client->id(), get_zone_names());
      break;
    case WS_EVT_DISCONNECT:
      pair_request_flag = 0;
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len, client->id());
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint32_t client_id) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    handle_browser_message((char *)data, len, client_id);

    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      Serial.println((char*)data);
      //tft.fillRect(0, 0, 150, 50, ledState ? TFT_GREEN : TFT_BLACK);
      //notifyClients(0);
    }

    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      Serial.println((char*)data);
      //tft.fillRect(0, 0, 150, 50, ledState ? TFT_GREEN : TFT_BLACK);
      //notifyClients(0);
    }
    if (strcmp((char*)data, "newp") == 0) {
      ledState = !ledState;
      //tft.fillRect(0, 0, 150, 50, ledState ? TFT_RED : TFT_BLACK);
      notifyClients(1);
    }
    //else
      // Serial.println((char*)data);
      //ws.textAll((char*)data);
     // ws.textAll("salam, khodafez");
  }
}

void notifyClients(int dd) {
  if (dd)
  ws.textAll(String(ledState));
  else
  ws.textAll("2");
  Serial.println("ssssssaaaaaaaaaagg");
}

void notifyClients_txt(String txt) {
  ws.textAll(txt);
}

void notify_a_client_txt(String txt, uint32_t myID) {
  ws.text(myID, txt);
}

