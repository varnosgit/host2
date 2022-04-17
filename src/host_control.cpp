#include "host_control.h"
#include "zones.h"
#include "wireless_host.h"

extern uint8_t pair_request_flag;
extern uint8_t globalZoneID, globalDeviceID;
extern const uint8_t numOfMaxZones;
extern systemZone zones[];;
extern uint8_t FanState;  // 0 = auto, 1 = On
extern uint8_t ModeState;  // 0 = off, 1 = Heat, 2 = Cool
 
hc_message hc_mesg;
extern uint8_t hc_sendFlag, hc_recvFlag;
extern IPAddress localIp;

uint8_t controller_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  

// this 3 below functions must run on the second core
void send_data_to_controller(void)
{
    Serial.println("Sending data to controller");
    hc_mesg.begin_validator[0] = 'V';
    hc_mesg.begin_validator[1] = 'A';
    hc_mesg.begin_validator[2] = 'C';
    hc_mesg.end_validator = 'H';
    Serial2.write((const uint8_t *)&hc_mesg, sizeof(hc_mesg));  
    hc_sendFlag = 0;
}

bool receive_data_from_controller(void)
{
    if (Serial2.available() > 10)
    //if (Serial2.available()) 
    {
      delay(10);  
      char* rxdata = (char*) &hc_mesg;
      Serial2.read(rxdata, sizeof(hc_mesg));
      return true;
    }
    return false;
}

void print_hcMessage(hc_message DD)
{
    Serial.printf("   reciever MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    DD.reciever_MAC_addr[0], DD.reciever_MAC_addr[1], DD.reciever_MAC_addr[2], 
    DD.reciever_MAC_addr[3], DD.reciever_MAC_addr[4], DD.reciever_MAC_addr[5]); 
    Serial.printf("   sender MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    DD.sender_MAC_addr[0], DD.sender_MAC_addr[1], DD.sender_MAC_addr[2], 
    DD.sender_MAC_addr[3], DD.sender_MAC_addr[4], DD.sender_MAC_addr[5]);

    Serial.print("    * __hcdata: ");      Serial.println(DD.__hcdata);
    Serial.print("    * _sender: ");       Serial.println(DD._sender);
    Serial.print("    * _command: ");      Serial.println(DD._command);
    Serial.print("    * temprature: ");    Serial.println(DD.temprature);
    Serial.print("    * batStat: ");       Serial.println(DD.batStat);
    Serial.print("    * fanStatus: ");     Serial.println(DD.fanStatus);
    Serial.print("    * setPoint_temp: "); Serial.println(DD.setPoint_temp);
    Serial.print("    * ventStatus: ");    Serial.println(DD.ventStatus);
}

void handle_controller_message(void) 
{
    hc_recvFlag = 0;
    if ((hc_mesg.begin_validator[0] == 'V') & (hc_mesg.begin_validator[1] == 'A') & (hc_mesg.begin_validator[2] == 'C') &
        (hc_mesg.end_validator == 'H'))
    {
        Serial.println("data recieved from controller:");
        print_hcMessage(hc_mesg);
        if (hc_mesg.__hcdata == 0x01) // 1 = introduce controller
        {
            hc_mesg.__hcdata = 0x02;
            hc_mesg.reciever_MAC_addr[0] = localIp[0];
            hc_mesg.reciever_MAC_addr[1] = localIp[1];
            hc_mesg.reciever_MAC_addr[2] = localIp[2];
            hc_mesg.reciever_MAC_addr[3] = localIp[3];

            for (int i=0; i<6; i++)  controller_MAC[i] = hc_mesg.sender_MAC_addr[i];
            Serial.printf("Controller MAC address is: %02X:%02X:%02X:%02X:%02X:%02X\n",
            controller_MAC[0], controller_MAC[1], controller_MAC[2], controller_MAC[3], controller_MAC[4], controller_MAC[5]); 
            hc_sendFlag = 1;
        }
        else if (hc_mesg.__hcdata == 0x03) // 3 = cont.: this is a message from a device (sender mac address)
        {
            if (hc_mesg._command == 0x01) //a device requests to be paired
            {
                if (pair_request_flag == 1)  // user is searching for a new device
                {
                    if (redundant_macAdd_check(hc_mesg.sender_MAC_addr) == false)
                    {
                        pair_request_flag = 0;
                        //////////////////////////////////////////////// save new device on flash & database
                        String dev_name;
                        dev_name = add_device_to_zone(globalZoneID, hc_mesg._sender , hc_mesg.sender_MAC_addr);
                        if (dev_name != "no name")
                        {
                            // notify controller and the device that it is being added
                            hc_mesg.__hcdata = 4; // 4 = host: pair a device with sender MAC Add
                            hc_mesg.zone_id_group = 1 << globalZoneID;
                            Serial.println("Device added successfully, command controller to pair device and acknowlodge it.");
                            Serial.print("Zone ID = "); Serial.println(hc_mesg.zone_id_group);
                            hc_sendFlag = 1;
                            // notify browsers
                            String newDev = "Add Device," + String(zones[globalZoneID].name) + "," + dev_name; 
                            Serial.println(newDev);
                            notifyClients_txt(newDev);
                        }    
                    }
                }
            }
            if (hc_mesg._command == 0x02) // execute an order
            {
                if (hc_mesg._sender == 0x02) // termostat
                {
                    uint32_t zone_id_local = 0;
                    while (zone_id_local > 0) 
                    {
                        hc_mesg.zone_id_group >> 1;
                        zone_id_local += 1;
                    }
                    zones[globalZoneID].termos[globalDeviceID].batteryState = hc_mesg.batStat;
                    zones[globalZoneID].termos[globalDeviceID].setPoint = hc_mesg.setPoint_temp;
                    zones[globalZoneID].termos[globalDeviceID].temperature = hc_mesg.temprature;

                    String devices_stat = "Device Status," + String(zones[globalZoneID].name) + ","
                                        + String(zones[globalZoneID].termos[globalDeviceID].name) + "," 
                                        + String(hc_mesg.batStat) + ","
                                        + String(hc_mesg.setPoint_temp) + ","
                                        + String(hc_mesg.temprature);
                    Serial.println(devices_stat);
                    notifyClients_txt(devices_stat);
                }
            }
            if (hc_mesg._command == 0x03) // read status
            {
                zones[globalZoneID].termos[globalDeviceID].batteryState = hc_mesg.batStat;
                zones[globalZoneID].termos[globalDeviceID].setPoint = hc_mesg.setPoint_temp;
                zones[globalZoneID].termos[globalDeviceID].temperature = hc_mesg.temprature;

                String devices_stat = "Device Status," + String(zones[globalZoneID].name) + ","
                                    + String(zones[globalZoneID].termos[globalDeviceID].name) + "," 
                                    + String(hc_mesg.batStat) + ","
                                    + String(hc_mesg.setPoint_temp) + ","
                                    + String(hc_mesg.temprature);
                Serial.println(devices_stat);
                notifyClients_txt(devices_stat);

            }
        }
        else if (hc_mesg.__hcdata == 0x06) // 6 = cont.: delivery to device failed (dev mac = reciver mac address)
        {
           String devices_stat = "Device Status," + String(zones[globalZoneID].name) + ","
                                    + String(zones[globalZoneID].termos[globalDeviceID].name) + "," 
                                    + "Not Available" + ","
                                    + "Not Available" + ","
                                    + "Not Available";
            Serial.println(devices_stat);
            notifyClients_txt(devices_stat);
        }
    }
    else
    {
      Serial.println("Unknow data from controller!!! WTF");
      Serial2.read();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_browser_message(char *data, size_t len, uint32_t client_id)
{
    Serial.print("data from browser: ");Serial.printf((char *)data);Serial.println("");
    String mydata = String(data);
    String strs[20];
    int StringCount = 0;
    // interpret data from web
    while (mydata.length() > 0)
    {
        int index = mydata.indexOf(',');
        if (index == -1) // No seperator found
        {
            strs[StringCount++] = mydata;
            break;
        }
        else
        {
            strs[StringCount++] = mydata.substring(0, index);
            mydata = mydata.substring(index+1);
        }
    }
    // print data from web to serial
    for (int i = 0; i < StringCount; i++) {
        Serial.print(i);  Serial.print(": \"");    Serial.print(strs[i]);    Serial.println("\"");  }

    if (strs[0] == "Get Zone Names")
    {
        notify_a_client_txt(get_zone_names(), client_id);

        String FanS, ModeS;
        if (FanState == 0) FanS = "auto";
        if (FanState == 1) FanS = "on";
        if (ModeState == 0) ModeS = "off";
        if (ModeState == 1) ModeS = "heat";
        if (ModeState == 2) ModeS = "cool";
        notify_a_client_txt("Change State to," + FanS, client_id);
        notify_a_client_txt("Change State to," + ModeS, client_id);
    }
    if (strs[0] == "Add New Zone")
    {
        Serial.println("adding new zone...");
        add_new_zone(strs[1], strs[2]);
        notifyClients_txt("Zone Names," + strs[1]);
    }
    if (strs[0] == "Delete Zone")
    {
        Serial.println("deleting a zone...");
        delete_zone(strs[1]);
        notifyClients_txt("Delete a Zone," + strs[1]);
    }
    if (strs[0] == "Search Device")
    {
        Serial.println("search for paring a new device...");
        pair_request_flag = 1;
        globalZoneID = get_zoneID_by_name(strs[1]);
    }
    if (strs[0] == "Zone Device Names")
    {   
        globalZoneID = get_zoneID_by_name(strs[1]);
        String devices_names = "Device Names," + String(strs[1]);
        devices_names += get_zoen_device_names(globalZoneID);
        Serial.println(devices_names);
        notify_a_client_txt(devices_names, client_id);
    }
    if (strs[0] == "Get Device now Status")
    {   
        globalZoneID = get_zoneID_by_name(strs[1]);
        globalDeviceID = get_DeviceID_by_name(globalZoneID, strs[2]);
        hc_mesg.__hcdata = 0x03; // host : send this message directly to device (reciver mac addr) 
        hc_mesg._sender = 0x01;
        hc_mesg._command = 0x03;
        for (int i=0; i<6; i++){
            hc_mesg.reciever_MAC_addr[i] = zones[globalZoneID].termos[globalDeviceID].MAC_addr[i];
            hc_mesg.sender_MAC_addr[i] = controller_MAC[i];
        }
        hc_sendFlag = 1;
    }
    if (strs[0] == "Remove Device")
    {   
        globalZoneID = get_zoneID_by_name(strs[1]);
        globalDeviceID = get_DeviceID_by_name(globalZoneID, strs[2]);
        zones[globalZoneID].termos[globalDeviceID].isActive = 0;
        hc_mesg.__hcdata = 0x05; // host : unpair devie
        hc_mesg._sender = 0x01;
        hc_mesg._command = 0x04;
        for (int i=0; i<6; i++){
            hc_mesg.reciever_MAC_addr[i] = zones[globalZoneID].termos[globalDeviceID].MAC_addr[i];
            hc_mesg.sender_MAC_addr[i] = controller_MAC[i];
        }
        Serial.println("send data for unpair a device:");
        print_hcMessage(hc_mesg);
        hc_sendFlag = 1;
        notifyClients_txt("Delete a Device," + strs[1] + "," + strs[2]);
    }
    if (strs[0] == "Change Setpoint")
    {   
        globalZoneID = get_zoneID_by_name(strs[1]);
        globalDeviceID = get_DeviceID_by_name(globalZoneID, strs[2]);
        hc_mesg.__hcdata = 0x03; // host : send this message directly to device (reciver mac addr) 
        hc_mesg._sender = 0x01; //controller
        hc_mesg._command = 0x02; // execute order
        hc_mesg.setPoint_temp = strs[3].toInt();

        zones[globalZoneID].termos[globalDeviceID].setPoint = hc_mesg.setPoint_temp;

        for (int i=0; i<6; i++){
            hc_mesg.reciever_MAC_addr[i] = zones[globalZoneID].termos[globalDeviceID].MAC_addr[i];
            hc_mesg.sender_MAC_addr[i] = controller_MAC[i];
        }
        Serial.println("send data to change setpoint");
        print_hcMessage(hc_mesg);
        hc_sendFlag = 1;
    }
    if (strs[0] == "Change State")
    {
        Serial.print("change state to ");Serial.println(strs[1]);
        if (strs[1] == "auto") FanState = 0;
        if (strs[1] == "on") FanState = 1;
        if (strs[1] == "off") ModeState = 0;
        if (strs[1] == "heat")  ModeState = 1;
        if (strs[1] == "cool") ModeState = 2;
        Serial.println(ModeState);
        notifyClients_txt("Change State to," + strs[1]);
    }

    if (strs[0] == "Change Manual")
    {
        Serial.print("change manual of "); Serial.print(strs[1]); Serial.print(" to "); Serial.println(strs[2]);
        // if (strs[1] == "auto") FanState = 0;
        // if (strs[1] == "on") FanState = 1;
        // if (strs[1] == "off") ModeState = 0;
        // if (strs[1] == "heat")  ModeState = 1;
        // if (strs[1] == "cool") ModeState = 2;
        // Serial.println(ModeState);

        
        notifyClients_txt("Change Manual to," + strs[1] + "," + strs[2]);
        Serial.println("Notify clients: Change Manual to," + strs[1] + "," + strs[2]);
    }
}