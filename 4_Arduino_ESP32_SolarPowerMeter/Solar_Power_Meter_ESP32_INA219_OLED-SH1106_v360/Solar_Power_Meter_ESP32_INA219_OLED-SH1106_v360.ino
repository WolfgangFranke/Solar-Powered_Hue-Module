float SW_version = 3.60;

/*
  ' ######################################################################
  ' ###
  ' ###       *** Solar Power-Meter with ESP32 Pico-Kit v4 and INA219 ***
  ' ###       ----------------------------------------------------------
  ' ###       *Wolfgang Franke*
  ' ###       Projekt v1.0: 01. July 2019 - 10. July 2019 (Prototype Software)
  ' ###       Software-Version 1.50    (used IDE: Arduino v1.8.9)
  ' ###       Projekt v1.6: 10. Aug 2019 - 11. Aug 2019 (Arduino Hardware+Software)
  ' ###       Software-Version 1.60    (used IDE: Arduino v1.8.9)
  ' ###       Projekt v2.0: 10. Jan 2020 - 11. Jan 2020 (port to ESP32 Hardware+SW)
  ' ###       Software-Version 2.0x    (used IDE: Arduino v1.8.10)
  ' ###       Projekt v3.0: 11. Jan 2020 - 19. Jan 2020 (added Wifi+IoT in ESP32 Software)
  ' ###       Software-Version 3.4x    (used IDE: Arduino v1.8.10)
  ' ###       Projekt v3.5: 08. Feb 2020 - 09. Feb 2020 (added NTP in ESP32 Software)
  ' ###       Software-Version 3.5x    (used IDE: Arduino v1.8.10)
  ' ###       Projekt v3.6: 25. Feb 2020 - 26. Feb 2020 (added corrected ADC in ESP32 Software)
  ' ###       Software-Version 3.6x    (used IDE: Arduino v1.8.10)
  ' ###       ----------------------------------------------------------
  ' ###
  ' ###       Hardware Setup:
  ' ###           + ESP32 Pico-Kit v4 (5V Micro-USB, intern 3,3V, 80 MHz, 4MB Flash)
  ' ###           + OLED 1,3" SH1106 128x64 (I2C Address: 0x3C) with U8g2lib Library
  ' ###           + INA219 Shunt-Voltage meter (I2C Addresses: 0x40-0x4F) using own I2C Module selection routines
  ' ###           + LED to signal WiFi status (L-active)
  ' ###
  ' ###       Software capabilities:
  ' ###       --v1.5/1.6 functions on Arduino Nano:
  ' ###       (1) measure the Current, Voltage, Power and Energy produced by a Solar Panel (connected via a Shunt resistor to the load)
  ' ###             - Solar Panel:  up to 24V, 20W (max 1A)
  ' ###             - INA219 cartridges: using 6 INA219 cartridge with different Shunt resistors and each having an own I2C address
  ' ###       (2) INA219 measures Shunt Voltage , will be displayed on OLED
  ' ###             - measurement loop: sum up the Energy (U*I*time) in 1s intervals (Integral of Power over time in Ws) and display as Energy in Wh,
  ' ###             - INA219 Cartridge hot-swap: before every measure determine the INA2019 module (shunt, volt) by I2C address
  ' ###             - INA219 auto-select PGA/GainDiv: before every measure determine the PGA/GainDiv with highes ADC-resolution in INA219
  ' ###             - INA219 config: V-Shunt-max = 40-80-160-320mV (PGA/ainDiv) ; V-Bus=32V ; ADC=12bit ; Single Conversation, Continuous Mode, but no calibration,
  ' ###             - INA219 used to measure Shunt and Bus Voltage only, but then current, power, energy (current/max/day) are calculated based on used Shunt,
  ' ###             - the INA219 internal calculation and calibration of current and power is not used here,
  ' ###       (3) Auto-Reset data over night (darkness of x hours)
  ' ###             - reset max current/power and day energy,
  ' ###             - after day change, keep last day's value on display,
  ' ###
  ' ###       --v2.0 changes from Arduino to ESP32:
  ' ###             - 1s interval more precise: use a ESP32 Timer-INT to generate the 1 sec interval for main loop (with Arduino was a trimmed/hacked 1s in the main loop)
  ' ###             - ADC: ESP32 ADC default is 12bit ADC, using a 3,3V reference, with R1/R2=10k/10k (Arduino was 10bit; 1,1V internal reference; R1/R2=4,7k/800R)
  ' ###             - delay() function removed: for WiFi config process a non blocking delay (useing millies) is used, in all other cases using ESP32's vTaskDelay(ms/portTICK_PERIOD_MS) , not Arduino delay() anymore;
  ' ###             - externalized the private credentials to ".h" file
  ' ###
  ' ###       --v3.0 additional features using ESP32 capabilities:
  ' ###
  ' ###       (4) WiFi connectivity implemented by using IotWebConf library for ESP8266/ESP32
  ' ###             - IotWebConf library allows to dynamically enter the WiFi config by Web interface, no need anymore to store SSID and PW in source code
  ' ###             - Source: Arduino Library Manager, or https://github.com/prampec/IotWebConf (author: Balázs Kelemen)
  ' ###             - it's a non blocking WiFi/AP web configuration library for Arduino, no reboot to switch WiFi settings,
  ' ###             - supports a WiFi connection status LED, and a Switch to open an AP at power-on to config WiFi,
  ' ###             - Bug: the version IotWebConf v2.3.1 does only support WiFi passwords of <33 characters,
  ' ###                 patch: modify in IotWebConf.h to line  #define IOTWEBCONF_PASSWORD_LEN 65  (was 33)
  ' ###             - FYI: in case of WiFi-AP connection issues in test-phase of your scetch, then manually delete the first byte of EEPROM to invalidate the WiFi config,
  ' ###
  ' ###       (5) IoT Cloud: ThingSpeak analytic IoT platform service (https://www.thingspeak.com)
  ' ###             - Source: Arduino Library Manager, or https://github.com/mathworks/thingspeak-arduino
  ' ###             - sending 4 data items and Status to a channel in ThingSpeak cloud, by WebClient with HTTP-GET
  ' ###             - setting a time interval to send data (around every 10-15 min (600/900s), data items are float (mA, V, max-W, Wh-day) and string (status)
  ' ###
  ' ###       --v3.5 additional features using NTP capabilities:
  ' ###
  ' ###       (6) NTP support implemented by using Arduino (ESP32) library for ESP32
  ' ###             - Arduino (ESP32) Time library gets time from NTP servers (up to 3 can be defined), translates to local time, and set ESP32 RTC,
  ' ###             - Source: default Arduino (ESP32) library
  ' ###             - if NTP available, then reset daily values at midnight (trigger at 23:59:55), otherwise reset current day's values after detecting some hours of darkness,
  ' ###             - NTP/RTC time re-synch, daily 04:00:00 a.m. synch MCU time with NTP Server, and send NTP-status to ThingSpeak status,
  ' ###
  ' ###       --v3.6 additional features to measure an alternative Voltage (e.g. a Solar or Battery Voltage) using ESP32 ADC:
  ' ###
  ' ###       (7) ADC sub-routine for simple correction of non-linearity in ESP32 ADC measure curve (no correction for the unpreciseness of ESP32 internal U-Ref)
  ' ###             - issue: ESP32 ADC curve is very non-linear (up to -25%), and internal reference voltage of 1,1V can range from 1,0V to 1,2V (issue doc: esp32.com/viewtopic.php?f=19&t=2881)
  ' ###             - Source: github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
  ' ###             - re-calc ADC readings with some polynomial correction, achieves good ±2% in mid range volt, but still >10% near 0 and near 4095
  ' ###
  ' ###       (x) known limitations:
  ' ###         - The ThingSpeak Status message as generated in Sub "normal_ThingSpeak_Status_Msg()" use hard coded IF-THEN values, but could be auto-calibrating for the Solar Cells, Li-Ion charger and battery (is hard coded to the 24V Solar Cells, the LT5236 charger, the ESP32 ADC7 for Solar-Voltage and INA219 for Charge-Current which I use in my home installation),
  ' ###         - Not an issue for measuring solar power, but generally: the time interval for data upload to ThingSpeak (every 15 min) is not synched with the NTP time which triggers the reset of daily max-values (00:00), means losing the maxima occuring <15 minutes before NTP reset
  ' ###         - FYI only: after darkness has reset the max/day values, the counter Darkness_Counter_sec keeps incrementing a long-var, until the next daylight resets it,
  ' ###
  ' ######################################################################

  ' ######################################################################
  ' ###  Pinbelegungen
  ' ###  -----------------------------------------------------------------
  ' ###  Arduino      ATMega328P        ESP32 PicoKitv4   external circuit
  ' ###  Nano                                             Beschaltung_WF
  ' ###  -----------------------------------------------------------------
  ' ###  V-In/RAW     -                                   not used, disabled in Arduino Nano
  ' ###  3,3V         -                                   not used, Arduino Vout=3,3V (I-out<100mA?)
  ' ###  5V           -                                   5V+=V-USB=VCC, removed internal Diode in Arduino Nano allows use of full 5V+ from V-USB
  ' ###  Vcc          (07)                                5V+ = V-USB
  ' ###  Vcc          (20)                                5V+ = V-USB
  ' ###  GND          (08)                                GND
  ' ###  GND          (22)                                GND
  ' ###
  ' ###                                 5V                USB power for ESP32 Module
  ' ###                                 3,3V              3,3V=VCC for ESP32-Chip, OLED and INA219 on I2C
  ' ###
  ' ###
  ' ###  A0           (21)              ADC1_CH6=GPIO-34  measures VCC (ESP32: via 10k/10k against URef=3,3V; Arduino: via 4,7k/800 against URef=1,1V)
  ' ###                                 ADC1_CH7=GPIO-35  measures V-Solar (ESP32: via 10k/100k to GND, against URef=3,3V)
  ' ###  A5/SCL       (28)PC5           GPIO-22           I2C-SCL (external Pull-Up 10k to Vcc=3,3V -or- 1k with 2m long I2C cable)
  ' ###                                 GPIO-14           LED to VCC, used by IotWebConf library on runtime to indicated WiFi connection status
  ' ###                                 GPIO-27           Switch to GND (not soldered in), used by IotWebConf library on startup to open AP with SSID+PW from source code
  ' ###
  ' ###  RESET        (01)              EN                onboard Reset-Button to GND, with onboard 10k Pull-Up
  ' ###                                 GPIO-0            onboard Boot-Button to GND, with external 10k Pull-Up (onboard on PicoKit v4.1)
  ' ###                                 GPIO-13           RTS for programming
  ' ###                                 GPIO-15           CTS for programming
  ' ###                                 GPIO-34..39       FYI: Input only
  ' ###
  ' ######################################################################

  ' ######################################################################
  ' ###  INA 219 registers and config
  ' ###  --------------------------------------------------------------
  ' ###  my cartridge I2C addresses:
  ' ###     0x40 =  0,1Ω      Imax = 3,2A  ; max ADC-resolution = 100µA
  ' ###     0x41 =   1Ω       Imax = 320mA ; max ADC-resolution = 10µA
  ' ###     0x44 =  10Ω       Imax = 32mA  ; max ADC-resolution = 1µA
  ' ###     0x45 = 100Ω       Imax = 3,2mA ; max ADC-resolution = 0,1µA
  ' ###     0x46 =  1kΩ       Imax = 0,3mA ; max ADC-resolution = 0,01µA
  ' ###     0x4F = 0,1Ω for 52V
  ' ###
  ' ###  INA219 Register numbers:
  ' ###     0x00: INA219_REG_CONFIG
  ' ###     0x01: INA219_REG_SHUNTVOLTAGE (value is signed-int in right 9-12bit + x left signs, depend on ADC resolution)
  ' ###     0x02: INA219_REG_BUSVOLTAGE   (value is in bit 15-3 (32V) or 14-3 (16V), LSB always 4mV) -> (shift 3 bits right to drop CNVR and OVF and multiply by LSB)
  ' ###     0x03: INA219_REG_POWER        (not usedd here - will only fill values after Calibration register was written)
  ' ###     0x04: INA219_REG_CURRENT      (not usedd here - will only fill values after Calibration register was written)
  ' ###     0x05: INA219_REG_CALIBRATION  (not usedd here - needs the calibration value dependend from measure type)
  ' ###
  ' ###  init INA219 config register (GainDiv=8; Bus-Volt=32V; ADC=12bit; ADC=1sample/532µs; ADC continuous)
  ' ###  INA219_config = INA219_CONFIG_GAIN_8_320MV |
  ' ###                  INA219_CONFIG_BVOLTAGERANGE_32V |
  ' ###                  INA219_CONFIG_BADCRES_12BIT |
  ' ###                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
  ' ###                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  ' ###
  ' ###   Values for Gain-Div bits -> Gain Mask: INA219_CONFIG_GAIN_MASK (0x1800)
  ' ###     INA219_CONFIG_GAIN_8_320MV = (0x1800), // Gain 8, 320mV Range
  ' ###     INA219_CONFIG_GAIN_4_160MV = (0x1000), // Gain 4, 160mV Range
  ' ###     INA219_CONFIG_GAIN_2_80MV = (0x0800),  // Gain 2, 80mV Range
  ' ###     INA219_CONFIG_GAIN_1_40MV = (0x0000),  // Gain 1, 40mV Range
  ' ###
  ' ###   write config to INA219: I2C_Write_Register(INA219_REG_CONFIG, INA219_config);
  ' ###
  ' ######################################################################

  ' ######################################################################
  ' ###  ESP32 NTP and time
  ' ###  --------------------------------------------------------------
  ' ###  *Get time from NTP Server as Unix-Timestamp + save in struct tm + set MCU clock:
  ' ###    1) configTime(0, 0, ntpServer);   // prepare to connect to NTP Servers (allows 3 names comma separated)
  ' ###    2) getLocalTime(&RTC_time_components)        // send request to NTP server, parse received NTP time packet (Unix-Timestamp in Seconds since 1.1.1970)
  ' ###                                          and save in a tm-struct, http://www.cplusplus.com/reference/ctime/localtime/
  ' ###    3) setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);  // set Time-Zone https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  ' ###
  ' ###  *Before time operations of 5+6, get current MCU time and convert in tm-struct as local time :
  ' ###    4a) time_t NTP_time_UnixSec = time(&NTP_time_UnixSec);        // fill a time struct with the current local time as Unix-Seconds
  ' ###    4b) localtime_r(&NTP_time_UnixSec, &RTC_time_components);          // Convert Unix-Seconds to RTC_time_components as local time, http://www.cplusplus.com/reference/ctime/localtime/
  ' ###
  ' ###  *Print local time as string values from struct tm :
  ' ###    5a) Serial.println(&RTC_time_components, "%A, %B %d %Y %H:%M:%S");     // format: Saturday, February 08 2020 21:52:01
  ' ###    5b) strftime (buf, sizeof(buf), "%d.%m.%Y %T ", &RTC_time_components); // format arguments: full list at http://www.cplusplus.com/reference/ctime/strftime/
  ' ###
  ' ###  *Process single data of local time values from struct tm :
  ' ###    6) access struct tm.fields;         // http://www.cplusplus.com/reference/ctime/tm/
  ' ###
  ' ###  *Re-synch with NTP (e.g. daily) :
  ' ###    7) re-synch MCU time with NTP server by re-doing steps 1,2,3 :
  ' ###            e.g. daily 4am: if ((RTC_time_components.tm_hour == 04) && (RTC_time_components.tm_min == 00) && (RTC_time_components.tm_sec == 0))
  ' ###            after successful getLocalTime(), wait for change of the second (Unix timestamp), to let the new NTP time take effect in time(&ow),
  ' ###            after successful getLocalTime(), change of the data in "struct tm" takes even longer than that change of second
  ' ###            BUG: any second getLocalTime() never returns an Error, even if the NTP is not reachable or WiFi=OFF
  ' ###
  ' ######################################################################
*/



//#########################################################################
//  ******  MCU setting, Hardware declaration, compiler definitions  ******
//#########################################################################

#include "my_IoT_credentials.h"       // external storage of the 5 personal credentials, WiFi passwords and IoT-Cloud connection keys in this seperate file
  //#define  my_external_WiFi_IotWebConf_initial_ApSSID "WF_Solar-Meter_ESP32"    // Initial name of the ESP32 device, used as SSID of the own Access Point, can be changed in UI
  //#define  my_external_WiFi_IotWebConf_initial_ApPassword "12345678"            // Initial password to connect to the ESP32 device, when it creates an own Access Point, can be changed in UI
  //#define  my_external_ThingSpeak_Channel_ID     959668             // my ThingSpeak channel number
  //#define  my_external_ThingSpeak_Write_API_Key  "..."              // my ThingSpeak channel write API Key
  //#define  my_external_ThingSpeak_Read_API_Key   "..."              // my ThingSpeak channel read API Key

#include <ThingSpeak.h>       // ThingSpeak v1.5.0 - library for ThingSpeak analytic IoT platform service (https://www.thingspeak.com) 
// source: Arduino Library Manager, or https://github.com/mathworks/thingspeak-arduino

#include <IotWebConf.h>       // IotWebConf v2.3.0. - library for managing WiFi connectivity for ESP8266/ESP32
// source: Arduino Library Manager, or https://github.com/prampec/IotWebConf (author: Balázs Kelemen)
// it includes: <WiFi.h> , <WebServer.h> , <DNSServer.h>  (for WiFi connection, AP and captive portal)
// IotWebConf allows to dynamically enter the WiFi config by Web interface, no need anymore to store SSID and PW in source code

#include "Wire.h"             // Library for I2C (1Wire)
// source: Arduino Standard Library

#include "time.h"             // Standard of C Time Library with functions to get NTP time from NTP servers and to manipulate date and time information
// source: Arduino Standard Library

#include <Adafruit_INA219.h>  // Adafruit_INA219 v1.0.6 - library to read INA219 Current and Voltages (only variable definitions used)
// source: Arduino Library Manager: Adafruit INA219
// Class library not used, as it supports only a single I2C address and I want to use Hot-Plug, and all PGA-configs for auto ranging
// Adafruit INA219 Library: https://github.com/adafruit/Adafruit_INA219

#include <U8g2lib.h>          // U8g2lib v2.27.6 - library for OLED with SH1106
// source: Arduino Library Manager: U8g2 - U8glib library for monochrome displays, version 2
// U8g2reference:    https://github.com/olikraus/u8g2/wiki/u8g2reference
// available fonts:  https://github.com/olikraus/u8g2/wiki/fntlistall
// FYI: 7 Pixel Height :  u8g2_font_profont11_mf / u8g2_font_lucasfont_alternate_tf / u8g2_font_mozart_nbp_tf / u8g2_font_haxrcorp4089_tr
// FYI: 8 Pixel Height + fixed width :  u8g2_font_t0_11_mf

#define OLED_I2C_Address  0x3C           // Address of OLED (0x3C for China Module, 0x3D for Adafruit compatible)
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLED(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // init a OLED display of library U8g2lib
// OLED SH1106 display connected to I2C
// GUIDE: U8g2 full buffer, page buffer and u8x8 mode:  https://github.com/olikraus/u8g2/wiki/setup_tutorial#u8g2-full-buffer--page-buffer-and-u8x8-mode
// "F" = Full-Buffer Mode is faster than "1" = Page-Buffer Mode in: U8G2_SH1106_128X64_NONAME_1_HW_I2C OLED(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


// *** DEBUG LEVELS ON/OFF ***
#define debug_level 2               // 0=Debug=OFF: no serial-prints 
// 1=Debug: serial-print only for initial I2C init result and I2C Runtime Errors
// 2=Debug: serial-print also for changes in main loop

//-------------------------------------------------------------------------
//  ******  End of MCU setting, Hardware declaration, compiler definitions  ******
//-------------------------------------------------------------------------



//#########################################################################
//   ****** global variables definitions   ******
//#########################################################################
// global variables

// ThingSpeak config for ESP32 Solar_Power_Meter
unsigned long ThingSpeak_upload_Interval_seconds = 900;                                  // 900s = 15 min as interval for sending IoT-data to ThingSpeak Cloud
unsigned long ThingSpeak_upload_Interval_StartSec = 0;                                   // store start second of last interval befor IoT-data to ThingSpeak Cloud

unsigned long my_ThingSpeak_ChannelNumber = my_external_ThingSpeak_Channel_ID;           // my ThingSpeak channel number, loaded from #include "my_IoT_credentials.h" 
const char * my_ThingSpeak_WriteAPIKey    = my_external_ThingSpeak_Write_API_Key;        // my ThingSpeak channel write API Key, loaded from #include "my_IoT_credentials.h" 

// using all 4 channel's and the status field available in the ThingSpeak Free license:
float lastInterval_average_Shunt_Current_mA = 0;                // last interval's average of solar Current (mA) = Shunt_Voltage_µV/1000 / cartridge_R_SHUNT_OHM  (I=U/R)
float lastInterval_average_Supply_Voltage_V = 0;                // last interval's average of solar Voltage (V) = Bus_Voltage_V + ((Shunt_Voltage_µV/1000) / 1000)
float lastInterval_maxPower_W = 0;                              // last interval's peak solar power (W) = Supply_Voltage_V * (Shunt_Current_mA / 1000)  (P=U*I)


// WiFi WebConfig variables for ESP32 Solar_Power_Meter
const char WiFi_IotWebConf_initial_ApSSID[]     = my_external_WiFi_IotWebConf_initial_ApSSID;     // "myThingName" Initial name of the ESP32 device, used as SSID of the own Access Point, can be changed in UI
const char WiFi_IotWebConf_initial_ApPassword[] = my_external_WiFi_IotWebConf_initial_ApPassword; // "12345678" Initial password (min 8 char) to connect to the ESP32 device, when it creates an own Access Point, can be changed in UI
#define WiFi_IotWebConf_Button_Pin      27   // when Button_Pin=low on startup, IotWebConf will open an AP, using the initial SSID+PW from SourceCode (e.g. in case of lost custom password)
#define WiFi_IotWebConf_Status_LED_Pin  14   // LED to 3,3V as WiFi status indicator pin (fast blink: searching Wifi and open AP, slow flashing: connected to the your Wifi)


// NTP and RTC time variable
const char* RTC_local_TZ_DST = "CET-1CEST,M3.5.0/02,M10.5.0/03";            // local TZ and DST, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* const PROGMEM ntpServer[] = {"de.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org", "pool.ntp.org"};
struct tm RTC_time_components;            // struct containing calendar date and time broken down into its components, see www.cplusplus.com/reference/ctime/tm/
time_t RTC_time_last_sec {0};             // keeps last second of NTP-Time loop
time_t NTP_time_UnixSec = time(&NTP_time_UnixSec);  // fill a time struct with the current local time as Unix-Seconds
byte NTP_time_valid = 0;                  // flag if time was synched with NTP
byte NTP_msg_for_ThingSpeak_status_flag = 0;                 // flag if ThingSpeak status should be overwritten with NTP update status

// ADC variables
const byte ADC_pin_VCC = 34;              // pin used to ADC the VCC/V-USB (by voltage devider)
const byte ADC_pin_Vsolar = 35;           // pin used to ADC the V-Solar (by voltage devider)
double ADC_CorrectedVoltage;              // ADC Correction by using a binominal function

const byte myINA219_I2C_addr[] = {        // I2C addresses of my INA219-cartridges (range: 0x40-0x4F)
  0x40,   // Shunt   0.1 Ohm  /  3,2A
  0x41,   // Shunt   1.0 Ohm  / 320mA
  0x44,   // Shunt  10.0 Ohm  /  32mA
  0x45,   // Shunt 100.0 Ohm  / 3,2mA
  0x46,   // Shunt   1.0 kOhm / 0,3mA
  0x4F    // Shunt   0.1 Ohm  /  3,2A for 52V
};
byte cartridge_INA219_I2C_addr = 0;       // current I2C addresses (one of my INA219-cartridges)
float cartridge_R_SHUNT_OHM = 0;          // Shunt Resistor value in Ohms
byte cartridge_Voltage_Devider = 1;       // cartridge 0x4F devides the Solar Voltage by 2, to support up to 52V

int actual_V_SHUNT_GainRange_mV;          // holds max possible Voltage (in mV) over shunt (can be 8=320, 4=160, 2=80, 1=40mV)
long Shunt_Voltage_uV = 0;                // shunt volt (in µV) from INA219 over shunt (factor 10 * (INA219 raw of +-32767 signed-INT))
float Shunt_Current_mA = 0;               // = Shunt_Voltage_µV/1000 / cartridge_R_SHUNT_OHM  (I=U/R)
float Bus_Voltage_V = 0;                  // measured by INA219
float Supply_Voltage_V = 0;               // = Bus_Voltage_V + ((Shunt_Voltage_µV/1000) / 1000)
float Power_W = 0;                        // = Supply_Voltage_V * (Shunt_Current_mA / 1000)  (P=U*I)

float Shunt_Current_daymax_mA = 0;      // = Shunt_Voltage_µV/1000 / cartridge_R_SHUNT_OHM  (I=U/R)
float Supply_Voltage_daymax_V = 0;      // = Bus_Voltage_V + (Shunt_Voltage_µV/1000 / 1000)
float Power_daymax_W = 0;               // = Supply_Voltage_V * (Shunt_Current_mA / 1000)  (P=U*I)

float Power_Sum_Ws_today = 0;           // Energy Counter (Ws) = Integral of Watt over today (Ws)
float Power_Sum_Wh_today = 0;           // Energy Counter (Wh) = Power_Sum_Wsec_today / 3600
float Power_Sum_Wh_yesterday = 0;       // Energy Counter (Wh) for yesterday


const float Darkness_Solar_Volt = 0.01; // define a low Solar Voltage to detect darkness
const long Darkness_Hours_to_Reset = 4; // define seconds of darkness to reset last day's max-values (1h = 3600s)
const int Light_Sec_to_wakeup = 60;     // define seconds of light to end darkness

unsigned long Power_On_Seconds = 0;     // counts seconds since power-on (u-long seconds = 136 Jahre)
unsigned long Darkness_Counter_sec = 0; // counts seconds of detected darkness
unsigned long Light_Counter_sec = 0;    // counts seconds of detected light to end a darkness period
byte Darkness_waits_to_Reset_flag = 0;  // indicates if darkness was detected and now waits to reset today's values

byte heartbeat_pixel = 0;               // flag for heartbeat by toggling a single pixel
byte OLEDprint_decimalplaces_shunt_mA;  // decimal places for Shunt_Current_mA on OLED are dependent from Cartridge and digits of value

unsigned long MilliSec_Start;           // store start of millis for a non-blocking delay

char string_buf22[22];                  // String Buffer for e.g. date/time manipulations
String string_buf;                      // String Buffer for concat ThingSpeak status

//-------------------------------------------------------------------------
//   ****** End of global variables definitions ******
//-------------------------------------------------------------------------


//#########################################################################
//   ****** hardware decalarations, ISR and software inits   ******
//#########################################################################

// init of WebServer for IotWebConf (WiFi Manager), WebClient for ThingSpeak
DNSServer dnsServer;                          // start DNS Server for Captive Portal in own AP of IotWebConf
WebServer ESP32WebServer(80);                 // start WebServer on Port 80 for AP of IotWebConf
IotWebConf iotWebConf(WiFi_IotWebConf_initial_ApSSID, &dnsServer, &ESP32WebServer, WiFi_IotWebConf_initial_ApPassword); // init a IotWebConf as WiFi Config manager
WiFiClient  ESP32WebClient;                   // start WebClient for ThingSpeak

// Timer declaration
hw_timer_t * timer0 = NULL;                              // pointer to a variable of type hw_timer_t, used in the Arduino "timerBegin()" function
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;   // variable of type portMUX_TYPE, to protect read/write of data values by a Critical Section in ISR and main loop,
volatile int Timer0_INT_counter;                        // flag/counter/semaphore to signal the 1s from ISR to main-loop (shared variable between ISR and main loop to be declared "volatile" to avoid removal due to compiler optimizations)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Timer-0 ISR ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IRAM_ATTR Timer0_ISR()
{
  // use a critical section to protect shared variables
  portENTER_CRITICAL_ISR(&timerMux);
  // here safely read timer value or calculate counters, protected by a critical section
  Timer0_INT_counter++;        // Set flag/counter/semaphore to signal to the main loop that Timer0 has fired
  portEXIT_CRITICAL_ISR(&timerMux);
} // end of Timer0_ISR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//-------------------------------------------------------------------------
//   ****** End of hardware decalarations, ISR and software inits ******
//-------------------------------------------------------------------------



//#########################################################################
//    ****** main program init ******
//#########################################################################

// main setup
void setup(void)
{
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // *** MCU config ***

  // init serial port
#if debug_level >= 1
  Serial.begin(115200);
  while (!Serial) {
    // will pause Zero, Leonardo, etc until serial console opens
    vTaskDelay(1 / portTICK_PERIOD_MS); //delay in ms
  }
  Serial.println();
  Serial.println(F("Welcome @ ESP32 Pico-Kit v4 (115.200 baud)"));
  Serial.print(F("Solar-Power-Meter with INA219 - W.Franke 2020 - v"));
  Serial.println(SW_version, 2);
  Serial.println(F(" "));
#endif


  // Config Timer0 to generate 1s intervals by Timer-INT
  Timer0_INT_counter = 0;                             // reset flag/counter/semaphore of signal to main loop when the timer has fired
  timer0 = timerBegin(0, 80, true);                   // config Timer0 , with prescaler=80 (down to 1MHz) ,  true=count-up
  timerAttachInterrupt(timer0, &Timer0_ISR, true);    // attach Timer0_ISR , interrupt generated is of edge type
  timerAlarmWrite(timer0, 1000000, true);             // set timer counter to 1s , enable auto-reload counter
  timerAlarmEnable(timer0);                           // start Timer0


  // Config Wire I2C speed
  // The ESP32 has two I2C channels and any pin can be set as SDA or SCL.
  // When using the ESP32 with the Arduino IDE, the default I2C pins are: GPIO 21 (SDA) / GPIO 22 (SCL)
  Wire.begin(21, 22);
  Wire.setClock(800000);        // set I2C bus speed to Fast Mode 400kHz or 800kHz (I2C pull-up is 4,7k)


  // init ADC
  pinMode(ADC_pin_VCC, INPUT);          // the INPUT mode explicitly disables the internal pullups
  pinMode(ADC_pin_Vsolar, INPUT);       // the INPUT mode explicitly disables the internal pullups  
  analogReadResolution(12);             // Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
  analogSetAttenuation(ADC_11db);       // default (ADC_11db), for Attenuation=ADC_11db and Resolution=12 then U-Ref is 3v3 (3v3 = 4095 in range 0 to 4095)
  analogSetCycles(8);                   // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
  analogSetSamples(1);                  // Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
  analogSetClockDiv(1);                 // Set the divider for the ADC clock, default is 1, range is 1 - 255 (255 = 1338ms)

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // *** external hardware config ***

  // init OLED_display
  OLED.begin();           // will call: initDisplay(), clearDisplay(), setPowerSave(1)
  OLED.setPowerSave(0);   // disable power save mode to turn-on display (RAM is always on)


  // init INA219
  // search for an INA219 cartridge
  search_INA219_I2C_addr();                       // returns I2C address, or 0 in case of no INA219 cartridge found

  // config the INA219 cartridge
  if (cartridge_INA219_I2C_addr != 0)             // if a INA219 cartridge found, then config it
  {
    init_INA219_defaults_by_I2C_addr();           // search for I2C addr: 0x40=0,1Ω / 0x41=1Ω / 0x44=10Ω / 0x45=100Ω / 0x46=1kΩ / 0x4F=0,1Ω+52V
  }

  DebugPrint_to_Serial_Setup_INAdefaults();       // write all measures and results to Serial Port

  OLED_SplashScreen();          // prepare and show Splash Screen
  //now we use the time to display the SplashScreen to connect to the WiFi
  //vTaskDelay(7000/portTICK_PERIOD_MS); //delay in ms      // show splash screen for some seconds

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // *** Software config ***

  // Init Web-Server and set up required URL handlers on the web server
  ESP32WebServer.on("/", ESP32WebServer_handleRoot);                // handler for web requests to "/" path
  ESP32WebServer.on("/config", [] { iotWebConf.handleConfig(); });  // function provided by IotWebConf library?
  ESP32WebServer.onNotFound([]() {
    iotWebConf.handleNotFound();
  });  // function provided by IotWebConf library?

  // Init IotWebConf configuration (automatic WiFi Manager)
  pinMode(WiFi_IotWebConf_Button_Pin, INPUT_PULLUP);        // enable internal pull-up of WiFi_IotWebConf_Button_Pin, to keep HIGH without external switch-circuit
  iotWebConf.setStatusPin(WiFi_IotWebConf_Status_LED_Pin);  // init IotWebConf with Status_LED_Pin
  iotWebConf.setConfigPin(WiFi_IotWebConf_Button_Pin);      // init IotWebConf with Button_Pin
  iotWebConf.skipApStartup();                               // calling this method before the init will force IotWebConf to connect immediatelly to the configured WiFi (not opening the start-up AP)
  iotWebConf.init();                                        // start IotWebConf to configure WiFi automatically

  // show SplashScreen for 7s and try connect to WiFi by iotWebConf.doLoop (therefor it can not be used delay()/vTaskDelay() as they are blocking
  MilliSec_Start = millis();                                  // save millis-start of non-blocking delay
  while ((unsigned long)(millis() - MilliSec_Start) < 7000)   // wait for x ms (handles also roll-over at 4,294,967,295 ms correctly)
  {
    iotWebConf.doLoop();                // check and manage WiFi connection
  }

  // show WiFi connection status on OLED
  if (iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE)
  {
    OLED_WiFiScreen(0);                   // OLED message: WiFi connected to "SSID"
    // show OLED_WiFiScreen "connection success" for 4s and handle WiFi iotWebConf.doLoop (therefor it can not be used delay()/vTaskDelay() as they are blocking
    MilliSec_Start = millis();                                  // save millis-start of non-blocking delay
    while ((unsigned long)(millis() - MilliSec_Start) < 4000)   // wait for x ms (handles also roll-over at 4,294,967,295 ms correctly)
    {
      iotWebConf.doLoop();                // check and manage WiFi connection
    }
  }
  else
  {
    OLED_WiFiScreen(1);                   // OLED message: please connect WiFo to ESP32 Access Point
    // show OLED_WiFiScreen "please connect to AP" for 10s and handle WiFi iotWebConf.doLoop (therefor it can not be used delay()/vTaskDelay() as they are blocking
    MilliSec_Start = millis();                                  // save millis-start of non-blocking delay
    while ((unsigned long)(millis() - MilliSec_Start) < 10000)   // wait for x ms (handles also roll-over at 4,294,967,295 ms correctly)
    {
      iotWebConf.doLoop();                // check and manage WiFi connection
    }
    OLED_WiFiScreen(2);                   // OLED message: please connect WiFo to ESP32 Access Point
    // show OLED_WiFiScreen "don't forget t disconnect from AP after WiFi-config" for 10s and handle WiFi iotWebConf.doLoop (therefor it can not be used delay()/vTaskDelay() as they are blocking
    MilliSec_Start = millis();                                  // save millis-start of non-blocking delay
    while ((unsigned long)(millis() - MilliSec_Start) < 10000)   // wait for x ms (handles also roll-over at 4,294,967,295 ms correctly)
    {
      iotWebConf.doLoop();                // check and manage WiFi connection
    }
  }

  // init ThingSpeak IoT Cloud access
  ThingSpeak.begin(ESP32WebClient);     // Initialize ThingSpeak lib for IoT Cloud access


  //init NTP and get time from NTP Server
  configTime(0, 0, ntpServer[1], ntpServer[2], ntpServer[3]);     // NTP Server einstellen (max 3 Server)
  // configure automatic TZ and DST
  setenv("TZ", RTC_local_TZ_DST, 1);                              // Zeitzone einstellen https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  // receive time from NTP server and convert it to local time
  if (!getLocalTime(&RTC_time_components))                        // getLocalTime(): save NTP-time in tm-struct, http://www.cplusplus.com/reference/ctime/localtime/
  {
    NTP_time_valid = 0;
    Serial.println("");
    Serial.println("Failed to obtain time from NTP Server");
    Serial.println("");
  }
  else
  {
    NTP_time_valid = 1;
    Serial.println("");
  }

  // init variables
  Shunt_Current_daymax_mA = 0;          // reset Shunt_Current_daymax_mA
  Supply_Voltage_daymax_V = 0;          // reset Supply_Voltage_daymax_V
  Power_daymax_W = 0;                   // reset Power_daymax_W
  Power_Sum_Ws_today = 0;               // reset Energy counter (Integral of Watt over Time (Ws or Wh))
  Power_On_Seconds = 0;                 // reset counter of seconds since power-on
  Darkness_Counter_sec = 0;             // reset seconds of detected darkness
  Light_Counter_sec = 0;                // reset seconds of detected light to end a darkness period
  Darkness_waits_to_Reset_flag = 0;     // reset flag for darkness was detected and now waits t reset today's values
  NTP_msg_for_ThingSpeak_status_flag = 0;                  // reset flag if ThingSpeak status should be overwritten with NTP update status

  // reset counted values of last interval
  lastInterval_average_Shunt_Current_mA = 0;                            // reset last interval's average of solar Current (mA)
  lastInterval_average_Supply_Voltage_V = 0;                            // reset last interval's average of solar Voltage (V)
  lastInterval_maxPower_W = 0;                                          // reset last interval's peak solar power (W)

}//end of Arduino "Setup"
//-------------------------------------------------------------------------
//   ****** End setup ******
//-------------------------------------------------------------------------



//#########################################################################
//    ****** Arduino main program loop ******
//#########################################################################

//Arduino main loop
void loop(void)
{

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // *** run WiFi connection handling by IoTWebConf library (not blocking the main loop) ***
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  iotWebConf.doLoop();                    // check and manage WiFi connection, should be called as frequently as possible
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // *** wait for next second interval generated by Timer0-ISR ***
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // check if Timer0 has fired the 1s signal
  if (Timer0_INT_counter > 0)             // >0 means Timer0 has fired x times, counter would allow to process all/missing INTs
  {
    // increase power-on second counter
    Power_On_Seconds = Power_On_Seconds + 1;        // add 1 second to the power-on counter (to display as minutes running)

    portENTER_CRITICAL(&timerMux);
    // here safely read/copy the global variable set by the Timer0-ISR (protected by a critical section)
    Timer0_INT_counter = 0;             // reset Timer0-ISR counter
    portEXIT_CRITICAL(&timerMux);;

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // *** INA219 cartridge handling: ***
    //      -> read from it and calculate Solar Power values
    //      -> HotSwap: INA219 cartridge removed -> search for it to configure
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // test if INA219 cartridge is still present, read solar values or wait for HotSwap INA219 module
    HotSwap_test_INA219_I2C_addr();         // returns "cartridge_INA219_I2C_addr = 0" if cartdridge is removed
    if (cartridge_INA219_I2C_addr != 0)     // if same INA219 cartridge found, read from it and calculate Solar Power values
    {
      INA219_found_ReadCalc_solar_power();  // same INA219 cartridge found, read from it and calculate Solar Power values
    }
    else      // INA219 cartridge not found -> try HotSwap: search for cartridge and configure it
    {
      INA219_notfound_SearchConfigure_Module();  // try find a cartridge and configure it
    }
  }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // *** handle NTP synch and update time variable (tm-struct RTC_time_components.) with current time ***
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // update time variable (tm-struct) with current time
  NTP_time_UnixSec = time(&NTP_time_UnixSec);                         // get NTP/RTC time
  localtime_r(&NTP_time_UnixSec, &RTC_time_components);               // Convert Unix-Seconds to RTC_time_components as local time, http://www.cplusplus.com/reference/ctime/localtime/

  // on each new NTP/RTC second, start handling the NTP time triggered actions and NTP re-synch:
  if (RTC_time_components.tm_sec != RTC_time_last_sec)    // loop to wait for the next second to change (trigger is changing second)
  {
    RTC_time_last_sec = RTC_time_components.tm_sec;       // stores old second for trigger by changing second
#if debug_level >= 2
    // print the new time (implements a clock)
    Serial.print("current ESP32 RTC/NTP time: ");
    Serial.printf("%.2d:%.2d:%.2d (UTC)\n", NTP_time_UnixSec % 86400L / 3600, NTP_time_UnixSec % 3600 / 60, NTP_time_UnixSec % 60);  // Serielle Ausgabe Koordinierte Weltzeit
#endif

    // if NTP time available, then reset daily values at midnight (trigger at 23:59:55 -> will reset before 00:00:00 upload to cloud)
    if ((NTP_time_valid == 1) && (RTC_time_components.tm_hour == 23) && (RTC_time_components.tm_min == 59) && (RTC_time_components.tm_sec == 59))
    {
      Shunt_Current_daymax_mA = 0;                       // reset Shunt_Current_daymax_mA
      Supply_Voltage_daymax_V = 0;                       // reset Supply_Voltage_daymax_V
      Power_daymax_W = 0;                                // reset Power_daymax_W
      Power_Sum_Ws_today = 0;                            // reset Energy counter (Integral of Watt over Time (Ws or Wh))
      Power_Sum_Wh_yesterday = Power_Sum_Wh_today;       // save current Energy_Wh as yesterday's value
    }

    // daily time synch with NTP server at 04:00:00 a.m. (synch MCU time with NTP Server)
    if ((RTC_time_components.tm_hour == 04) && (RTC_time_components.tm_min == 00) && (RTC_time_components.tm_sec == 00))
    {
      Serial.println("start NTP synch:");
      time_t NTP_sync_diff_UnixSeconds = time(&NTP_time_UnixSec);     // store timestamp before NTP update

      //init and get time from NTP Server
      configTime(0, 0, ntpServer[1], ntpServer[2], ntpServer[3]);     // chose NTP Server (max 3 Server)
      // configure automatic TZ and DST
      setenv("TZ", RTC_local_TZ_DST, 1);              // time zone definitions https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
      // receive time from NTP server and convert it to local time
      if (!getLocalTime(&RTC_time_components))                    // getLocalTime(): save NTP-time in tm-struct, http://www.cplusplus.com/reference/ctime/localtime/
        // Bug: getLocalTime() never returns an Error here, even if configTime() before had wrong servers or WiFi=OFF
      {
        Serial.println("Failed to obtain time from NTP Server");
      }

      // wait for change on the second to let the new NTP time take effect
      while (time(&NTP_time_UnixSec) == NTP_sync_diff_UnixSeconds);       // wait for the next second to get new NTP time

      // check if NTP/RTC current time is valid
      if ((RTC_time_components.tm_year + 1900) < 2019)
      {
        NTP_time_valid = 0;
        // send NTP error Message to to ThingSpeak status
        strftime(string_buf22, sizeof(string_buf22), "%d.%b.%Y %T ", &RTC_time_components);             // http://www.cplusplus.com/reference/ctime/strftime/
        string_buf = String("NTP synch failed, continue the internal time: ") + string_buf22;
        Serial.println(string_buf);
        // this NTP synch status should be the next ThingSpeak status message
        ThingSpeak.setStatus(string_buf);
        NTP_msg_for_ThingSpeak_status_flag = 1;                        // ThingSpeak status should be overwritten with NTP update status
      }
      else
      {
        NTP_time_valid = 1;

        // send ThingSpeak status Message with NTP-Time, NTP synch status, and re-synched time difference
        NTP_time_UnixSec = time(&NTP_time_UnixSec);                                     // store new timestamp after NTP update
        NTP_sync_diff_UnixSeconds = NTP_time_UnixSec - NTP_sync_diff_UnixSeconds - 1;   // corrected difference of seconds between RTC and NTP

        strftime(string_buf22, sizeof(string_buf22), "%d.%b.%Y %T ", &RTC_time_components);             // http://www.cplusplus.com/reference/ctime/strftime/
        string_buf = String("NTP time synch done: ") + string_buf22;
        string_buf = string_buf + String(" -> time difference was: ");    // HTML <br> works for ThinkSpeak status
        string_buf = string_buf + (NTP_sync_diff_UnixSeconds);
        string_buf = string_buf + String(" sec");
        Serial.println(string_buf);
        // this NTP synch status should be the next ThingSpeak status message
        ThingSpeak.setStatus(string_buf);
        NTP_msg_for_ThingSpeak_status_flag = 1;                        // ThingSpeak status should be overwritten with NTP update status
      }
    }
  }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // *** wait for Cloud-upload interval end, to send IoT-data as multiple fields to ThingSpeak IoT Cloud ***
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // after every interval of x seconds, send data of last interval to the ThingSpeak channel:
  if (Power_On_Seconds - ThingSpeak_upload_Interval_StartSec >= ThingSpeak_upload_Interval_seconds)     // if interval lenght is reached or over
  {
    //reset interval counter
    ThingSpeak_upload_Interval_StartSec = Power_On_Seconds;                 // save start second of running interval

    lastInterval_average_Shunt_Current_mA = lastInterval_average_Shunt_Current_mA / ThingSpeak_upload_Interval_seconds; // average last interval's average of solar Current (mA)
    lastInterval_average_Supply_Voltage_V = lastInterval_average_Supply_Voltage_V / ThingSpeak_upload_Interval_seconds; // average last interval's average of solar Voltage (V)
    // lastInterval_maxPower_W = value is calculated every second...                               // last interval's peak solar power (W)

    // fill the channel fields with the values, ThingSpeak Free License field numbers can be 1..4 (not 8) within a channel (int, long, float, string, const char *)
    ThingSpeak.setField(1, lastInterval_average_Shunt_Current_mA);          // last interval's average of solar Current (mA)
    ThingSpeak.setField(2, lastInterval_average_Supply_Voltage_V);          // last interval's average of solar Voltage (V)
    ThingSpeak.setField(3, lastInterval_maxPower_W);                        // last interval's peak solar power (W)
    ThingSpeak.setField(4, Power_Sum_Wh_today);                             // current day's real counted Wh (Wh)


    // set the ThingSpeak status based on ADC7 (ADC_CorrectedVoltage) OR on INA219 (lastInterval_average_Shunt_Current_mA)
    // the status will be visible in the ThingSpeak Status Widget, or can be used e.g. by the ThingTweet App or Twitter integration
    // read ADC-7 (for solar voltage)
    ADC_CorrectedVoltage = analogRead(ADC_pin_Vsolar);       // with defaults (Attenuation=ADC_11db and Resolution=12) the U-Ref is 3v3 (3v3 = 4095 in range 0 to 4095)  
    // ESP32 ADC-correction -> source: github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
    if((ADC_CorrectedVoltage > 0) && (ADC_CorrectedVoltage <= 4095))
    {                                                         // ADC non-linear error corrected by simple binomial function
      ADC_CorrectedVoltage = (-0.000000000000016 * pow(ADC_CorrectedVoltage,4)) + (0.000000000118171 * pow(ADC_CorrectedVoltage,3)) - (0.000000301211691 * pow(ADC_CorrectedVoltage,2)) + (0.001109019271794 * ADC_CorrectedVoltage) + 0.034143524634089;
      ADC_CorrectedVoltage = ADC_CorrectedVoltage * 11;       // Source Voltage is measured by Vsolar/11: ADC_pin_Vsolar connected by Voltage devider 100k+10k 
      ADC_CorrectedVoltage = ADC_CorrectedVoltage - 0.2;      // Manually correct Source Voltage ADC by my ESP32, against real measure around MPP=16V
    }

    // set the ThingSpeak status:  
        // - based on ADC7 (ADC_CorrectedVoltage) and/or on INA219 (lastInterval_average_Shunt_Current_mA),
        // - in special case of daily NTP re-synch, the NTP message is used and the normal status on ADC7/INA219 is skipped,
    if (NTP_msg_for_ThingSpeak_status_flag == 1)              // skip normal message generation and use the already set NTP message as ThingSpeak status
    {
      NTP_msg_for_ThingSpeak_status_flag = 0;
    }
    else                                                      // use the Solar Voltage or INA219 Solar Current for the ThingSpeak status
    {
      normal_ThingSpeak_Status_Msg();         // returning: ThingSpeak.setStatus("myStatus") with myStatus a zero terminated String (UTF8) <255 bytes
                                              // this is a special implementation for the Solar Cells I use, 
                                              // the Subroutine needs to be adopted for other Solar Cells installations
    }


    // post data to ThingSpeak cloud only if WiFi is connected
    if (iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE)
    {
#if debug_level >= 2
      Serial.println(F("ThingSpeak interval is over ... sending data of last interval to ThingSpeak "));
      Serial.print(F("Field-1 is average of solar Current (mA):  "));  Serial.println(lastInterval_average_Shunt_Current_mA, 5);
      Serial.print(F("Field-2 is average of solar Voltage (V):   "));  Serial.println(lastInterval_average_Supply_Voltage_V, 5);
      Serial.print(F("Field-3 is peak solar power (W):           "));  Serial.println(lastInterval_maxPower_W, 5);
      Serial.print(F("Field-4 is today's counted Energy (Wh):    "));  Serial.println(Power_Sum_Wh_today, 5);
      Serial.print(F("Status based on Solar Voltage (V):    "));  Serial.println(ADC_CorrectedVoltage, 2);
#endif
    }

    // write fields and status to the ThingSpeak channel
    int HTTP_Get_Response = ThingSpeak.writeFields(my_ThingSpeak_ChannelNumber, my_ThingSpeak_WriteAPIKey);
    if (HTTP_Get_Response == 200)         // ThingSpeak.writeFields() returns HTTP status code of 200 if successful
    {
      Serial.println("ThingSpeak Cloud channel update was successful.");
      Serial.println();
    }
    else
    {
      Serial.println();
      Serial.println("Problem updating ThingSpeak channel: HTTP error code " + String(HTTP_Get_Response));
      Serial.println();
      Serial.println();
    }

    // reset counted values of last interval to start new interval value counting
    lastInterval_average_Shunt_Current_mA = 0;                            // reset last interval's average of solar Current (mA)
    lastInterval_average_Supply_Voltage_V = 0;                            // reset last interval's average of solar Voltage (V)
    lastInterval_maxPower_W = 0;                                          // reset last interval's peak solar power (W)
  }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}// end of Arduino "Main Loop"
//------------------------------------------------------------------------
//   ****** End main program loop ******
//-------------------------------------------------------------------------




//#########################################################################
//   ****** Subroutines ; ISR ; Handlers ******
//#########################################################################

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// // *** Handler: handle web requests to "/" path ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ESP32WebServer_handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 02 Status and Reset</title></head><body>Hello world!";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  ESP32WebServer.send(200, "text/html", s);
} // end of ESP32WebServer_handleRoot
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// // *** Subroutine: INA219_found_ReadCalc_solar_power ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void INA219_found_ReadCalc_solar_power(void)   // INA219 cartridge found -> read from it and calculate Solar Power values
{
  // read Shunt voltage from INA219 and make sure the most efficient PGA/GainDiv is used
  Shunt_Voltage_uV = INA219_I2C_Read_ShuntVoltage();          // in µV measured over Shunt by INA219, by most efficient PGA/GainDiv
  Shunt_Current_mA = (Shunt_Voltage_uV / cartridge_R_SHUNT_OHM) / 1000;  // from µV to mV (float)I = (long)U / (float)R

  // read Bus voltage from INA219 -> FYI: if no Load is conncted at Vin- to GND, INA219 returns 1V of Bus Voltage
  Bus_Voltage_V = INA219_I2C_Read_BusVoltage();               // measured behind Shunt (at Vin-) by INA219
  if (cartridge_Voltage_Devider == 2)
  {
    Bus_Voltage_V = 2 * Bus_Voltage_V;                // double measured Bus_Voltage if INA219 cartridge has voltage devider (e.g. I2C add 0x4F)
  }

  // calc Solar Power (Watt)
  Supply_Voltage_V = Bus_Voltage_V + (Shunt_Voltage_uV / 1000000);  // calculate Solar voltage
  Power_W = Supply_Voltage_V * (Shunt_Current_mA / 1000);           // P=U*I

  // check for max daily values of Shunt-Current and Solar-Voltage and Solar-Power
  if (Shunt_Current_mA > Shunt_Current_daymax_mA)  {
    Shunt_Current_daymax_mA = Shunt_Current_mA;  // set max of daily peak solar Current
  }
  if (Supply_Voltage_V > Supply_Voltage_daymax_V)  {
    Supply_Voltage_daymax_V = Supply_Voltage_V;  // set max of daily peak solar Voltage
  }
  if (Power_W > Power_daymax_W)  {
    Power_daymax_W = Power_W;  // set max of daily peak solar power (W)
  }

  // daily accumulated Solar-Energy since last darkness (Power_W = averaged Energy of last second in Ws)
  Power_Sum_Ws_today = Power_Sum_Ws_today + Power_W;    // add the last second's Ws to the Energy Counter (= Integral of Watt over Time -> in Ws)
  Power_Sum_Wh_today = Power_Sum_Ws_today / 3600;       // Energy Counter in Wh (since power-on)

  // calc values to be sent to ThingSpeak cloud
  if (Power_W > lastInterval_maxPower_W)  {
    lastInterval_maxPower_W = Power_W;
  };                          // set max of last ThingSpeak interval's peak solar power (W)
  lastInterval_average_Shunt_Current_mA = lastInterval_average_Shunt_Current_mA + Shunt_Current_mA;        // sum up for average of last interval's average of solar Current (mA)
  lastInterval_average_Supply_Voltage_V = lastInterval_average_Supply_Voltage_V + Supply_Voltage_V;        // sum up for average of last interval's average of solar Voltage (V)


  // auto-reset daily values in the middle of the night
  // if darkness for >x hours, then reset sum and max of last day (Shunt_Current_daymax_mA, Supply_Voltage_daymax_V, Power_daymax_W, Power_Sum_Ws_today)
  if (Bus_Voltage_V < Darkness_Solar_Volt)            // detect Darkness by low Solar Voltage
  {
    Darkness_Counter_sec = Darkness_Counter_sec + 1;  // increase seconds of detected darkness
    Darkness_waits_to_Reset_flag = 1;                 // darkness was detected and now waits t reset today's values
    Light_Counter_sec = 0;                            // reset seconds of detected light to end a darkness period
    //reset day counters at midnight time when NTP available, or after x hours darkness when is NTP not available
    if ((NTP_time_valid == 0) && Darkness_Counter_sec == (Darkness_Hours_to_Reset * 3600))   // if darkness for =x hours then init today-values with zero (Darkness_Counter_sec keeps counting a long var)
    {
      Shunt_Current_daymax_mA = 0;                       // reset Shunt_Current_daymax_mA
      Supply_Voltage_daymax_V = 0;                       // reset Supply_Voltage_daymax_V
      Power_daymax_W = 0;                                // reset Power_daymax_W
      Power_Sum_Ws_today = 0;                         // reset Energy counter (Integral of Watt over Time (Ws or Wh))
      Power_Sum_Wh_yesterday = Power_Sum_Wh_today;    // save current Energy_Wh as yesterday's value
    }
  }
  else                                                // minimum of Brightness detected
  {
    if (Darkness_waits_to_Reset_flag == 1)            // prevent at night to interrupt the darkness by short light rays (while waiting to Reset today's values)
    {
      Light_Counter_sec = Light_Counter_sec + 1;      // increase seconds of detected light to end a darkness period
      if (Light_Counter_sec >= Light_Sec_to_wakeup)   // if light is >x sec then end darkness
      {
        Darkness_Counter_sec = 0;                     // reset seconds of detected darkness, after a certain time of light was detected
        Darkness_waits_to_Reset_flag = 0;             // rseet darkness was detected and now waits t reset today's values
      }
    }
  }  // end: check if darkness for >x hours

  DebugPrint_to_Serial_MainLoop_INA_inserted();          // write all measures and results to Serial Port

  // write measures and results on OLED screen
  // indicate if Shunt_Current_mA >= Imax for this Shunt @ 320mV (I=U/R)
  // use dynamic digits: cartridges 0x40+41+44+4F show >= 10mA y(mA) and <10mA y.x(mA) ; the 0x45=3mA always shows y.xx(mA) ; the 0x46=0,3mA always shows yyy(µA)
  OLED.clearBuffer();           // "F" is full buffer mode: very fast (but needs big memory)
  // Draw a line
  OLED.drawLine(0, 41, 127, 41);
  // Print text
  OLED.setFont(u8g2_font_lucasfont_alternate_tf);  // 7 Pixel Height

  // calculate number of decimal places to shown on OLED for Shunt_Current_mA
  // set decimal places for cartridges of Imax 3,2A and 320mA and 32mA
  OLEDprint_decimalplaces_shunt_mA = 0;       // assume no decimal place, required for 0x40+41+44+4F at >=10mA
  if (Shunt_Current_mA < 10)
  {
    OLEDprint_decimalplaces_shunt_mA = 1;     // set 1 decimal place, required for 0x40+41+44+4F at <10mA
  }
  if (Shunt_Current_mA < 1)
  {
    OLEDprint_decimalplaces_shunt_mA = 3;     // set 1 decimal place, required for 0x40+41+44+4F at <10mA
  }

  // set decimal places for cartridges of Imax 3mA and 0,3mA
  switch (cartridge_INA219_I2C_addr)          // check I2C address of used INA2019
  {
    case 0x45:
      OLEDprint_decimalplaces_shunt_mA = 3;   // set 2 decimal place, for 0x45 (3mA)
      break;  // end case
    case 0x46:
      OLEDprint_decimalplaces_shunt_mA = 3; // set 3 decimal place, for 0x46 (0,3mA)
      break;  // end case
    default:
      break;  // end case
  }

  // indicate Shunt_Current_mA >= Imax for this Shunt (@ 320mV, by I=U/R) by >yyy.xx< if
  if (Shunt_Current_mA < (actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM))  // Shunt_Current_mA < Imax for this Shunt and GainDiv (I=U/R)
  {
    OLED.setCursor(0, 7);    OLED.print(F("Current:        "));  OLED.print(Shunt_Current_mA, OLEDprint_decimalplaces_shunt_mA);  OLED.println(F(" mA"));
  }
  else    // indicate that Shunt_Current_mA >= Imax for this Shunt and GainDiv (I=U/R)
  {
    OLED.setCursor(0, 7);    OLED.print(F("Current:        >"));  OLED.print(Shunt_Current_mA, OLEDprint_decimalplaces_shunt_mA);  OLED.println(F(" mA<"));
  }

  // set decimal places for Supply_Voltage_V <1V
  if (Supply_Voltage_V >= 1)
  {
    OLED.setCursor(0, 17);   OLED.print(F("Solar-Volt:      "));  OLED.print(Supply_Voltage_V, 1);  OLED.println(F(" V"));
  }
  else
  {
    OLED.setCursor(0, 17);   OLED.print(F("Solar-Volt:      "));  OLED.print(Supply_Voltage_V, 2);  OLED.println(F(" V"));
  }

  OLED.setCursor(0, 28);   OLED.print(F("Day max:  "));  OLED.print(Shunt_Current_daymax_mA, 0);  OLED.print(F("mA / "));
  OLED.print(Supply_Voltage_daymax_V, 1); OLED.println(F("V"));
  OLED.setCursor(0, 38);   OLED.print(F("P-now/max:  "));  OLED.print(Power_W, 2);  OLED.print(F("/"));
  OLED.print(Power_daymax_W, 2); OLED.println(F("W"));
  OLED.setCursor(0, 52);   OLED.print(F("Energy today:  "));
  OLED.setCursor(83, 52);  OLED.print(Power_Sum_Wh_today, 2);  OLED.println(F(" Wh"));
  OLED.setCursor(0, 63);   OLED.print(F("Energy day-1:  "));
  OLED.setCursor(83, 63);  OLED.print(Power_Sum_Wh_yesterday, 2);  OLED.println(F(" Wh"));

  OLED_move_heartbeat_pixel();   // move the heartbeat pixel animation on OLED

  OLED.sendBuffer();            // use full buffer mode

}//end of sub "INA219_found_ReadCalc_solar_power"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: INA219_notfound_SearchConfigure_Module ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void INA219_notfound_SearchConfigure_Module(void)     // INA219 cartridge not found -> try HotSwap: search for cartridge and configure it
{
  // search for an INA219 cartridge
  search_INA219_I2C_addr();               // returns I2C address, or 0 in case of no INA219 cartridge found

  // config the INA219 cartridge
  if (cartridge_INA219_I2C_addr != 0)     // if a INA219 cartridge found, then config it
  {
    init_INA219_defaults_by_I2C_addr();          // search for I2C addr: 0x40=0,1Ω / 0x41=1Ω / 0x44=10Ω / 0x45=100Ω / 0x46=1kΩ / 0x4F=0,1Ω+52V
  }

  DebugPrint_to_Serial_MainLoop_INA_missing();  // write all measures and results to Serial Port

  // write message to OLED if INA219 found or not
  if (cartridge_INA219_I2C_addr != 0)       // a new INA219 cartridge was found
  {
    // write a Found-message and selected results on OLED screen
    OLED.clearBuffer();           // "F" is full buffer mode: very fast (but needs big memory)
    // Draw a line
    OLED.drawLine(0, 41, 127, 41);
    // Print text
    OLED.setFont(u8g2_font_lucasfont_alternate_tf);  // 7 Pixel Height
    OLED.setCursor(0, 7);    OLED.print(F("New INA219 found..."));
    OLED.setCursor(0, 17);   OLED.print(F("I2C addr:     0x")); OLED.println(cartridge_INA219_I2C_addr, HEX);

    OLED.setCursor(0, 27);   OLED.print(F("Shunt:           "));
    if (cartridge_R_SHUNT_OHM < 1)            // print format x.y Ohm for <1 Ohm
    {
      OLED.print(cartridge_R_SHUNT_OHM, 1);
    }
    else                                      // print format x Ohm for >=1 Ohm
    {
      OLED.print(cartridge_R_SHUNT_OHM, 0);
    }
    OLED.println(F(" Ohm"));

    OLED.setCursor(0, 37);   OLED.print(F("I-Shunt max:   "));
    if (cartridge_R_SHUNT_OHM < 1000)         // print format x mA for Shunt <1kOhm
    {
      OLED.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 0);
    }
    else                                      // print format x.y mA for Shunt >=1kOhm
    {
      OLED.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 1);
    }
    OLED.println(F(" mA"));

    OLED.setCursor(0, 52);   OLED.print(F("Energy today:  "));
    OLED.setCursor(83, 52);  OLED.print(Power_Sum_Wh_today, 2);  OLED.println(F(" Wh"));
    OLED.setCursor(0, 63);   OLED.print(F("Energy day-1:  "));
    OLED.setCursor(83, 63);  OLED.print(Power_Sum_Wh_yesterday, 2);  OLED.println(F(" Wh"));
    // write to OLED
    OLED.sendBuffer();            // use full buffer mode

    vTaskDelay(7000 / portTICK_PERIOD_MS); //delay in ms      // display new INA found
  }
  else                              // INA219 cartridge is still removed
  {
    // write a Removed-message and selected results on OLED screen
    OLED.clearBuffer();           // "F" is full buffer mode: very fast (but needs big of memory)
    for (byte i = 5; ((i <= 5) && (i >= 0)); i--)
    {
      // Draw a line
      OLED.drawLine(0, 41, 127, 41);
      // Print text
      OLED.setFont(u8g2_font_lucasfont_alternate_tf);  // 7 Pixel Height
      OLED.setCursor(0, 7);    OLED.print(F("! ! !  INA219  removed  ! ! ! "));
      OLED.setCursor(0, 17);   OLED.print(F("       Searching:  ")); OLED.print(i);
      OLED.setCursor(0, 28);   OLED.print(F("Day max:  "));  OLED.print(Shunt_Current_daymax_mA, 0);  OLED.print(F("mA / "));
      OLED.print(Supply_Voltage_daymax_V, 1); OLED.println(F("V"));
      OLED.setCursor(0, 38);   OLED.print(F("P-now/max:  "));  OLED.print(0, 2);  OLED.print(F("/"));
      OLED.print(Power_daymax_W, 2); OLED.println(F("W"));
      OLED.setCursor(0, 52);   OLED.print(F("Energy today:  "));
      OLED.setCursor(83, 52);  OLED.print(Power_Sum_Wh_today, 2);  OLED.println(F(" Wh"));
      OLED.setCursor(0, 63);   OLED.print(F("Energy day-1:  "));
      OLED.setCursor(83, 63);  OLED.print(Power_Sum_Wh_yesterday, 2);  OLED.println(F(" Wh"));
      // write to OLED
      OLED.sendBuffer();            // use full buffer mode

      if (i > 0) {
        vTaskDelay(900 / portTICK_PERIOD_MS); // delay for countdown to 0, then search again for INA219 cartridge
      } else {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }
    }
  } //end: write message to OLED if INA219 found or not
}//end of sub "INA219_notfound_SearchConfigure_Module"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: SplashScreen ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OLED_SplashScreen(void)
{ // OLED.setCursor(0,10) points to the lower-left pixel if the first character
  OLED.clearDisplay();
  OLED.clearBuffer();           // "F" is full buffer mode: very fast (but needs big memory)
  OLED.drawPixel(1, 1);
  OLED.drawLine(0, 24, 127, 24);
  OLED.drawFrame(0, 0, 127, 64);
  //OLED.setFont(u8g2_font_t0_11_mf);  // 8 Pixel Height + fixed width :  u8g2_font_t0_11_mf
  OLED.setFont(u8g2_font_profont11_mf);  // 7 Pixel Height :  u8g2_font_profont11_mf / u8g2_font_lucasfont_alternate_tf / u8g2_font_mozart_nbp_tf / u8g2_font_haxrcorp4089_tr

  OLED.setCursor(3, 10);   OLED.print(F("Power-Meter INA219"));
  OLED.setCursor(3, 20);   OLED.print(F("W.Franke 2019, v")); OLED.print(SW_version, 2);
  if (cartridge_INA219_I2C_addr != 0)
  {
    OLED.setCursor(3, 36);   OLED.print(F("INA219 Config: 0x")); OLED.println(cartridge_INA219_I2C_addr, HEX);

    OLED.setCursor(3, 47);   OLED.print(F("Shunt:   "));
    if (cartridge_R_SHUNT_OHM < 1)            // print format x.y Ohm for <1 Ohm
    {
      OLED.print(cartridge_R_SHUNT_OHM, 1);
    }
    else                                      // print format x Ohm for >=1 Ohm
    {
      OLED.print(cartridge_R_SHUNT_OHM, 0);
    }
    OLED.println(F(" Ohm"));

    OLED.setCursor(3, 58);   OLED.print(F("I-Shunt max: "));
    if (cartridge_R_SHUNT_OHM < 1000)         // print format x mA for Shunt <1kOhm
    {
      OLED.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 0);
    }
    else                                      // print format x.y mA for Shunt >=1kOhm
    {
      OLED.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 1);
    }
    OLED.println(F(" mA"));
  }
  else
  {
    OLED.setCursor(3, 37);   OLED.println(F("INA219 Config:"));
    OLED.setCursor(3, 47);   OLED.println(F("INA219 not found !!!"));
    int V_USB = map(analogRead(ADC_pin_VCC), 0, 4095, 0, 6970);  // (6970 is measured correction) accurate would be double the 3,3V reference due to Voltage devider 10k+10k
    OLED.setCursor(3, 57);   OLED.print(F("V-USB: "));   OLED.print(V_USB);   OLED.println(F(" mV"));
  }
  OLED.sendBuffer();            // use full buffer mode

}//end of sub "SplashScreen"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: OLED_WiFiScreen ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OLED_WiFiScreen(int x)
{
  if (x == 0)
  {
    OLED.clearDisplay();
    OLED.clearBuffer();
    OLED.drawPixel(1, 1);
    OLED.drawFrame(0, 0, 127, 64);
    OLED.setFont(u8g2_font_t0_11_mf);       // 8 Pixel Height + fixed width :  u8g2_font_t0_11_mf
    //OLED.setFont(u8g2_font_profont11_mf);  // 7 Pixel Height :  u8g2_font_profont11_mf / u8g2_font_lucasfont_alternate_tf / u8g2_font_mozart_nbp_tf / u8g2_font_haxrcorp4089_tr

    OLED.setCursor(3, 13);   OLED.print(F("ESP32 connected to"));
    OLED.setCursor(3, 26);   OLED.print(F("your WiFi SSID: "));
    OLED.setCursor(3, 46);   OLED.print(WiFi.SSID());               // show the SSID of the WiFi currently connected to
  }

  if (x == 1)
  {
    OLED.clearDisplay();
    OLED.clearBuffer();
    OLED.drawPixel(1, 1);
    OLED.drawFrame(0, 0, 127, 64);
    OLED.setFont(u8g2_font_t0_11_mf);       // 8 Pixel Height + fixed width :  u8g2_font_t0_11_mf
    //OLED.setFont(u8g2_font_profont11_mf);  // 7 Pixel Height :  u8g2_font_profont11_mf / u8g2_font_lucasfont_alternate_tf / u8g2_font_mozart_nbp_tf / u8g2_font_haxrcorp4089_tr

    OLED.setCursor(3, 19);   OLED.print(F("Connect your WiFi to"));
    OLED.setCursor(3, 32);   OLED.print(F("ESP32 Access Point: "));
    OLED.setCursor(3, 52);   OLED.print(iotWebConf.getThingName()); // show the current name of the ESP32 which is also it's WiFi AccessPoint name
  }

  if (x == 2)
  {
    OLED.clearDisplay();
    OLED.clearBuffer();
    OLED.drawPixel(1, 1);
    OLED.drawFrame(0, 0, 127, 64);
    OLED.setFont(u8g2_font_t0_11_mf);       // 8 Pixel Height + fixed width :  u8g2_font_t0_11_mf
    //OLED.setFont(u8g2_font_profont11_mf);  // 7 Pixel Height :  u8g2_font_profont11_mf / u8g2_font_lucasfont_alternate_tf / u8g2_font_mozart_nbp_tf / u8g2_font_haxrcorp4089_tr

    OLED.setCursor(3, 13);   OLED.print(F("when config is saved"));
    OLED.setCursor(3, 26);   OLED.print(F("then disconnect from"));
    OLED.setCursor(3, 39);   OLED.print(F("ESP32 AccessPoint:"));
    OLED.setCursor(3, 55);   OLED.print(iotWebConf.getThingName()); // show the current name of the ESP32 which is also it's WiFi AccessPoint name
  }

  OLED.sendBuffer();            // use full buffer mode

}//end of sub "OLED_WiFiScreen"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: DebugPrint_to_Serial_Setup_INAdefaults ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebugPrint_to_Serial_Setup_INAdefaults(void)   // DebugPrint_to_Serial_Setup_INAdefaults
{
#if debug_level >= 1
  if (cartridge_INA219_I2C_addr != 0)
  {
    Serial.print(F("INA219 found at I2C address: 0x"));
    Serial.println(cartridge_INA219_I2C_addr, HEX);
    Serial.println();
    Serial.println(F("init INA219 done with config:"));
    Serial.print(F("Shunt: ")); Serial.print(cartridge_R_SHUNT_OHM, 1); Serial.println(F(" Ohm"));
    Serial.print(F("V-Shunt max: ")); Serial.print(actual_V_SHUNT_GainRange_mV); Serial.println(F(" mV"));
    Serial.print(F("I-Shunt max: ")); Serial.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 0); Serial.println(F(" mA"));
    Serial.println();
  }
  else
  {
    Serial.println("ATTENTION: INA219 not found");
    vTaskDelay(100 / portTICK_PERIOD_MS); //delay in ms
    int V_USB = map(analogRead(ADC_pin_VCC), 0, 4095, 0, 6970);  // (6970 is measured correction) accurate would be double the 3,3V reference due to Voltage devider 10k+10k
    Serial.print(F("V-USB (5V): "));
    Serial.println(V_USB);
    Serial.println();
  }
#endif
}//end of sub "DebugPrint_to_Serial_Setup_INAdefaults"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: DebugPrint_to_Serial_MainLoop_INA_inserted ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebugPrint_to_Serial_MainLoop_INA_inserted(void)   // DebugPrint_to_Serial_MainLoop_INA_inserted
{
#if debug_level >= 2
  // write all measures and results to Serial Port
  Serial.print(F("Time since Power-On:    ")); Serial.print(Power_On_Seconds / 60); Serial.println(F(" min"));
  Serial.print(F("V-Shunt max:            ")); Serial.print(actual_V_SHUNT_GainRange_mV); Serial.println(F(" mV"));
  Serial.print(F("measured Shunt Voltage: ")); Serial.print(Shunt_Voltage_uV);  Serial.println(F(" µV"));
  Serial.print(F("calculated Current:     ")); Serial.print(Shunt_Current_mA, 3);  Serial.println(F(" mA"));
  Serial.print(F("measured Bus-Voltage:   ")); Serial.print(Bus_Voltage_V);     Serial.println(F(" V"));
  Serial.print(F("calculated Solar-Volt:  ")); Serial.print(Supply_Voltage_V);  Serial.println(F(" V"));
  Serial.print(F("calculated Solar-Power: ")); Serial.print(Power_W, 3);        Serial.println(F(" W"));
  Serial.print(F("Solar-Current max:      ")); Serial.print(Shunt_Current_daymax_mA, 0);  Serial.println(F(" mA"));
  Serial.print(F("Solar-Volt max:         ")); Serial.print(Supply_Voltage_daymax_V, 2);  Serial.println(F(" V"));
  Serial.print(F("Solar-Power max:        ")); Serial.print(Power_daymax_W, 3);     Serial.println(F(" W"));
  Serial.print(F("Solar-Energy today:     ")); Serial.print(Power_Sum_Wh_today, 3); Serial.println(F(" Wh"));
  Serial.println("");
#endif
}//end of sub "DebugPrint_to_Serial_MainLoop_INA_inserted"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: DebugPrint_to_Serial_MainLoop_INA_missing ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebugPrint_to_Serial_MainLoop_INA_missing(void)   // DebugPrint_to_Serial_MainLoop_INA_missing
{
#if debug_level >= 1
  if (cartridge_INA219_I2C_addr != 0)     // a new INA219 cartridge was found
  {
    Serial.print(F("INA219 found at I2C address: 0x"));
    Serial.println(cartridge_INA219_I2C_addr, HEX);
    Serial.println();
    Serial.println(F("init INA219 done with config:"));
    Serial.print(F("Shunt: ")); Serial.print(cartridge_R_SHUNT_OHM, 1); Serial.println(F(" Ohm"));
    Serial.print(F("I-Shunt max: ")); Serial.print(actual_V_SHUNT_GainRange_mV / cartridge_R_SHUNT_OHM, 0); Serial.println(F(" mA"));
    Serial.println();
  }
  else                                    // INA219 cartridge is still removed
  {
    Serial.println("ATTENTION: INA219 is removed");
    vTaskDelay(100 / portTICK_PERIOD_MS); //delay in ms
    int V_USB = map(analogRead(ADC_pin_VCC), 0, 4095, 0, 6970);  // (6970 is measured correction) accurate would be double the 3,3V reference due to Voltage devider 10k+10k
    Serial.print(F("V-USB (5V): "));
    Serial.print(V_USB);
    Serial.println(F(" mV"));
    Serial.println();
  }
#endif
}//end of sub "DebugPrint_to_Serial_MainLoop_INA_missing"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: Move Heartbeat Pixel on OLED ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OLED_move_heartbeat_pixel(void)   // move the heartbeat pixel animation on OLED
{
  switch (heartbeat_pixel)        // show heartbeat by pixel animation
  {
    case 0:
      OLED.drawPixel(55, 4); OLED.setDrawColor(0); OLED.drawPixel(58, 4); OLED.drawPixel(61, 4); OLED.drawPixel(64, 4); heartbeat_pixel = 1; OLED.setDrawColor(1);
      OLED.drawPixel(55, 3); OLED.setDrawColor(0); OLED.drawPixel(58, 3); OLED.drawPixel(61, 3); OLED.drawPixel(64, 3); heartbeat_pixel = 1; OLED.setDrawColor(1);
      break;  // end case
    case 1:
      OLED.setDrawColor(0); OLED.drawPixel(55, 4); OLED.setDrawColor(1); OLED.drawPixel(58, 4); OLED.setDrawColor(0); OLED.drawPixel(61, 4); OLED.drawPixel(64, 4); heartbeat_pixel = 2; OLED.setDrawColor(1);
      OLED.setDrawColor(0); OLED.drawPixel(55, 3); OLED.setDrawColor(1); OLED.drawPixel(58, 3); OLED.setDrawColor(0); OLED.drawPixel(61, 3); OLED.drawPixel(64, 3); heartbeat_pixel = 2; OLED.setDrawColor(1);
      break;  // end case
    case 2:
      OLED.setDrawColor(0); OLED.drawPixel(55, 4); OLED.drawPixel(58, 4); OLED.setDrawColor(1); OLED.drawPixel(61, 4); OLED.setDrawColor(0); OLED.drawPixel(64, 4); heartbeat_pixel = 3; OLED.setDrawColor(1);
      OLED.setDrawColor(0); OLED.drawPixel(55, 3); OLED.drawPixel(58, 3); OLED.setDrawColor(1); OLED.drawPixel(61, 3); OLED.setDrawColor(0); OLED.drawPixel(64, 3); heartbeat_pixel = 3; OLED.setDrawColor(1);
      break;  // end case
    case 3:
      OLED.setDrawColor(0); OLED.drawPixel(55, 4); OLED.drawPixel(58, 4); OLED.drawPixel(61, 4); OLED.setDrawColor(1); OLED.drawPixel(64, 4); heartbeat_pixel = 0;
      OLED.setDrawColor(0); OLED.drawPixel(55, 3); OLED.drawPixel(58, 3); OLED.drawPixel(61, 3); OLED.setDrawColor(1); OLED.drawPixel(64, 3); heartbeat_pixel = 0;
      break;  // end case
  }
}//end of sub "OLED_move_heartbeat_pixel"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: find INA2019 cartridge by I2C address ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void search_INA219_I2C_addr(void)   // search for an INA219 cartridge by scanning possible I2C adresses (I2C addr: 0x40=0,1Ω / 0x41=1Ω / 0x44=10Ω / 0x45=100Ω / 0x46=1kΩ / 0x4F=0,1Ω+52V)
{
  cartridge_INA219_I2C_addr = 0;
  for (byte i = 0; i < (sizeof(myINA219_I2C_addr) / sizeof(myINA219_I2C_addr[0])); i++) // loop through array, works correctly with an array of any type or size
  {
    Wire.beginTransmission (myINA219_I2C_addr[i]);      // start I2C communiaction with slave
    if (Wire.endTransmission() == 0)                    // if slaved answered with no error
    {
      cartridge_INA219_I2C_addr = myINA219_I2C_addr[i];   // remember last found address in case of >1 INA219 found,
    }
  }
  vTaskDelay(10 / portTICK_PERIOD_MS); //delay in ms
  // returns last found I2C address in gloabl variable "cartridge_INA219_I2C_addr"
}//end of sub "search_INA219_I2C_addr"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: config INA2019 cartridge by I2C address ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void init_INA219_defaults_by_I2C_addr(void)    // config the INA219 cartridge by using I2C adresses
{
  uint16_t INA219_config;               // stores the bit settings for INA219 config register

  // set system variable for INA219 cartridge by I2C address
  switch (cartridge_INA219_I2C_addr)    // load config data for cartridge found
  {
    case 0x40:
      cartridge_R_SHUNT_OHM = 0.1;      // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 1;    // no devider, Solar Voltage up to 26V
      break;  // end case
    case 0x41:
      cartridge_R_SHUNT_OHM = 1.0;      // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 1;    // no devider, Solar Voltage up to 26V
      break;  // end case
    case 0x44:
      cartridge_R_SHUNT_OHM = 10;       // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 1;    // no devider, Solar Voltage up to 26V
      break;  // end case
    case 0x45:
      cartridge_R_SHUNT_OHM = 100;      // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 1;    // no devider, Solar Voltage up to 26V
      break;  // end case
    case 0x46:
      cartridge_R_SHUNT_OHM = 1000;     // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 1;    // no devider, Solar Voltage up to 26V
      break;  // end case
    case 0x4F:
      cartridge_R_SHUNT_OHM = 0.1;      // Shunt Resistor value in Ohms
      cartridge_Voltage_Devider = 2;    // Solar Voltage diveded by 2, to support up to 52V
      break;  // end case
  }

  actual_V_SHUNT_GainRange_mV = 320;    // start default is 320mV => GainDiv=8

  // init INA219 config register (GainDiv=8; Bus-Volt=32V; ADC=12bit; ADC=1sample/532µs; ADC continuous)
  INA219_config = INA219_CONFIG_GAIN_8_320MV |
                  INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C_Write_Register(INA219_REG_CONFIG, INA219_config);     // write new INA219_config to register

}//end of sub "init_INA219_defaults_by_I2C_addr"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: hot-swap test, if INA2019 cartridge is still inserted ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void HotSwap_test_INA219_I2C_addr(void)   // search for an INA219 cartridge by scanning possible I2C adresses
{
  Wire.beginTransmission (cartridge_INA219_I2C_addr);       // start I2C communiaction with slave
  if (Wire.endTransmission() != 0)                          // if slaved answered with no error
  {
    cartridge_INA219_I2C_addr = 0;                          // reset I2C address to indicate that cartridge is removed,
  }
  vTaskDelay(10 / portTICK_PERIOD_MS); //delay in ms
  // returns in gloabl variable "cartridge_INA219_I2C_addr = 0" if cartdridge is removed
}//end of sub "HotSwap_test_INA219_I2C_addr"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: I2C_Write_Register ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void I2C_Write_Register(byte RegisterNumber, uint16_t value)     // write to a register in INA219
{
  Wire.beginTransmission (cartridge_INA219_I2C_addr);       // start I2C communiaction with slave
  Wire.write(RegisterNumber);                 // Register to write to (16 bit)
  // write INT to I2C by shift out as 2 byte
  Wire.write((value >> 8) & 0xFF);            // Upper 8-bits
  Wire.write(value & 0xFF);                   // Lower 8-bits
  if (Wire.endTransmission() != 0)                          // if slaved answered with no error
  {
#if debug_level >= 2
    Serial.print(F("I2C error when writing register"));
#endif
  }
  vTaskDelay(10 / portTICK_PERIOD_MS); //delay in ms
}//end of sub "I2C_Write_Register"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: I2C_Read_Register ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint16_t I2C_Read_Register(byte RegisterNumber)   // read a register from INA219
{
  Wire.beginTransmission (cartridge_INA219_I2C_addr);       // start I2C communiaction with slave
  Wire.write(RegisterNumber);                 // Register to read from (16 bit)
  Wire.endTransmission();
  vTaskDelay(10 / portTICK_PERIOD_MS); // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(cartridge_INA219_I2C_addr, (uint8_t)2);
  // read 2 byte from I2C and shift to integer
  return (uint16_t)((Wire.read() << 8) | Wire.read());
}//end of sub "I2C_Read_Register"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: INA219_I2C_Read_ShuntVoltage ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
long INA219_I2C_Read_ShuntVoltage()   // read Shunt Voltage from an INA219
{
  int16_t V_value;
  uint16_t INA219_config;
  uint16_t INA219_CONFIG_GAIN_new;

  // initial read of Shunt_Voltage with highest PGA/GainDiv = 320mV
  // init INA219 config register (GainDiv=320mV; Bus-Volt=32V; ADC=12bit; ADC=1sample/532µs; ADC continuous)
  INA219_config = INA219_CONFIG_GAIN_8_320MV |
                  INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C_Write_Register(INA219_REG_CONFIG, INA219_config);       // write new INA219_config to register

  V_value = I2C_Read_Register(INA219_REG_SHUNTVOLTAGE);       // INA219 register value is signed-int in right 9-12bit, with left filling sign(s) depend on ADC resolution
  // INA219 Shunt Voltage Register has LSB = 10μV and max volt-equivalent is: PGA=8 is max +-32000, PGA=1 is +-4000

  // set PGA (1to8) is smallest to fit to measured Shunt_Voltage (mV), to get best ADC-resolution,
  switch (V_value)        // check on Shunt_Voltage raw from INA2019 (V_value in 0,01 mV)
  {
    case 0 ... 4000:
      INA219_CONFIG_GAIN_new = INA219_CONFIG_GAIN_1_40MV;     // Gain 1, 40mV Range
      actual_V_SHUNT_GainRange_mV = 40;
      break;  // end case
    case 4001 ... 8000:
      INA219_CONFIG_GAIN_new = INA219_CONFIG_GAIN_2_80MV;     // Gain 2, 80mV Range
      actual_V_SHUNT_GainRange_mV = 80;
      break;  // end case
    case 8001 ... 16000:
      INA219_CONFIG_GAIN_new = INA219_CONFIG_GAIN_4_160MV;     // Gain 3, 160mV Range
      actual_V_SHUNT_GainRange_mV = 160;
      break;  // end case
    case 16001 ... 32700:
      INA219_CONFIG_GAIN_new = INA219_CONFIG_GAIN_8_320MV;     // Gain 4, 320mV Range
      actual_V_SHUNT_GainRange_mV = 320;
      break;  // end case
    default:
      break;  // end case
  }

  // re-init INA219 config register (GainDiv=new; Bus-Volt=32V; ADC=12bit; ADC=1sample/532µs; ADC continuous)
  INA219_config = INA219_CONFIG_GAIN_new |
                  INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C_Write_Register(INA219_REG_CONFIG, INA219_config);   // write new INA219_config to register

  // now take the optimal read and ADC-resolution of Shunt_Voltage (in 0,01 mV)
  V_value = I2C_Read_Register(INA219_REG_SHUNTVOLTAGE);   // re-read INA219 Shunt Voltage register, using new PGA/GainDiv and ADC resolution

  return (long)((long)V_value * 10);                       // result within +-320µV (not mV)
}//end of sub "INA219_I2C_Read_ShuntVoltage"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: INA219_I2C_Read_BusVoltage ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
float INA219_I2C_Read_BusVoltage()    // read Bus Voltage from an INA219
{
  uint16_t V_value;
  V_value = I2C_Read_Register(INA219_REG_BUSVOLTAGE);         // INA219 register value (mV) in bit 15-3 (32V) or 14-3 (16V) , LSB always 4mV
  // shift 3 bits right to drop CNVR and OVF and multiply by LSB
  return (float)((int16_t)((V_value >> 3) * 4) * 0.001);    // result float in Volts
}//end of sub "INA219_I2C_Read_BusVoltage"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// *** Subroutine: normal_ThingSpeak_Status_Message ***
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void normal_ThingSpeak_Status_Msg()    // use the Solar Voltage and/or the INA219 Solar Current for the ThingSpeak status
// this is a special implementation using the values and energy data of the Solar Cells I use,
// the IF-THEN-ELSE cascade (on Volt and mA) needs to be adopted for other Solar Cells installations
{  // assume ADC7 is connected to measure the Solar Voltage
  if (ADC_CorrectedVoltage > 24)                                    // Solar Volt is close to max U-Solar (of my Solar Cell = 30V)
  {
    ThingSpeak.setStatus("the sun is shining here: Solar=" + String(ADC_CorrectedVoltage, 0) + "V");              // myStatus is zero terminated String (UTF8) <255 bytes
  }
  else
  {
    if (ADC_CorrectedVoltage > 14)                                  // minimum Solar Volt for charging battery (minimum PowerPoint of my Solar Cell is MPP=16,0V)
    {
      ThingSpeak.setStatus("it's cloudy here: Solar=" + String(ADC_CorrectedVoltage, 0) + "V ");       
    }
    else
    {
      if (ADC_CorrectedVoltage > 1)                                 // my Solar Cell can not be charging if Solar Volt <16V MPP)
      {
        ThingSpeak.setStatus("it's too shaded here: Solar=" + String(ADC_CorrectedVoltage, 0) + "V");  
      }
      else        
      // an ADC_CorrectedVoltage > 1 Volt can have 2 reasons:
          // - if ADC7 is connected to measure the Solar Voltage, then it is dark
          // - if ADC7 is not connected to measure the Solar Voltage, then use the INA219 measured Solar Charge Current for ThingSpeak status
      {
        if (lastInterval_average_Shunt_Current_mA > 100)            // no ADC7 connected, but high charge current
        {
          ThingSpeak.setStatus("the sun is charging hot");                                             
        }  
        else
        {
          if (lastInterval_average_Shunt_Current_mA > 30)           // no ADC7 connected, but good charge current
          {
            ThingSpeak.setStatus("the sun is charging nicely");                                        
          }
          else
          {
            if (lastInterval_average_Shunt_Current_mA > 1)
            {
              ThingSpeak.setStatus("the sun only charges a bit");   // no ADC7 connected, but low charge current                               
            }
            else                                                    // either ADC7 connected but it is night, OR , ADC7 not connected and battery is full
            {
              if (lastInterval_average_Supply_Voltage_V > 8.15)     // for my LT5236, after fully charged, it will wait for discahrge to 8,15V to restart CCCV charge cycle
              {
                ThingSpeak.setStatus("the battery is fully charged"); 
              }
              else                                                  // IF ("battery <8,1V" AND "no current is flowing") THEN it is night
              {
                ThingSpeak.setStatus("have a good night, it's dark here");                                        // myStatus is zero terminated String (UTF8) <255 bytes
              }
            }
          }
        }
      }
    }
  }
}//end of sub "normal_ThingSpeak_Status_Msg"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//------------------------------------------------------------------------
//   ****** End of subroutines ******
//-------------------------------------------------------------------------
