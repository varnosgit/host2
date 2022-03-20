#include "zones.h"
#include "host_control.h"
#include "FS.h"
#include "SPIFFS.h"

extern hc_message hc_mesg;
extern uint8_t hc_sendFlag, hc_recvFlag;

uint8_t pair_request_flag=0, status_request_flag=0;
uint8_t globalZoneID, globalDeviceID;
const uint8_t numOfMaxZones = 20;
struct systemZone zones[numOfMaxZones];
struct systemZone* zonesAddr = &zones[0];

//////////////////////////////////// privet functions
char* readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("Failed to open file for reading");
        return "FAIL";
    }
    file.read((uint8_t *)&zones[0], sizeof(zones));
    delayMicroseconds(100);
    file.close(); 
}
////////////
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.write((uint8_t *)message, sizeof(zones));
    file.close();
}
//////////////////////////////////////////////////////

void zone_reload(void)
{   
    ///////////////////////////////////////////////////// flash zone file read
    //writeFile(SPIFFS, "/zones.txt", (char *)zonesAddr);
    readFile(SPIFFS, "/zones.txt");
    if (strcmp(zones[0].name, "Main Room") != 0) //no main room available
    {
        zones[0].isActive = true;    
        strcpy(zones[0].name, "Main Room");
        strcpy(zones[0].icon, "icon MRoom");
        writeFile(SPIFFS, "/zones.txt", (char *)zonesAddr);
    }
}

String get_zone_names(void)
{
    String __zone_name_commands = "Zone Names";
    for(int i=0; i<numOfMaxZones; i++)
    {
        if (zones[i].isActive)
            __zone_name_commands += String(",") + String(zones[i].name);
    } 
   // __zone_name_commands += String(zones[0].name) +  String(",") + String(zones[1].name);
    return __zone_name_commands;
}

uint8_t add_new_zone(String name, String icon)
{   
    char temp[16];
    for (int i=0;i<numOfMaxZones; i++)
        {
            if (zones[i].isActive == false) //available empty zone
            {
                zones[i].isActive = true;
                name.toCharArray(temp, 16);
                strcpy(zones[i].name, temp);
                icon.toCharArray(temp, 16);
                strcpy(zones[i].icon, temp);
                writeFile(SPIFFS, "/zones.txt", (char *)zonesAddr);
                return i;
            }
        }
    return 0;    
}

uint8_t delete_zone(String name)
{
    char temp[16];
    name.toCharArray(temp, 16);
    for (int i=0;i<numOfMaxZones; i++)
        {
            if (strcmp(zones[i].name, temp) == 0)  //equal names
            {   
                Serial.println("A zone has been deleted succesfully!");
                zones[i].isActive = false;
                for (int j=0; j<10;j++)
                {
                    if (zones[i].termos[j].isActive == 1)
                    {
                        zones[i].termos[j].isActive = 0;
                        // send unpair command ????
                        // hc_mesg.__hcdata = 5; // 5 = host: unpair this device with reciever MAC Add if any
                        // for (int k=0; k<6; k++)
                        //     hc_mesg.reciever_MAC_addr[k] = zones[i].termos[j].MAC_addr[k];
                        // hc_sendFlag = 1;
                        // while (hc_sendFlag);                        
                    }
                    if (zones[i].vents[j].isActive == 1)
                    {
                        zones[i].vents[j].isActive = 0;
                        // send unpair command ????
                    }
                }
                writeFile(SPIFFS, "/zones.txt", (char *)zonesAddr);
                return i;
            }
        }
    return 0;    
}

uint8_t get_zoneID_by_name(String zone_name)
{
    char temp[16];
    zone_name.toCharArray(temp, 16);
    for (int i=0; i<numOfMaxZones; i++)
    {
        if (String(zones[i].name) == zone_name) //same name
        {
            Serial.print("zone id from name: "); Serial.println(i);
            return i;
        }
    }
    Serial.print("zone id from name: "); Serial.println(255);
    return 255;
}
uint8_t get_DeviceID_by_name(int zoneID, String dev_name)
{
    for (int i=0; i<10; i++)
    {
        if (String(zones[zoneID].termos[i].name) == dev_name) //same name
        {
            Serial.print("Device termostat id from name in zone: "); Serial.println(i);
            return i;
        }
        if (String(zones[zoneID].vents[i].name) == dev_name) //same name
        {
            Serial.print("Device vent id from name in zone: "); Serial.println(i);
            return i;
        }
    }
    Serial.print("no device found"); Serial.println(255);
    return 255;
}

String add_device_to_zone(uint8_t zoneID, uint8_t devModel, uint8_t *devMAC)
{
    String dev_name = "no name";
    for (int i=0;i<10; i++)
    {
        if (devModel == 2) //termostat
        {
            if (zones[zoneID].termos[i].isActive == 0) //empty position
            {
                zones[zoneID].termos[i].isActive = 1;
                zones[zoneID].termos[i].name[10] = i + '0';
                dev_name = String(zones[zoneID].termos[i].name);
                for (int j=0; j<6; j++)
                    zones[zoneID].termos[i].MAC_addr[j] = devMAC[j];        
                break;
            }
        }
        else if (devModel == 3) //vent
        {
            if (zones[zoneID].vents[i].isActive == 0) //empty position
            {
                zones[zoneID].vents[i].isActive = 1;
                zones[zoneID].vents[i].name[5] = i + '0';
                dev_name = String(zones[zoneID].termos[i].name);
                for (int j=0; j<6; j++)
                    zones[zoneID].vents[i].MAC_addr[j] = devMAC[j];   
                break;    
            }
        }
    }
    if (dev_name != "no name")
    {
        writeFile(SPIFFS, "/zones.txt", (char *)zonesAddr);
        Serial.println("A dev added to a zone succesfully!");
    }
    else
    {
        Serial.println("Can not add new device!");
    }
    return dev_name;
}

String get_zoen_device_names(uint8_t zoneID)
{
    String names = "Device Names";
    for (int i=0;i<10; i++)
    {
        if (zones[zoneID].termos[i].isActive) {
            names+="," + String(zones[zoneID].termos[i].name);
        }
    }
    for (int i=0;i<10; i++)
    {
        if (zones[zoneID].vents[i].isActive) names+="," + String(zones[zoneID].vents[i].name);
    }
    return names;
}