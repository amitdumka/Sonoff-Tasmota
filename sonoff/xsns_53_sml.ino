/*
  xsns_53_sml.ino - SML,OBIS,EBUS,RAW,COUNTER interface for Sonoff-Tasmota

  Created by Gerhard Mutz on 07.10.11.
  adapted for Tasmota

  Copyright (C) 2019  Gerhard Mutz and Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifdef USE_SML_M
//

#define XSNS_53 53

// Baudrate des D0 Ausgangs, sollte bei den meisten Zählern 9600 sein
#define SML_BAUDRATE 9600

// sende dies alle N Sekunden, für Zähler die erst auf Anforderung etwas senden
//#define SML_SEND_SEQ

// debug counter input to led for counter1 and 2
//#define DEBUG_CNT_LED1 2
//#define DEBUG_CNT_LED1 2

// use analog optical counter sensor with AD Converter ADS1115
//#define ANALOG_OPTO_SENSOR
// fototransistor mit pullup an A0, A1 des ADS1115 A3 and +3.3V
// die pegel und die verstärkung können dann automatisch kalibriert werden


// support für mehr als 2 Meter mit spezieller Tasmota Serial Version
// dazu muss der modifizierte Treiber => TasmotaSerial-2.3.1 ebenfalls kopiert werden

#include <TasmotaSerial.h>

// use special no wait serial driver
#define SPECIAL_SS

// addresse a bug in meter DWS74
//#define DWS74_BUG

// max 23 chars
#if DMY_LANGUAGE==de-DE
// german web text
#define D_TPWRIN "Verbrauch"
#define D_TPWROUT "Einspeisung"
#define D_TPWRCURR "Aktueller Verbrauch"
#define D_TPWRCURR1 "Verbrauch P1"
#define D_TPWRCURR2 "Verbrauch P2"
#define D_TPWRCURR3 "Verbrauch P3"
#define D_Strom_L1 "Strom L1"
#define D_Strom_L2 "Strom L2"
#define D_Strom_L3 "Strom L3"
#define D_Spannung_L1 "Spannung L1"
#define D_Spannung_L2 "Spannung L2"
#define D_Spannung_L3 "Spannung L3"
#define D_METERNR "Zähler Nr"
#define D_METERSID "Service ID"
#define D_GasIN "Zählerstand"                // Gas-Verbrauch
#define D_H2oIN "Zählerstand"                // H2o-Verbrauch
#define D_StL1L2L3 "Ströme L1+L2+L3"
#define D_SpL1L2L3 "Spannung L1+L2+L3/3"

#else
// other languages (tbd)
#undef D_TPWRIN
#undef D_TPWROUT
#undef D_TPWRCURR
#undef D_TPWRCURR1
#undef D_TPWRCURR2
#undef D_TPWRCURR3
#undef D_Strom_L1
#undef D_Strom_L2
#undef D_Strom_L3
#undef D_Spannung_L1
#undef D_Spannung_L2
#undef D_Spannung_L3
#undef D_METERNR
#undef D_METERSID
#undef D_GasIN
#undef D_H2oIN
#undef D_StL1L2L3
#undef D_SpL1L2L3

#define D_TPWRIN "Total-In"
#define D_TPWROUT "Total-Out"
#define D_TPWRCURR "Current-In/Out"
#define D_TPWRCURR1 "Current-In p1"
#define D_TPWRCURR2 "Current-In p2"
#define D_TPWRCURR3 "Current-In p3"
#define D_Strom_L1 "Current L1"
#define D_Strom_L2 "Current L2"
#define D_Strom_L3 "Current L3"
#define D_Spannung_L1 "Voltage L1"
#define D_Spannung_L2 "Voltage L2"
#define D_Spannung_L3 "Voltage L3"
#define D_METERNR "Meter_number"
#define D_METERSID "Service ID"
#define D_GasIN "Counter"                // Gas-Verbrauch
#define D_H2oIN "Counter"                // H2o-Verbrauch
#define D_StL1L2L3 "Current L1+L2+L3"
#define D_SpL1L2L3 "Voltage L1+L2+L3/3"

#endif

// JSON Strings do not translate
// max 23 char
#define DJ_TPWRIN "Total_in"
#define DJ_TPWROUT "Total_out"
#define DJ_TPWRCURR "Power_curr"
#define DJ_TPWRCURR1 "Power_p1"
#define DJ_TPWRCURR2 "Power_p2"
#define DJ_TPWRCURR3 "Power_p3"
#define DJ_CURR1 "Curr_p1"
#define DJ_CURR2 "Curr_p2"
#define DJ_CURR3 "Curr_p3"
#define DJ_VOLT1 "Volt_p1"
#define DJ_VOLT2 "Volt_p2"
#define DJ_VOLT3 "Volt_p3"
#define DJ_METERNR "Meter_number"
#define DJ_METERSID "Meter_id"
#define DJ_CSUM "Curr_summ"
#define DJ_VAVG "Volt_avg"
#define DJ_COUNTER "Count"

struct METER_DESC {
  uint8_t srcpin;
  uint8_t type;
  uint16_t flag;
  int32_t params;
  char prefix[8];
};

// meter list , enter new meters here
//=====================================================
#define EHZ161_0 1
#define EHZ161_1 2
#define EHZ363 3
#define EHZH 4
#define EDL300 5
#define Q3B 6
#define COMBO3 7
#define COMBO2 8
#define COMBO3a 9
#define Q3B_V1 10
#define EHZ363_2 11
#define COMBO3b 12
#define WGS_COMBO 13
#define EBZD_G 14

// select this meter
#define METER EHZ161_1


#if METER==EHZ161_0
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS"}};
const uint8_t meter[]=
"1,1-0:1.8.0*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.0*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,1-0:21.7.0*255(@1," D_TPWRCURR1 ",W," DJ_TPWRCURR1 ",0|"
"1,1-0:41.7.0*255(@1," D_TPWRCURR2 ",W," DJ_TPWRCURR2 ",0|"
"1,1-0:61.7.0*255(@1," D_TPWRCURR3 ",W," DJ_TPWRCURR3 ",0|"
"1,=m 3+4+5 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";

#endif

//=====================================================

#if METER==EHZ161_1
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS"}};
const uint8_t meter[]=
"1,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,=d 2 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";
#endif

//=====================================================

#if METER==EHZ363
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'s',0,SML_BAUDRATE,"SML"}};
// 2 Richtungszähler EHZ SML 8 bit 9600 baud, binär
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
//0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff
"1,77070100020800ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x10,0x07,0x00,0xff
"1,77070100100700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
//0x77,0x07,0x01,0x00,0x00,0x00,0x09,0xff
"1,77070100000009ff@#," D_METERNR ",," DJ_METERNR ",0";
#endif

//=====================================================

#if METER==EHZH
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'s',0,SML_BAUDRATE,"SML"}};
// 2 Richtungszähler EHZ SML 8 bit 9600 baud, binär
// verbrauch total
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xff
"1,77070100020800ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x0f,0x07,0x00,0xff
"1,770701000f0700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0";
#endif

//=====================================================

#if METER==EDL300
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'s',0,SML_BAUDRATE,"SML"}};
// 2 Richtungszähler EHZ SML 8 bit 9600 baud, binär
// verbrauch total
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xff
"1,77070100020801ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x0f,0x07,0x00,0xff
"1,770701000f0700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0";
#endif

#if METER==EBZD_G
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'s',0,SML_BAUDRATE,"strom"}};
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
// ..
"1,77070100020800ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xff
"1,77070100010801ff@1000," D_TPWRCURR1 ",KWh," DJ_TPWRCURR1 ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x02,0xff
"1,77070100010802ff@1000," D_TPWRCURR2 ",KWh," DJ_TPWRCURR2 ",4|"
// 77 07 01 00 10 07 00 FF
"1,77070100100700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
// ..
"1,77070100600100ff@#," D_METERNR ",," DJ_METERNR ",0";
#endif


//=====================================================

#if METER==Q3B
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'s',0,SML_BAUDRATE,"SML"}};
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
//0x77,0x07,0x01,0x00,0x02,0x08,0x01,0xff
"1,77070100020801ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x01,0x07,0x00,0xff
"1,77070100010700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0";
#endif

#if METER==COMBO3
// 3 Zähler Beispiel
#undef METERS_USED
#define METERS_USED 3

struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS"}, // harware serial RX pin
  [1]={14,'s',0,SML_BAUDRATE,"SML"}, // GPIO14 software serial
  [2]={4,'o',0,SML_BAUDRATE,"OBIS2"}}; // GPIO4 software serial

// 3 Zähler definiert
const uint8_t meter[]=
"1,1-0:1.8.0*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.0*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,1-0:21.7.0*255(@1," D_TPWRCURR1 ",W," DJ_TPWRCURR1 ",0|"
"1,1-0:41.7.0*255(@1," D_TPWRCURR2 ",W," DJ_TPWRCURR2 ",0|"
"1,1-0:61.7.0*255(@1," D_TPWRCURR3 ",W," DJ_TPWRCURR3 ",0|"
"1,=m 3+4+5 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0|"
"2,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"2,77070100020800ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"2,77070100100700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"3,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"3,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"3,=d 2 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"3,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";

#endif

#if METER==COMBO2
// 2 Zähler Beispiel
#undef METERS_USED
#define METERS_USED 2

struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS1"}, // harware serial RX pin
  [1]={14,'o',0,SML_BAUDRATE,"OBIS2"}}; // GPIO14 software serial

// 2 Zähler definiert
const uint8_t meter[]=
"1,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,=d 2 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0|"

"2,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"2,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"2,=d 6 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"2,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";

#endif

#if METER==COMBO3a
#undef METERS_USED
#define METERS_USED 3

struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS1"}, // harware serial RX pin
  [1]={14,'o',0,SML_BAUDRATE,"OBIS2"},
  [2]={1,'o',0,SML_BAUDRATE,"OBIS3"}};

// 3 Zähler definiert
const uint8_t meter[]=
"1,=h --- Zähler Nr 1 ---|"
"1,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,=d 2 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0|"
"2,=h --- Zähler Nr 2 ---|"
"2,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"2,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"2,=d 6 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"2,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0|"
"3,=h --- Zähler Nr 3 ---|"
"3,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"3,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"3,=d 10 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"3,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";

#endif

//=====================================================

#if METER==Q3B_V1
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
[0]={3,'o',0,SML_BAUDRATE,"OBIS"}};
const uint8_t meter[]=
"1,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,=d 1 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0";
#endif

//=====================================================

#if METER==EHZ363_2
#undef METERS_USED
#define METERS_USED 1
struct METER_DESC const meter_desc[METERS_USED]={
[0]={3,'s',0,SML_BAUDRATE,"SML"}};
// 2 Richtungszähler EHZ SML 8 bit 9600 baud, binär
const uint8_t meter[]=
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"1,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
//0x77,0x07,0x01,0x00,0x02,0x08,0x00,0xff
"1,77070100020800ff@1000," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x01,0xff
"1,77070100010801ff@1000," D_TPWRCURR1 ",KWh," DJ_TPWRCURR1 ",4|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x02,0xff
"1,77070100010802ff@1000," D_TPWRCURR2 ",KWh," DJ_TPWRCURR2 ",4|"
//0x77,0x07,0x01,0x00,0x10,0x07,0x00,0xff
"1,77070100100700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
//0x77,0x07,0x01,0x00,0x00,0x00,0x09,0xff
"1,77070100000009ff@#," D_METERNR ",," DJ_METERNR ",0";
#endif

// Beispiel für einen OBIS Stromzähler und einen Gaszähler + Wasserzähler
#if METER==COMBO3b
#undef METERS_USED
#define METERS_USED 3
struct METER_DESC const meter_desc[METERS_USED]={
  [0]={3,'o',0,SML_BAUDRATE,"OBIS"}, // harware serial RX pin
  [1]={14,'c',0,50,"Gas"}, // GPIO14 gas counter
  [2]={1,'c',0,10,"Wasser"}}; // water counter

// 3 Zähler definiert
const uint8_t meter[]=
"1,1-0:1.8.1*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
"1,1-0:2.8.1*255(@1," D_TPWROUT ",KWh," DJ_TPWROUT ",4|"
"1,=d 2 10 @1," D_TPWRCURR ",W," DJ_TPWRCURR ",0|"
"1,1-0:0.0.0*255(@#)," D_METERNR ",," DJ_METERNR ",0|"

// bei gaszählern (countern) muss der Vergleichsstring so aussehen wie hier
"2,1-0:1.8.0*255(@100," D_GasIN ",cbm," DJ_COUNTER ",2|"

"3,1-0:1.8.0*255(@100," D_H2oIN ",cbm," DJ_COUNTER ",2";
#endif


#if METER==WGS_COMBO
#undef METERS_USED
#define METERS_USED 3

struct METER_DESC const meter_desc[METERS_USED]={
  [0]={1,'c',0,10,"H20"}, // GPIO1 Wasser Zähler
  [1]={4,'c',0,50,"GAS"}, // GPIO4 gas Zähler
  [2]={3,'s',0,SML_BAUDRATE,"SML"}}; // SML harware serial RX pin

const uint8_t meter[]=
//----------------------------Wasserzähler--sensor53 c1------------------------------------
//"1,=h==================|"
"1,1-0:1.8.0*255(@10000," D_H2oIN ",cbm," DJ_COUNTER ",4|"            // 1
//----------------------------Gaszähler-----sensor53 c2------------------------------------
// bei gaszählern (countern) muss der Vergleichsstring so aussehen wie hier
"2,=h==================|"
"2,1-0:1.8.0*255(@100," D_GasIN ",cbm," DJ_COUNTER ",3|"              // 2
//----------------------------Stromzähler-EHZ363W5--sensor53 d0----------------------------
"3,=h==================|"
//0x77,0x07,0x01,0x00,0x01,0x08,0x00,0xff
"3,77070100010800ff@1000," D_TPWRIN ",KWh," DJ_TPWRIN ",3|"         // 3  Zählerstand Total
"3,=h==================|"
//0x77,0x07,0x01,0x00,0x10,0x07,0x00,0xff
"3,77070100100700ff@1," D_TPWRCURR ",W," DJ_TPWRCURR ",2|"          // 4  Aktuelle Leistung
"3,=h -------------------------------|"
"3,=m 10+11+12 @100," D_StL1L2L3 ",A," DJ_CSUM ",2|"            // 5  Summe Aktuelle Ströme
//"3,=h -------------------------------|"
"3,=m 13+14+15/#3 @100," D_SpL1L2L3 ",V," DJ_VAVG ",2|"      // 6   Mittelwert Spannungen
"3,=h==================|"
//0x77,0x07,0x01,0x00,0x24,0x07,0x00,0xff
"3,77070100240700ff@1," D_TPWRCURR1 ",W," DJ_TPWRCURR1 ",2|"        // 7  Wirkleistung L1
//0x77,0x07,0x01,0x00,0x38,0x07,0x00,0xff
"3,77070100380700ff@1," D_TPWRCURR2 ",W," DJ_TPWRCURR2 ",2|"        // 8  Wirkleistung L2
//0x77,0x07,0x01,0x00,0x4c,0x07,0x00,0xff
"3,770701004c0700ff@1," D_TPWRCURR3 ",W," DJ_TPWRCURR3 ",2|"        // 9  Wirkleistung L3
"3,=h -------------------------------|"
//0x77,0x07,0x01,0x00,0x1f,0x07,0x00,0xff
"3,770701001f0700ff@100," D_Strom_L1 ",A," DJ_CURR1 ",2|"        // 10 Strom L1
//0x77,0x07,0x01,0x00,0x33,0x07,0x00,0xff
"3,77070100330700ff@100," D_Strom_L2 ",A," DJ_CURR2 ",2|"        // 11 Strom L2
//0x77,0x07,0x01,0x00,0x47,0x07,0x00,0xff
"3,77070100470700ff@100," D_Strom_L3 ",A," DJ_CURR3 ",2|"        // 12 Strom L3
"3,=h -------------------------------|"
//0x77,0x07,0x01,0x00,0x20,0x07,0x00,0xff
"3,77070100200700ff@100," D_Spannung_L1 ",V," DJ_VOLT1 ",2|"  // 13 Spannung L1
//0x77,0x07,0x01,0x00,0x34,0x07,0x00,0xff
"3,77070100340700ff@100," D_Spannung_L2 ",V," DJ_VOLT2 ",2|"  // 14 Spannung L2
//0x77,0x07,0x01,0x00,0x48,0x07,0x00,0xff
"3,77070100480700ff@100," D_Spannung_L3 ",V," DJ_VOLT3 ",2|"  // 15 Spannung L3
"3,=h==================|"
//0x77,0x07,0x01,0x00,0x00,0x00,0x09,0xff
"3,77070100000009ff@#," D_METERSID ",," DJ_METERSID ",0|"           // 16 Service ID
"3,=h--------------------------------";                             // letzte Zeile
#endif


// this driver uses double because meter vars would not fit in float
//=====================================================

// median filter eliminates outliers, but uses much RAM and CPU cycles
// 672 bytes extra RAM with MAX_VARS = 16
//#define USE_MEDIAN_FILTER

// max number of vars , may be adjusted
#define MAX_VARS 16
// max number of meters , may be adjusted
#define MAX_METERS 5
double meter_vars[MAX_VARS];
// calulate deltas
#define MAX_DVARS MAX_METERS*2
double dvalues[MAX_DVARS];
uint32_t dtimes[MAX_DVARS];
uint8_t meters_used;

struct METER_DESC const *meter_desc_p;
const uint8_t *meter_p;

// software serial pointers
TasmotaSerial *meter_ss[MAX_METERS];

// serial buffers, may be made larger depending on telegram lenght
#define SML_BSIZ 48
uint8_t smltbuf[MAX_METERS][SML_BSIZ];

// meter nr as string
#define METER_ID_SIZE 24
char meter_id[MAX_METERS][METER_ID_SIZE];

#define EBUS_SYNC		0xaa
#define EBUS_ESC    0xa9
uint8_t ebus_pos;


#ifdef USE_MEDIAN_FILTER
// median filter, should be odd size
#define MEDIAN_SIZE 5
struct MEDIAN_FILTER {
double buffer[MEDIAN_SIZE];
int8_t index;
} sml_mf[MAX_VARS];

// calc median
double median(struct MEDIAN_FILTER* mf, double in) {
  double tbuff[MEDIAN_SIZE],tmp;
  uint8_t flag;
  mf->buffer[mf->index]=in;
  mf->index++;
  if (mf->index>=MEDIAN_SIZE) mf->index=0;
  // sort list and take median
  memmove(tbuff,mf->buffer,sizeof(tbuff));
  for (byte ocnt=0; ocnt<MEDIAN_SIZE; ocnt++) {
    flag=0;
    for (byte count=0; count<MEDIAN_SIZE-1; count++) {
      if (tbuff[count]>tbuff[count+1]) {
        tmp=tbuff[count];
        tbuff[count]=tbuff[count+1];
        tbuff[count+1]=tmp;
        flag=1;
      }
    }
    if (!flag) break;
  }
  return tbuff[MEDIAN_SIZE/2];
}
#endif

#ifdef ANALOG_OPTO_SENSOR
// sensor over ADS1115 with i2c Bus
uint8_t ads1115_up;

// ads1115 driver
#define SAMPLE_BIT (0x8000)

#define ADS1115_COMP_QUEUE_SHIFT 0
#define ADS1115_COMP_LATCH_SHIFT 2
#define ADS1115_COMP_POLARITY_SHIFT 3
#define ADS1115_COMP_MODE_SHIFT 4
#define ADS1115_DATA_RATE_SHIFT 5
#define ADS1115_MODE_SHIFT 8
#define ADS1115_PGA_SHIFT 9
#define ADS1115_MUX_SHIFT 12

enum ads1115_comp_queue {
	ADS1115_COMP_QUEUE_AFTER_ONE = 0,
	ADS1115_COMP_QUEUE_AFTER_TWO = 0x1 << ADS1115_COMP_QUEUE_SHIFT,
	ADS1115_COMP_QUEUE_AFTER_FOUR = 0x2 << ADS1115_COMP_QUEUE_SHIFT,
	ADS1115_COMP_QUEUE_DISABLE = 0x3 << ADS1115_COMP_QUEUE_SHIFT,
	ADS1115_COMP_QUEUE_MASK = 0x3 << ADS1115_COMP_QUEUE_SHIFT,
};

enum ads1115_comp_latch {
	ADS1115_COMP_LATCH_NO = 0,
	ADS1115_COMP_LATCH_YES = 1 << ADS1115_COMP_LATCH_SHIFT,
	ADS1115_COMP_LATCH_MASK = 1 << ADS1115_COMP_LATCH_SHIFT,
};

enum ads1115_comp_polarity {
	ADS1115_COMP_POLARITY_ACTIVE_LOW = 0,
	ADS1115_COMP_POLARITY_ACTIVE_HIGH = 1 << ADS1115_COMP_POLARITY_SHIFT,
	ADS1115_COMP_POLARITY_MASK = 1 << ADS1115_COMP_POLARITY_SHIFT,
};

enum ads1115_comp_mode {
	ADS1115_COMP_MODE_WINDOW = 0,
	ADS1115_COMP_MODE_HYSTERESIS = 1 << ADS1115_COMP_MODE_SHIFT,
	ADS1115_COMP_MODE_MASK = 1 << ADS1115_COMP_MODE_SHIFT,
};

enum ads1115_data_rate {
	ADS1115_DATA_RATE_8_SPS = 0,
	ADS1115_DATA_RATE_16_SPS = 0x1 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_32_SPS = 0x2 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_64_SPS = 0x3 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_128_SPS = 0x4 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_250_SPS = 0x5 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_475_SPS = 0x6 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_860_SPS = 0x7 << ADS1115_DATA_RATE_SHIFT,
	ADS1115_DATA_RATE_MASK = 0x7 << ADS1115_DATA_RATE_SHIFT,
};

enum ads1115_mode {
	ADS1115_MODE_CONTINUOUS = 0,
	ADS1115_MODE_SINGLE_SHOT = 1 << ADS1115_MODE_SHIFT,
	ADS1115_MODE_MASK = 1 << ADS1115_MODE_SHIFT,
};

enum ads1115_pga {
	ADS1115_PGA_TWO_THIRDS = 0, //±6.144 V
	ADS1115_PGA_ONE = 0x1 << ADS1115_PGA_SHIFT, //±4.096 V
	ADS1115_PGA_TWO = 0x2 << ADS1115_PGA_SHIFT, //±2.048 V
	ADS1115_PGA_FOUR = 0x3 << ADS1115_PGA_SHIFT, //±1.024 V
	ADS1115_PGA_EIGHT = 0x4 << ADS1115_PGA_SHIFT, //±0.512 V
	ADS1115_PGA_SIXTEEN = 0x5 << ADS1115_PGA_SHIFT, //±0.256 V
	ADS1115_PGA_MASK = 0x7 << ADS1115_PGA_SHIFT,
};


enum ads1115_mux {
	ADS1115_MUX_DIFF_AIN0_AIN1 = 0,
	ADS1115_MUX_DIFF_AIN0_AIN3 = 0x1 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_DIFF_AIN1_AIN3 = 0x2 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_DIFF_AIN2_AIN3 = 0x3 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_GND_AIN0 = 0x4 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_GND_AIN1 = 0x5 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_GND_AIN2 = 0x6 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_GND_AIN3 = 0x7 << ADS1115_MUX_SHIFT,
	ADS1115_MUX_MASK = 0x7 << ADS1115_MUX_SHIFT,
};

class ADS1115 {
public:
	ADS1115(uint8_t address = 0x48);

	void begin();
	uint8_t trigger_sample();
	uint8_t reset();
	bool is_sample_in_progress();
	int16_t read_sample();
	float sample_to_float(int16_t val);
	float read_sample_float();

	void set_comp_queue(enum ads1115_comp_queue val) { set_config(val, ADS1115_COMP_QUEUE_MASK); }
	void set_comp_latching(enum ads1115_comp_latch val) { set_config(val, ADS1115_COMP_LATCH_MASK); }
	void set_comp_polarity(enum ads1115_comp_polarity val) { set_config(val, ADS1115_COMP_POLARITY_MASK); }
	void set_comp_mode(enum ads1115_comp_mode val) { set_config(val, ADS1115_COMP_MODE_MASK); }
	void set_data_rate(enum ads1115_data_rate val) { set_config(val, ADS1115_DATA_RATE_MASK); }
	void set_mode(enum ads1115_mode val) { set_config(val, ADS1115_MODE_MASK); }
	void set_pga(enum ads1115_pga val) { set_config(val, ADS1115_PGA_MASK); m_voltage_range = val >> ADS1115_PGA_SHIFT; }
	void set_mux(enum ads1115_mux val) { set_config(val, ADS1115_MUX_MASK); }

private:
	void set_config(uint16_t val, uint16_t mask) {
		m_config = (m_config & ~mask) | val;
	}

	uint8_t write_register(uint8_t reg, uint16_t val);
	uint16_t read_register(uint8_t reg);

	uint8_t m_address;
	uint16_t m_config;
	int m_voltage_range;
};


enum ads1115_register {
	ADS1115_REGISTER_CONVERSION = 0,
	ADS1115_REGISTER_CONFIG = 1,
	ADS1115_REGISTER_LOW_THRESH = 2,
	ADS1115_REGISTER_HIGH_THRESH = 3,
};

#define FACTOR 32768.0
static float ranges[] = { 6.144 / FACTOR, 4.096 / FACTOR, 2.048 / FACTOR, 1.024 / FACTOR, 0.512 / FACTOR, 0.256 / FACTOR};

ADS1115::ADS1115(uint8_t address)
{
        m_address = address;
        m_config = ADS1115_COMP_QUEUE_AFTER_ONE |
                   ADS1115_COMP_LATCH_NO |
                   ADS1115_COMP_POLARITY_ACTIVE_LOW |
                   ADS1115_COMP_MODE_WINDOW |
                   ADS1115_DATA_RATE_128_SPS |
                   ADS1115_MODE_SINGLE_SHOT |
                   ADS1115_MUX_GND_AIN0;
        set_pga(ADS1115_PGA_ONE);
}

uint8_t ADS1115::write_register(uint8_t reg, uint16_t val)
{
        Wire.beginTransmission(m_address);
        Wire.write(reg);
        Wire.write(val>>8);
        Wire.write(val & 0xFF);
        return Wire.endTransmission();
}

uint16_t ADS1115::read_register(uint8_t reg)
{
        Wire.beginTransmission(m_address);
        Wire.write(reg);
        Wire.endTransmission();

        uint8_t result = Wire.requestFrom((int)m_address, 2, 1);
        if (result != 2) {
                Wire.flush();
                return 0;
        }

        uint16_t val;

        val = Wire.read() << 8;
        val |= Wire.read();
        return val;
}

void ADS1115::begin()
{
        Wire.begin();
}

uint8_t ADS1115::trigger_sample()
{
        return write_register(ADS1115_REGISTER_CONFIG, m_config | SAMPLE_BIT);
}

uint8_t ADS1115::reset()
{
	Wire.beginTransmission(0);
	Wire.write(0x6);
	return Wire.endTransmission();
}

bool ADS1115::is_sample_in_progress()
{
	uint16_t val = read_register(ADS1115_REGISTER_CONFIG);
	return (val & SAMPLE_BIT) == 0;
}

int16_t ADS1115::read_sample()
{
        return read_register(ADS1115_REGISTER_CONVERSION);
}

float ADS1115::sample_to_float(int16_t val)
{
	return val * ranges[m_voltage_range];
}

float ADS1115::read_sample_float()
{
	return sample_to_float(read_sample());
}

ADS1115 adc;

void ADS1115_init(void) {

  ads1115_up=0;
  if (!i2c_flg) return;

  adc.begin();
  adc.set_data_rate(ADS1115_DATA_RATE_128_SPS);
  adc.set_mode(ADS1115_MODE_CONTINUOUS);
  adc.set_mux(ADS1115_MUX_DIFF_AIN0_AIN3);
  adc.set_pga(ADS1115_PGA_TWO);

  int16_t val = adc.read_sample();
  ads1115_up=1;
}

#endif

char sml_start;
uint8_t dump2log=0;

#define SML_SAVAILABLE Serial_available()
#define SML_SREAD Serial_read()
#define SML_SPEAK Serial_peek()

bool Serial_available() {
  uint8_t num=dump2log&7;
  if (num<1 || num>meters_used) return Serial.available();
  if (num==1) {
      return Serial.available();
  } else {
    return meter_ss[num-1]->available();
  }
}

uint8_t Serial_read() {
  uint8_t num=dump2log&7;
  if (num<1 || num>meters_used) return Serial.read();
  if (num==1) {
      return Serial.read();
  } else {
    return meter_ss[num-1]->read();
  }
}

uint8_t Serial_peek() {
  uint8_t num=dump2log&7;
  if (num<1 || num>meters_used) return Serial.peek();
  if (num==1) {
      return Serial.peek();
  } else {
    return meter_ss[num-1]->peek();
  }
}

void Dump2log(void) {

int16_t index=0,hcnt=0;
uint32_t d_lastms;
uint8_t dchars[16];

  if (!SML_SAVAILABLE) return;

  if (dump2log&8) {
    // combo mode
    while (SML_SAVAILABLE) {
      log_data[index]=':';
      index++;
      log_data[index]=' ';
      index++;
      d_lastms=millis();
      while ((millis()-d_lastms)<40) {
        if (SML_SAVAILABLE) {
          uint8_t c=SML_SREAD;
          sprintf(&log_data[index],"%02x ",c);
          dchars[hcnt]=c;
          index+=3;
          hcnt++;
          if (hcnt>15) {
            // line complete, build asci chars
            log_data[index]='=';
            index++;
            log_data[index]='>';
            index++;
            log_data[index]=' ';
            index++;
            for (uint8_t ccnt=0; ccnt<16; ccnt++) {
              if (isprint(dchars[ccnt])) {
                log_data[index]=dchars[ccnt];
              } else {
                log_data[index]=' ';
              }
              index++;
            }
            break;
          }
        }
      }
      if (index>0) {
        log_data[index]=0;
        AddLog(LOG_LEVEL_INFO);
        index=0;
        hcnt=0;
      }
    }
  } else {
    //while (SML_SAVAILABLE) {
      index=0;
      log_data[index]=':';
      index++;
      log_data[index]=' ';
      index++;
      d_lastms=millis();
      while ((millis()-d_lastms)<40) {
        if (SML_SAVAILABLE) {
          if (meter_desc_p[(dump2log&7)-1].type=='o') {
            // obis
            char c=SML_SREAD&0x7f;
            if (c=='\n' || c=='\r') break;
            log_data[index]=c;
            index++;
          } else {
            unsigned char c;
            if (meter_desc_p[(dump2log&7)-1].type=='e') {
              // ebus
              c=SML_SREAD;
              sprintf(&log_data[index],"%02x ",c);
              index+=3;
              if (c==EBUS_SYNC) break;
            } else {
              // sml
              if (sml_start==0x77) {
                sml_start=0;
              } else {
                c=SML_SPEAK;
                if (c==0x77) {
                  sml_start=c;
                  break;
                }
              }
              c=SML_SREAD;
              sprintf(&log_data[index],"%02x ",c);
              index+=3;
            }
          }
        }
      }
      if (index>0) {
        log_data[index]=0;
        AddLog(LOG_LEVEL_INFO);
      }
    }
  //}

}

// skip sml entries
uint8_t *skip_sml(uint8_t *cp,int16_t *res) {
    uint8_t len,len1,type;
    len=*cp&0xf;
    type=*cp&0x70;
    if (type==0x70) {
        // list, skip entries
        // list
        cp++;
        while (len--) {
            len1=*cp&0x0f;
            cp+=len1;
        }
         *res=0;
    } else {
        // skip len
        *res=(signed char)*(cp+1);
        cp+=len;
    }
    return cp;
}

// get sml binary value
// not defined for unsigned >0x7fff ffff ffff ffff (should never happen)
double sml_getvalue(unsigned char *cp,uint8_t index) {
uint8_t len,unit,type;
int16_t scaler,result;
int64_t value;
double dval;

  // scan for values
  // check status
  cp=skip_sml(cp,&result);
  // check time
  cp=skip_sml(cp,&result);
  // check unit
  cp=skip_sml(cp,&result);
  // check scaler
  cp=skip_sml(cp,&result);
  scaler=result;
  // get value
  type=*cp&0x70;
  len=*cp&0x0f;
  cp++;
    if (type==0x50 || type==0x60) {
        // shift into 64 bit
        uint64_t uvalue=0;
        uint8_t nlen=len;
        while (--nlen) {
            uvalue<<=8;
            uvalue|=*cp++;
        }
        if (type==0x50) {
            // signed
            switch (len-1) {
                case 1:
                    // byte
                    value=(signed char)uvalue;
                    break;
                case 2:
                    // signed 16 bit
#ifdef DWS74_BUG
                    if (scaler==-2) {
                      value=(uint32_t)uvalue;
                    } else {
                      value=(int16_t)uvalue;
                    }
#else
                    value=(int16_t)uvalue;
#endif
                    break;
                case 3:
                case 4:
                    // signed 32 bit
                    value=(int32_t)uvalue;
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                    // signed 64 bit
                    value=(int64_t)uvalue;
                    break;
            }
        } else {
            // unsigned
            value=uvalue;
        }

    } else {
        if (!(type&0xf0)) {
            // octet string serial number
            // no coding found on the net
            // up to now 2 types identified on Hager
            if (len==9) {
              // serial number on hager => 24 bit - 24 bit
                cp++;
                uint32_t s1,s2;
                s1=*cp<<16|*(cp+1)<<8|*(cp+2);
                cp+=4;
                s2=*cp<<16|*(cp+1)<<8|*(cp+2);
                sprintf(&meter_id[index][0],"%u-%u",s1,s2);
            } else {
                // server id on hager
                char *str=&meter_id[index][0];
                for (type=0; type<len-1; type++) {
                    sprintf(str,"%02x",*cp++);
                    str+=2;
                }
            }
            value=0;
        } else {
            value=999999;
            scaler=0;
        }
    }
    dval=value;
    if (scaler==-1) {
      dval/=10;
    } else if (scaler==-2) {
      dval/=100;
    } else if (scaler==-3) {
      dval/=1000;
    } else if (scaler==-4) {
      dval/=10000;
    } else if (scaler==1) {
      dval*=10;
    } else if (scaler==2) {
      dval*=100;
    } else if (scaler==3) {
      dval*=1000;
    }
    return dval;
}

uint8_t hexnibble(char chr) {
  uint8_t rVal = 0;
  if (isdigit(chr)) {
    rVal = chr - '0';
  } else  {
    chr=toupper(chr);
    if (chr >= 'A' && chr <= 'F') rVal = chr + 10 - 'A';
  }
  return rVal;
}

uint8_t sb_counter;

// because orig CharToDouble was defective
// fixed in Tasmota  6.4.1.19 20190222
double xCharToDouble(const char *str)
{
  // simple ascii to double, because atof or strtod are too large
  char strbuf[24];

  strlcpy(strbuf, str, sizeof(strbuf));
  char *pt = strbuf;
  while ((*pt != '\0') && isblank(*pt)) { pt++; }  // Trim leading spaces

  signed char sign = 1;
  if (*pt == '-') { sign = -1; }
  if (*pt == '-' || *pt=='+') { pt++; }            // Skip any sign

  double left = 0;
  if (*pt != '.') {
    left = atoi(pt);                               // Get left part
    while (isdigit(*pt)) { pt++; }                 // Skip number
  }

  double right = 0;
  if (*pt == '.') {
    pt++;
    right = atoi(pt);                              // Decimal part
    while (isdigit(*pt)) {
      pt++;
      right /= 10.0;
    }
  }

  double result = left + right;
  if (sign < 0) {
    return -result;                                // Add negative sign
  }
  return result;
}


// remove ebus escapes
void ebus_esc(uint8_t *ebus_buffer, unsigned char len) {
    short count,count1;
    for (count=0; count<len; count++) {
        if (ebus_buffer[count]==EBUS_ESC) {
            //found escape
            ebus_buffer[count]+=ebus_buffer[count+1];
            // remove 2. char
            count++;
            for (count1=count; count1<len; count1++) {
                ebus_buffer[count1]=ebus_buffer[count1+1];
            }
        }
    }

}

uint8_t ebus_crc8(uint8_t data, uint8_t crc_init) {
	uint8_t crc;
	uint8_t polynom;
	int i;

	crc = crc_init;
	for (i = 0; i < 8; i++) {
		if (crc & 0x80) {
			polynom = (uint8_t) 0x9B;
		}
		else {
			polynom = (uint8_t) 0;
		}
		crc = (uint8_t)((crc & ~0x80) << 1);
		if (data & 0x80) {
			crc = (uint8_t)(crc | 1) ;
		}
		crc = (uint8_t)(crc ^ polynom);
		data = (uint8_t)(data << 1);
	}
	return (crc);
}

// ebus crc
uint8_t ebus_CalculateCRC( uint8_t *Data, uint16_t DataLen ) {
	uint16_t i;
	uint8_t Crc = 0;
	for(i = 0 ; i < DataLen ; ++i, ++Data ) {
      Crc = ebus_crc8( *Data, Crc );
   }
   return Crc;
}

void sml_shift_in(uint32_t meters,uint32_t shard) {
  uint32_t count;
  if (meter_desc_p[meters].type!='e') {
    // shift in
    for (count=0; count<SML_BSIZ-1; count++) {
      smltbuf[meters][count]=smltbuf[meters][count+1];
    }
  }
  uint8_t iob;
  if (shard) iob=(uint8_t)Serial.read();
  else iob=(uint8_t)meter_ss[meters]->read();

  if (meter_desc_p[meters].type=='o') {
    smltbuf[meters][SML_BSIZ-1]=iob&0x7f;
  } else if (meter_desc_p[meters].type=='s') {
    smltbuf[meters][SML_BSIZ-1]=iob;
  } else if (meter_desc_p[meters].type=='r') {
    smltbuf[meters][SML_BSIZ-1]=iob;
  } else {
    if (iob==EBUS_SYNC) {
    	// should be end of telegramm
      // QQ,ZZ,PB,SB,NN ..... CRC, ACK SYNC
      if (ebus_pos>4+5) {
      	// get telegramm lenght
        uint8_t tlen=smltbuf[meters][4]+5;
        // test crc
        if (smltbuf[meters][tlen]=ebus_CalculateCRC(smltbuf[meters],tlen)) {
            ebus_esc(smltbuf[meters],tlen);
            //eBus_analyze();
            // XX0204UUSS@
            SML_Decode(meters);
            //AddLog_P(LOG_LEVEL_INFO, PSTR("ebus block found"));
						//ebus_set_timeout();
        } else {
            // crc error
            //AddLog_P(LOG_LEVEL_INFO, PSTR("ebus crc error"));
        }
      }
      ebus_pos=0;
      return;
    }
		smltbuf[meters][ebus_pos] = iob;
		ebus_pos++;
		if (ebus_pos>=SML_BSIZ) {
			ebus_pos=0;
		}
  }
  sb_counter++;
  if (meter_desc_p[meters].type!='e') SML_Decode(meters);
}


// polled every 50 ms
void SML_Poll(void) {
uint32_t meters;

    for (meters=0; meters<meters_used; meters++) {
      if (meter_desc_p[meters].type!='c') {
        // poll for serial input
        if (meter_desc_p[meters].srcpin==3) {
          while (Serial.available()) {
            sml_shift_in(meters,1);
          }
        } else {
          while (meter_ss[meters]->available()) {
            sml_shift_in(meters,0);
          }
        }
      }
    }
}


void SML_Decode(uint8_t index) {
  const char *mp=(const char*)meter_p;
  int8_t mindex;
  uint8_t *cp;
  uint8_t dindex=0,vindex=0;
  delay(0);
  while (mp != NULL) {
    // check list of defines

    // new section
    mindex=((*mp)&7)-1;

    if (mindex<0 || mindex>=meters_used) mindex=0;
    mp+=2;
    if (*mp=='=' && *(mp+1)=='h') {
      mp = strchr(mp, '|');
      if (mp) mp++;
      continue;
    }

    if (index!=mindex) goto nextsect;

    // start of serial source buffer
    cp=&smltbuf[mindex][0];

    // compare
    if (*mp=='=') {
      // calculated entry, check syntax
      mp++;
      // do math m 1+2+3
      if (*mp=='m' && !sb_counter) {
        // only every 256 th byte
        // else it would be calculated every single serial byte
        mp++;
        while (*mp==' ') mp++;
        // 1. index
        double dvar;
        uint8_t opr;
        uint32_t ind;
        ind=atoi(mp);
        while (*mp>='0' && *mp<='9') mp++;
        if (ind<1 || ind>MAX_VARS) ind=1;
        dvar=meter_vars[ind-1];
        for (uint8_t p=0;p<5;p++) {
          if (*mp=='@') {
            // store result
            meter_vars[vindex]=dvar;
            mp++;
            SML_Immediate_MQTT((const char*)mp,vindex,mindex);
            break;
          }
          opr=*mp;
          mp++;
          uint8_t iflg=0;
          if (*mp=='#') {
            iflg=1;
            mp++;
          }
          ind=atoi(mp);
          while (*mp>='0' && *mp<='9') mp++;
          if (ind<1 || ind>MAX_VARS) ind=1;
          switch (opr) {
              case '+':
                if (iflg) dvar+=ind;
                else dvar+=meter_vars[ind-1];
                break;
              case '-':
                if (iflg) dvar-=ind;
                else dvar-=meter_vars[ind-1];
                break;
              case '*':
                if (iflg) dvar*=ind;
                else dvar*=meter_vars[ind-1];
                break;
              case '/':
                if (iflg) dvar/=ind;
                else dvar/=meter_vars[ind-1];
                break;
          }
          while (*mp==' ') mp++;
          if (*mp=='@') {
            // store result
            meter_vars[vindex]=dvar;
            mp++;
            SML_Immediate_MQTT((const char*)mp,vindex,mindex);
            break;
          }
        }
      } else if (*mp=='d') {
        // calc deltas d ind 10 (eg every 10 secs)
        if (dindex<MAX_DVARS) {
          // only n indexes
          mp++;
          while (*mp==' ') mp++;
          uint8_t ind=atoi(mp);
          while (*mp>='0' && *mp<='9') mp++;
          if (ind<1 || ind>MAX_VARS) ind=1;
          uint32_t delay=atoi(mp)*1000;
          uint32_t dtime=millis()-dtimes[dindex];
          if (dtime>delay) {
            // calc difference
            dtimes[dindex]=millis();
            double vdiff = meter_vars[ind-1]-dvalues[dindex];
            dvalues[dindex]=meter_vars[ind-1];
            meter_vars[vindex]=(double)360000.0*vdiff/((double)dtime/10000.0);

            mp=strchr(mp,'@');
            if (mp) {
              mp++;
              SML_Immediate_MQTT((const char*)mp,vindex,mindex);
            }
          }
          dindex++;
        }
      } else if (*mp=='h') {
        // skip html tag line
        mp = strchr(mp, '|');
        if (mp) mp++;
        continue;
      }
    } else {
      // compare value
      uint8_t found=1;
      int32_t ebus_dval=99;
      while (*mp!='@') {
        if (meter_desc_p[mindex].type=='o' || meter_desc_p[mindex].type=='c') {
          if (*mp++!=*cp++) {
            found=0;
          }
        } else {
          if (meter_desc_p[mindex].type=='s') {
            // sml
            uint8_t val = hexnibble(*mp++) << 4;
            val |= hexnibble(*mp++);
            if (val!=*cp++) {
              found=0;
            }
          } else {
            // ebus or raw
            // XXHHHHSSUU
            if (*mp=='x' && *(mp+1)=='x') {
              //ignore
              mp+=2;
              cp++;
            } else if (*mp=='u' && *(mp+1)=='u' && *(mp+2)=='u' && *(mp+3)=='u'){
              uint16_t val = *cp|(*(cp+1)<<8);
              ebus_dval=val;
              mp+=4;
              cp+=2;
            } else if (*mp=='u' && *(mp+1)=='u') {
              uint8_t val = *cp++;
              ebus_dval=val;
              mp+=2;
            }
            else if (*mp=='s' && *(mp+1)=='s' && *(mp+2)=='s' && *(mp+3)=='s'){
              int16_t val = *cp|(*(cp+1)<<8);
              ebus_dval=val;
              mp+=4;
              cp+=2;
            }
            else if (*mp=='s' && *(mp+1)=='s') {
              int8_t val = *cp++;
              ebus_dval=val;
              mp+=2;
            } else {
              uint8_t val = hexnibble(*mp++) << 4;
              val |= hexnibble(*mp++);
              if (val!=*cp++) {
                found=0;
              }
            }
          }
        }
      }
      if (found) {
        // matches, get value
        mp++;
        if (*mp=='#') {
          // get string value
          mp++;
          if (meter_desc_p[mindex].type=='o') {
            for (uint8_t p=0;p<METER_ID_SIZE;p++) {
              if (*cp==*mp) {
                meter_id[mindex][p]=0;
                break;
              }
              meter_id[mindex][p]=*cp++;
            }
          } else {
            sml_getvalue(cp,mindex);
          }
        } else {
          double dval;
          if (meter_desc_p[mindex].type!='e' && meter_desc_p[mindex].type!='r') {
            // get numeric values
            if (meter_desc_p[mindex].type=='o' || meter_desc_p[mindex].type=='c') {
              dval=xCharToDouble((char*)cp);
            } else {
              dval=sml_getvalue(cp,mindex);
            }
          } else {
            // ebus
            if (*mp=='b') {
              mp++;
              uint8_t shift=*mp&7;
              ebus_dval>>=shift;
              ebus_dval&=1;
              mp+=2;
            }
            dval=ebus_dval;
          }
#ifdef USE_MEDIAN_FILTER
          meter_vars[vindex]=median(&sml_mf[vindex],dval);
#else
          meter_vars[vindex]=dval;
#endif
          // get scaling factor
          double fac=xCharToDouble((char*)mp);
          meter_vars[vindex]/=fac;
          SML_Immediate_MQTT((const char*)mp,vindex,mindex);
        }
      }
    }
nextsect:
    // next section
    if (vindex<MAX_VARS-1) {
      vindex++;
    }
    mp = strchr(mp, '|');
    if (mp) mp++;
  }
}

//"1-0:1.8.0*255(@1," D_TPWRIN ",KWh," DJ_TPWRIN ",4|"
void SML_Immediate_MQTT(const char *mp,uint8_t index,uint8_t mindex) {
  char tpowstr[32];
  char jname[24];

  // we must skip sf,webname,unit
  char *cp=strchr(mp,',');
  if (cp) {
    cp++;
    // wn
    cp=strchr(cp,',');
    if (cp) {
      cp++;
      // unit
      cp=strchr(cp,',');
      if (cp) {
        cp++;
        // json mqtt
        for (uint8_t count=0;count<sizeof(jname);count++) {
          if (*cp==',') {
            jname[count]=0;
            break;
          }
          jname[count]=*cp++;
        }
        cp++;
        uint8_t dp=atoi(cp);
        if (dp&0x10) {
          // immediate mqtt
          dtostrfd(meter_vars[index],dp&0xf,tpowstr);
          ResponseBeginTime();
          ResponseAppend_P(PSTR(",\"%s\":{\"%s\":%s}}"),meter_desc_p[mindex].prefix,jname,tpowstr);
          MqttPublishPrefixTopic_P(TELE, PSTR(D_RSLT_SENSOR), Settings.flag.mqtt_sensor_retain);
        }
      }
    }
  }
}

// web + json interface
void SML_Show(boolean json) {
  int8_t count,mindex,cindex=0;
  char tpowstr[32];
  char name[24];
  char unit[8];
  char jname[24];
  int8_t index=0,mid=0;
  char *mp=(char*)meter_p;
  char *cp;
  //char b_mqtt_data[MESSZ];
  //b_mqtt_data[0]=0;

    int8_t lastmind=((*mp)&7)-1;
    if (lastmind<0 || lastmind>=meters_used) lastmind=0;
    while (mp != NULL) {
        // setup sections
        mindex=((*mp)&7)-1;
        if (mindex<0 || mindex>=meters_used) mindex=0;
        mp+=2;
        if (*mp=='=' && *(mp+1)=='h') {
          mp+=2;
          // html tag
          if (json) {
            mp = strchr(mp, '|');
            if (mp) mp++;
            continue;
          }
          // web ui export
          uint8_t i;
          for (i=0;i<sizeof(tpowstr)-2;i++) {
            if (*mp=='|' || *mp==0) break;
            tpowstr[i]=*mp++;
          }
          tpowstr[i]=0;
          // export html
          //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s{s}%s{e}", b_mqtt_data,tpowstr);
          WSContentSend_PD(PSTR("{s}%s{e}"),tpowstr);
          // rewind, to ensure strchr
          mp--;
          mp = strchr(mp, '|');
          if (mp) mp++;
          continue;
        }
        // skip compare section
        cp=strchr(mp,'@');
        if (cp) {
          cp++;
          if (*cp=='#') {
            // meter id
            sprintf(tpowstr,"\"%s\"",&meter_id[mindex][0]);
            mid=1;
          } else {
            mid=0;
          }
          // skip scaling
          cp=strchr(cp,',');
          if (cp) {
            // this is the name in web UI
            cp++;
            for (count=0;count<sizeof(name);count++) {
              if (*cp==',') {
                name[count]=0;
                break;
              }
              name[count]=*cp++;
            }
            cp++;

            for (count=0;count<sizeof(unit);count++) {
              if (*cp==',') {
                unit[count]=0;
                break;
              }
              unit[count]=*cp++;
            }
            cp++;

            for (count=0;count<sizeof(jname);count++) {
              if (*cp==',') {
                jname[count]=0;
                break;
              }
              jname[count]=*cp++;
            }

            cp++;

            if (!mid) {
              uint8_t dp=atoi(cp)&0xf;
              dtostrfd(meter_vars[index],dp,tpowstr);
            }

            if (json) {
              // json export
              if (index==0) {
                //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s,\"%s\":{\"%s\":%s", b_mqtt_data,meter_desc_p[mindex].prefix,jname,tpowstr);
                ResponseAppend_P(PSTR(",\"%s\":{\"%s\":%s"),meter_desc_p[mindex].prefix,jname,tpowstr);
              }
              else {
                if (lastmind!=mindex) {
                  // meter changed, close mqtt
                  //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s}", b_mqtt_data);
                  ResponseAppend_P(PSTR("}"));
                  // and open new
                  //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s,\"%s\":{\"%s\":%s", b_mqtt_data,meter_desc_p[mindex].prefix,jname,tpowstr);
                  ResponseAppend_P(PSTR(",\"%s\":{\"%s\":%s"),meter_desc_p[mindex].prefix,jname,tpowstr);
                  lastmind=mindex;
                } else {
                  //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s,\"%s\":%s", b_mqtt_data,jname,tpowstr);
                  ResponseAppend_P(PSTR(",\"%s\":%s"),jname,tpowstr);
                }
              }
            } else {
              // web ui export
              //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s{s}%s %s: {m}%s %s{e}", b_mqtt_data,meter_desc[mindex].prefix,name,tpowstr,unit);
              WSContentSend_PD(PSTR("{s}%s %s: {m}%s %s{e}"),meter_desc_p[mindex].prefix,name,tpowstr,unit);
            }
          }
        }
        if (index<MAX_VARS-1) {
          index++;
        }
        // next section
        mp = strchr(cp, '|');
        if (mp) mp++;
    }
    if (json) {
     //snprintf_P(b_mqtt_data, sizeof(b_mqtt_data), "%s}", b_mqtt_data);
     //ResponseAppend_P(PSTR("%s"),b_mqtt_data);
     ResponseAppend_P(PSTR("}"));
   } else {
     //WSContentSend_PD(PSTR("%s"),b_mqtt_data);
   }


#ifdef USE_DOMOTICZ
  if (json && !tele_period) {
    char str[16];
    dtostrfd(meter_vars[0], 1, str);
    DomoticzSensorPowerEnergy(meter_vars[1], str);  // PowerUsage, EnergyToday
    dtostrfd(meter_vars[2], 1, str);
    DomoticzSensor(DZ_VOLTAGE, str);  // Voltage
    dtostrfd(meter_vars[3], 1, str);
    DomoticzSensor(DZ_CURRENT, str);  // Current
  }
#endif  // USE_DOMOTICZ
}

struct SML_COUNTER {
  uint8_t sml_cnt_debounce;
  uint8_t sml_cnt_old_state;
  uint32_t sml_cnt_last_ts;
  uint32_t sml_counter_ltime;
  uint16_t sml_debounce;

#ifdef ANALOG_OPTO_SENSOR
  int16_t ana_curr;
  int16_t ana_max;
  int16_t ana_min;
  int16_t ana_cmpl;
  int16_t ana_cmph;
#endif
} sml_counters[MAX_COUNTERS];


void SML_CounterUpd(uint8_t index) {
  uint32_t ltime=millis()-sml_counters[index].sml_counter_ltime;
  sml_counters[index].sml_counter_ltime=millis();
  if (ltime>sml_counters[index].sml_debounce) {
    RtcSettings.pulse_counter[index]++;
    InjektCounterValue(sml_counters[index].sml_cnt_old_state,RtcSettings.pulse_counter[index]);
  }
}

void SML_CounterUpd1(void) {
  SML_CounterUpd(0);
}

void SML_CounterUpd2(void) {
  SML_CounterUpd(1);
}

void SML_CounterUpd3(void) {
  SML_CounterUpd(2);
}

void SML_CounterUpd4(void) {
  SML_CounterUpd(3);
}

#ifdef USE_SCRIPT
struct METER_DESC  script_meter_desc[MAX_METERS];
uint8_t *script_meter;
#endif

#define METER_DEF_SIZE 2000

bool Gpio_used(uint8_t gpiopin) {
  for (uint16_t i=0;i<GPIO_SENSOR_END;i++) {
    if (pin[i]==gpiopin) {
      return true;
    }
  }
  return false;
}

void SML_Init(void) {
  meters_used=METERS_USED;
  meter_desc_p=meter_desc;
  meter_p=meter;

  for (uint32_t cnt=0;cnt<MAX_VARS;cnt++) {
    meter_vars[cnt]=0;
  }


#ifdef USE_SCRIPT
  uint8_t meter_script=Run_Scripter(">M",-2,0);
  if (meter_script==99) {
    // use script definition
    if (script_meter) free(script_meter);
    uint8_t *tp=0;
    uint16_t index=0;
    uint8_t section=0;
    char *lp=glob_script_mem.scriptptr;
    while (lp) {
      if (!section) {
        if (*lp=='>' && *(lp+1)=='M') {
          lp+=2;
          meters_used=strtol(lp,0,10);
          section=1;
          uint32_t mlen=METER_DEF_SIZE;
          for (uint32_t cnt=0;cnt<METER_DEF_SIZE;cnt++) {
            if (lp[cnt]=='#') {
              mlen=cnt+3;
              break;
            }
          }
          if (mlen==METER_DEF_SIZE) return; // missing end #
          script_meter=(uint8_t*)calloc(mlen,1);
          if (!script_meter) return;
          tp=script_meter;
          goto next_line;
        }
      }
      else {
        if (*lp=='#') {
          if (*(tp-1)=='|') *(tp-1)=0;
          break;
        }
        if (*lp=='+') {
          // add descriptor +1,1,c,0,10,H20
          //toLogEOL(">>",lp);
          lp++;
          uint8_t index=*lp&7;
          lp+=2;
          if (index<1 || index>meters_used) goto next_line;
          index--;
          uint8_t srcpin=strtol(lp,&lp,10);
          if (Gpio_used(srcpin)) {
            AddLog_P(LOG_LEVEL_INFO, PSTR("gpio double define!"));
            return;
          }
          script_meter_desc[index].srcpin=srcpin;
          if (*lp!=',') goto next_line;
          lp++;
          script_meter_desc[index].type=*lp;
          lp+=2;
          script_meter_desc[index].flag=strtol(lp,&lp,10);
          if (*lp!=',') goto next_line;
          lp++;
          script_meter_desc[index].params=strtol(lp,&lp,10);
          if (*lp!=',') goto next_line;
          lp++;
          script_meter_desc[index].prefix[7]=0;
          for (uint32_t cnt=0; cnt<8; cnt++) {
            if (*lp==SCRIPT_EOL) {
              script_meter_desc[index].prefix[cnt]=0;
              lp--;
              break;
            }
            script_meter_desc[index].prefix[cnt]=*lp++;
          }
          goto next_line;
        }

        if (*lp=='-' || isdigit(*lp)) {
          //toLogEOL(">>",lp);
          // add meters line -1,1-0:1.8.0*255(@10000,H2OIN,cbm,COUNTER,4|
          if (*lp=='-') lp++;
          uint8_t mnum=strtol(lp,0,10);
          if (mnum<1 || mnum>meters_used) goto next_line;
          while (1) {
            if (*lp==SCRIPT_EOL) {
              if (*(tp-1)!='|') *tp++='|';
              goto next_line;
            }
            *tp++=*lp++;
            index++;
            if (index>=METER_DEF_SIZE) break;
          }
        }

      }

next_line:
      if (*lp==SCRIPT_EOL) {
        lp++;
      } else {
        lp = strchr(lp, SCRIPT_EOL);
        if (!lp) break;
        lp++;
      }
    }
    *tp=0;
    meter_desc_p=script_meter_desc;
    meter_p=script_meter;
  }
#endif

  typedef void (*function)();
  function counter_callbacks[] = {SML_CounterUpd1,SML_CounterUpd2,SML_CounterUpd3,SML_CounterUpd4};
  uint8_t cindex=0;
  // preloud counters
  for (byte i = 0; i < MAX_COUNTERS; i++) {
      RtcSettings.pulse_counter[i]=Settings.pulse_counter[i];
      sml_counters[i].sml_cnt_last_ts=millis();
  }
  for (uint8_t meters=0; meters<meters_used; meters++) {
    if (meter_desc_p[meters].type=='c') {
        if (meter_desc_p[meters].flag&2) {
          // analog mode
#ifdef ANALOG_OPTO_SENSOR
          ADS1115_init();
          sml_counters[cindex].ana_max=-32768;
          sml_counters[cindex].ana_min=+32767;
#endif
        } else {
          // counters, set to input with pullup
          if (meter_desc_p[meters].flag&1) {
            pinMode(meter_desc_p[meters].srcpin,INPUT_PULLUP);
          } else {
            pinMode(meter_desc_p[meters].srcpin,INPUT);
          }
          // check for irq mode
          if (meter_desc_p[meters].params<=0) {
            // init irq mode
            attachInterrupt(meter_desc_p[meters].srcpin, counter_callbacks[cindex], FALLING);
            sml_counters[cindex].sml_cnt_old_state=meters;
            sml_counters[cindex].sml_debounce=-meter_desc_p[meters].params;
          }
          InjektCounterValue(meters,RtcSettings.pulse_counter[cindex]);
          cindex++;
        }
    } else {
      // serial input, init
      if (meter_desc_p[meters].srcpin==3) {
        ClaimSerial();
        SetSerialBaudrate(meter_desc_p[meters].params);
      } else {
#ifdef SPECIAL_SS
        meter_ss[meters] = new TasmotaSerial(meter_desc_p[meters].srcpin,-1,0,1);
#else
        meter_ss[meters] = new TasmotaSerial(meter_desc_p[meters].srcpin,-1);
#endif
        if (meter_ss[meters]->begin(meter_desc_p[meters].params)) {
          meter_ss[meters]->flush();
        }
      }
    }
  }

}


void SetDBGLed(uint8_t srcpin, uint8_t ledpin) {
    pinMode(ledpin, OUTPUT);
    if (digitalRead(srcpin)) {
      digitalWrite(ledpin,LOW);
    } else {
      digitalWrite(ledpin,HIGH);
    }
}

// fast counter polling
void SML_Counter_Poll(void) {
uint16_t meters,cindex=0;
uint32_t ctime=millis();

  for (meters=0; meters<meters_used; meters++) {
    if (meter_desc_p[meters].type=='c') {
      // poll for counters and debouce
      if (meter_desc_p[meters].params>0) {
        if (ctime-sml_counters[cindex].sml_cnt_last_ts>meter_desc_p[meters].params) {
          sml_counters[cindex].sml_cnt_last_ts=ctime;

          if (meter_desc_p[meters].flag&2) {
            // analog mode, get next value
#ifdef ANALOG_OPTO_SENSOR
            if (ads1115_up) {
              int16_t val = adc.read_sample();
              if (val>sml_counters[cindex].ana_max) sml_counters[cindex].ana_max=val;
              if (val<sml_counters[cindex].ana_min) sml_counters[cindex].ana_min=val;
              sml_counters[cindex].ana_curr=val;
              int16_t range=sml_counters[cindex].ana_max-sml_counters[cindex].ana_min;
            }
#endif
          } else {
            // poll digital input
            uint8_t state;
            sml_counters[cindex].sml_cnt_debounce<<=1;
            sml_counters[cindex].sml_cnt_debounce|=(digitalRead(meter_desc_p[meters].srcpin)&1)|0x80;
            if (sml_counters[cindex].sml_cnt_debounce==0xc0) {
              // is 1
              state=1;
            } else {
              // is 0, means switch down
              state=0;
            }
            if (sml_counters[cindex].sml_cnt_old_state!=state) {
              // state has changed
              sml_counters[cindex].sml_cnt_old_state=state;
              if (state==0) {
                // inc counter
                RtcSettings.pulse_counter[cindex]++;
                InjektCounterValue(meters,RtcSettings.pulse_counter[cindex]);
              }
            }
          }
        }
#ifdef DEBUG_CNT_LED1
        if (cindex==0) SetDBGLed(meter_desc_p[meters].srcpin,DEBUG_CNT_LED1);
#endif
#ifdef DEBUG_CNT_LED2
        if (cindex==1) SetDBGLed(meter_desc_p[meters].srcpin,DEBUG_CNT_LED2);
#endif
      } else {
        if (ctime-sml_counters[cindex].sml_cnt_last_ts>10) {
          sml_counters[cindex].sml_cnt_last_ts=ctime;
#ifdef DEBUG_CNT_LED1
          if (cindex==0) SetDBGLed(meter_desc_p[meters].srcpin,DEBUG_CNT_LED1);
#endif
#ifdef DEBUG_CNT_LED2
          if (cindex==1) SetDBGLed(meter_desc_p[meters].srcpin,DEBUG_CNT_LED2);
#endif
        }
      }
      cindex++;
    }
  }
}

#ifdef SML_SEND_SEQ
#define SML_SEQ_PERIOD 5
uint8_t sml_seq_cnt;
void SendSeq(void) {
  sml_seq_cnt++;
  if (sml_seq_cnt>SML_SEQ_PERIOD) {
    sml_seq_cnt=0;
    // send sequence every N Seconds
    uint8_t sequence[]={0x2F,0x3F,0x21,0x0D,0x0A,0};
    uint8_t *ucp=sequence;
    while (*ucp) {
      uint8_t iob=*ucp++;
      // for no parity disable next line
      iob|=(CalcEvenParity(iob)<<7);
      Serial.write(iob);
    }
  }
}

// for odd parity init with 1
uint8_t CalcEvenParity(uint8_t data) {
uint8_t parity=0;

  while(data) {
    parity^=(data &1);
    data>>=1;
  }
  return parity;
}
#endif


// dump to log shows serial data on console
// has to be off for normal use
// in console sensor53 d1,d2,d3 .. or. d0 for normal use
// set counter => sensor53 c1 xxxx
// restart driver => sensor53 r

bool XSNS_53_cmd(void) {
  bool serviced = true;
  if (XdrvMailbox.data_len > 0) {
      char *cp=XdrvMailbox.data;
      if (*cp=='d') {
        // set dump mode
        cp++;
        dump2log=atoi(cp);
        ResponseBeginTime();
        ResponseAppend_P(PSTR(",\"SML\":{\"CMD\":\"dump: %d\"}}"),dump2log);
      } else if (*cp=='c') {
          // set ounter
          cp++;
          uint8_t index=*cp&7;
          if (index<1 || index>MAX_COUNTERS) index=1;
          cp++;
          while (*cp==' ') cp++;
          if (isdigit(*cp)) {
            uint32_t cval=atoi(cp);
            while (isdigit(*cp)) cp++;
            RtcSettings.pulse_counter[index-1]=cval;
            uint8_t cindex=0;
            for (uint8_t meters=0; meters<meters_used; meters++) {
              if (meter_desc_p[meters].type=='c') {
                InjektCounterValue(meters,RtcSettings.pulse_counter[cindex]);
                cindex++;
              }
            }
          }
          ResponseBeginTime();
          ResponseAppend_P(PSTR(",\"SML\":{\"CMD\":\"counter%d: %d\"}}"),index,RtcSettings.pulse_counter[index-1]);
      } else if (*cp=='r') {
        // restart
        ResponseBeginTime();
        ResponseAppend_P(PSTR(",\"SML\":{\"CMD\":\"restart\"}}"));
        SML_CounterSaveState();
        SML_Init();
      } else {
        serviced=false;
      }
  }
  return serviced;
}

void InjektCounterValue(uint8_t meter,uint32_t counter) {
  sprintf((char*)&smltbuf[meter][0],"1-0:1.8.0*255(%d)",counter);
  SML_Decode(meter);
}

void SML_CounterSaveState(void) {
  for (byte i = 0; i < MAX_COUNTERS; i++) {
      Settings.pulse_counter[i] = RtcSettings.pulse_counter[i];
  }
}



/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns53(byte function) {
  bool result = false;
    switch (function) {
      case FUNC_INIT:
        SML_Init();
        break;
      case FUNC_LOOP:
        SML_Counter_Poll();
        break;
      case FUNC_EVERY_50_MSECOND:
        if (dump2log) Dump2log();
        else SML_Poll();
        break;
#ifdef SML_SEND_SEQ
      case FUNC_EVERY_SECOND:
        SendSeq();
        break;
#endif
      case FUNC_JSON_APPEND:
        SML_Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        SML_Show(0);
        break;
#endif  // USE_WEBSERVER
      case FUNC_COMMAND_SENSOR:
        if (XSNS_53 == XdrvMailbox.index) {
          result = XSNS_53_cmd();
        }
        break;
      case FUNC_SAVE_BEFORE_RESTART:
      case FUNC_SAVE_AT_MIDNIGHT:
        SML_CounterSaveState();
        break;
    }
  return result;
}

#endif  // USE_SML
