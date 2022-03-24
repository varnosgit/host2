#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

void wifi_initial();
void initWebSocket();
void setup_webpages(void);


String processor(const String& var);
void notifyClients(int dd);
void notifyClients_txt(String txt);
void notify_a_client_txt(String txt, uint32_t myID);

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint32_t client_id);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
