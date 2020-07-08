//general/board settings
// Begin settings -------------------------------------------------
#define SERIAL_BAUD 9600  // The baud rate for the output serial port (only relevant when connected to computer)

//time settings
#define INTERVAL 60*1 //interval between measurements [sec]. Must result in an integer number of intervals per day.
#define AWAKE_TIME 3 //time for being awake before and after actual measurement [sec].

//start time of reading. All successive readings will be made at multiples of INTERVAL after/before this time
#define HOUR_START 0   
#define MINUTE_START 20
#define SEC_START 0
#define TIMESTAMP_START (HOUR_START*3600 + MINUTE_START*60 + SEC_START) //don't change this

//list of Arduino boards for easier access of the configurations
#define uno 1
#define promicro 2
#define nano 3
#define board nano //choose "uno", "promicro" or "nano"

#define chipSelect 10  //pin used by SD-card slot. default: 10
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN

// pin settings
#if board==uno | board==nano 
  //for SD-card
  #define wakeUpPin 2 // Interrupt Pin used by RTC (should be 2 on UNO, 7 on Pro Micro) 
  #define messagePin 3 // (optional) pin for connecting LED indicating messages (UNO: 3, don't use 0 or 1 when connected to USB; Pro Micro: 17)
#endif

#if board==promicro 
  #define wakeUpPin 7 // Interrupt Pin used  by RTC (should be 2 on UNO, 7 on Pro Micro) 
  #define messagePin 17 // (optional) pin for connecting LED indicating messages (UNO: 3, don't use 0 or 1 when connected to USB; Pro Micro: 17)
#endif

// end settings --------------------------------------------------------

//for SD-card
#include <SPI.h>
#include <SD.h>

// clock
#include <DS3231.h> // https://github.com/NorthernWidget/DS3231; delete any existing "DS3231"-library to avoid conflicts!!
#include <Wire.h>
RTClib RTC;
DateTime now;

//sleep mode
#include <LowPower.h> // https://github.com/rocketscream/Low-Power V1.8

//for reading logger ID from EEPROM
#include <EEPROM.h>
