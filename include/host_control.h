#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <EEPROM.h>

typedef struct hc_message {
  uint8_t begin_validator[3];
  uint8_t sender_MAC_addr[6];
  uint8_t reciever_MAC_addr[6];
  uint8_t _sender;   //0 unknown, 1 controller, 2 termostat, 3 vent
  uint8_t _command;  //0 unknown, 1 register, 2 execute, 3 readState
  uint8_t temprature;
  uint8_t batStat;
  uint8_t fanStatus;
  uint8_t setPoint_temp;
  uint8_t ventStatus;
  uint32_t zone_id_group;
  uint8_t dev_number;
  uint8_t reserved;
  uint8_t __hcdata; // 0 = do nothing
                    // 1 = cont.: introduce controller to host (reciver macAdd = controller MAC Add)
                    // 2 = host.: introduce host to controller (reciver macAdd = host IP)
                    // 3 = cont.: this is a message from a device (sender mac address)
                        // host : send this message directly to device (reciver mac addr) 
                    // 4 = host: pair a device with sender MAC Add
                    // 5 = host: unpair this device with reciever MAC Add if any
                    // 6 = cont.: delivery to device failed
                    // 7 = host: brodcast this message to all devices
  uint8_t end_validator;
} hc_message;

void print_hcMessage(hc_message DD);
void send_data_to_controller(void);
bool receive_data_from_controller(void);
void handle_controller_message(void);
void handle_browser_message( char *data, size_t len, uint32_t client_id);

                        /////////////////////////////////////// notify browser
                        // char buf[100]; 
                        // uint8_t* tempo = hc_mesg.sender_MAC_addr;
                        // String devModelWeb = "./vent.html"; 
                        // if (hc_mesg._sender == 2) devModelWeb = "./termo.html";
                        // // snprintf(buf, 100, "Add Device, %02X:%02X:%02X:%02X:%02X:%02X,./termo.html,ventName",
                        // //             tempo[0], tempo[1], tempo[2], tempo[3], tempo[4], tempo[5]); 
                        // snprintf(buf, 100, "Add Device, %02X:%02X:%02X:%02X:%02X:%02X,./termo.html,ventName",
                        //             tempo[0], tempo[1], tempo[2], tempo[3], tempo[4], tempo[5]); 
                        // String newDev = String(buf);
                        // Serial.println("");
                        // Serial.println(newDev);