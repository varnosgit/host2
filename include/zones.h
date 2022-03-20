#include <Arduino.h>


typedef struct ventDev { //16 character
  uint8_t batteryState;
  uint8_t doorState;
  uint8_t devID;
  uint8_t MAC_addr[6];
  char name[16]= "Vent ";
  bool isActive = 0;
} ventDev;

typedef struct termoDev { //16 character
  uint8_t batteryState;
  uint8_t setPoint;
  uint8_t temperature;
  uint8_t devID;
  uint8_t MAC_addr[6];
  char name[16]= "Termostat ";
  bool isActive = 0;
} termoDev;

typedef struct systemZone { //16 character
  char name[16]= "no Name";
  char icon[16]= "no icon";
  bool isActive = 0;
  termoDev termos[10];
  ventDev vents[10];
} systemZone;

void zone_reload(void);
String get_zone_names(void);
uint8_t add_new_zone(String name, String icon);
uint8_t delete_zone(String name);
uint8_t get_zoneID_by_name(String zone_name);

String add_device_to_zone(uint8_t zoneID, uint8_t devModel, uint8_t *devMAC);
String get_zoen_device_names(uint8_t zoneID);

uint8_t get_DeviceID_by_name(int zoneID, String dev_name);