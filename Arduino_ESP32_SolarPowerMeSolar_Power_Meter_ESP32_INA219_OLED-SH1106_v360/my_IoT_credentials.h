// this file store all of the private credentials, passwords and connection details

// WiFi config for ESP32 Solar_Power_Meter
#define  my_external_WiFi_IotWebConf_initial_ApSSID "Solar-Meter_ESP32"       // Initial name of the ESP32 device, used as SSID of the own Access Point, can be changed in UI
#define  my_external_WiFi_IotWebConf_initial_ApPassword "12345678"            // Initial password to connect to the ESP32 device, when it creates an own Access Point, can be changed in UI


// ThingSpeak config for ESP32 Solar_Power_Meter (https://thingspeak.com/channels)
#define my_external_ThingSpeak_Channel_ID     xxxxxx                          // my personal ThingSpeak channel number
#define my_external_ThingSpeak_Write_API_Key  "xxxxxxxxxxxxxxxx"              // my private ThingSpeak channel write API Key
#define my_external_ThingSpeak_Read_API_Key   "xxxxxxxxxxxxxxxx"              // my private ThingSpeak channel read API Key
