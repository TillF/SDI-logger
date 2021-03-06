# SDI-logger
Arduino-based data logger for logging SDI-12 (and other) sensor data to SD-card with RTC-time stamp. [ALog](https://github.com/NorthernWidget/ALog) may be a more advanced alternative you could also check out.

Contents:

[Features](#features)

[Usage](#usage)

[Hardware required](#hardware)

[Software required](#software)

[Wiring](#wiring)

[FAQs](#faqs)

[Tweaking Shields](#tweaking)


## Features <a name="features"> </a> 
- requires low-cost hardware (starting from 10 € [2019], excluding power supply, SD-card and casing)
- data logging to SD-card
- use of low-drift clock (D3231)
- supports reading/logging of 
	- multiple SDI-sensors
	- (pseudo-)voltage at controler for brownout-detection
	- event (e.g. reed contact)
	- IMU (magnetometer, gyrometer)
- optional sleep mode to save power
- optional support of message LED for indicating logger status
- optional assignment of unique logger-ID for managing multiple loggers
- optional logging of (pseudo-)voltage at controler for brownout-detection


## Usage <a name="usage"> </a> 
- install hardware required (see "Hardware required" and "Wiring")
- install libraries (see "Software required")
- verify/set clock using separate script (e.g. ```set_clock.ino```)
- verify/set logger-id using separate script (e.g. ```set_id.ino```)
- adjust settings 
	- ```setup_general.h``` (general settings)
	- ```setup_sdi.h``` (when using SDI-12 devices)
	- ```setup_event.h``` (when using the event logger)
	- ```setup_imu.h``` (when using IMU)
- enable/disable required header files in ```SDI-logger.ino```
- enable/disable reading of required sensors in ```read_sensors()``` in ```SDI-logger.ino```
- upload and run!	

## Hardware required<a name="hardware"> </a> 
- tested with Arduino Uno, rev. 3, Pro Micro, Nano (sleep mode not tested)
- D3231 real time clock, SD-card slot (or both combined in a Shield, e.g. https:snootlab.com/lang-en/shields-snootlab/1233-memoire-20-ds3231-fr.html [no longer produced] or Keyes Date logging Shield [reconstruction steps below])

for wiring details, see below

## Software required <a name="software"> </a> 
Please install additional libraries via "Tools" -> "Manage Libraries" or download and extract libraries to c:\Program Files (x86)\Arduino\libraries\

 SDI: https://github.com/EnviroDIY/Arduino-SDI-12 (tested: 29 Oct 2018)
 
 RTC: https://github.com/NorthernWidget/DS3231 (tested: 31 Oct 2018) delete/rename any existing library to avoid conflicts
 
 sleep mode: https://github.com/rocketscream/Low-Power (tested: V1.81)

## Wiring <a name="wiring"> </a> 

### RTC (when using Shield, only SWQ (between battery and SD-card slot) required)

| SD-card-pin | Ardu. Uno | Pro Micro | Nano |
| :---------- | :-------: | --------: |----: |
| Vin         | 5 V       | Vcc       | 5 V  |
| GND         | GND       | GND       | GND  |     
| SCL         | A 5       | A 3       | A 5  |
| SDA         | A 4       | A 2       | A 4  |
| SQW         | D 2       | D 7       | D 2  |

 (SQW optional, required only when using sleep mode)

### SD-card (no wiring required when using Shield)

| SD-card-pin | Uno, Nano | Pro Micro |
| :---------- | :-------:  | --------: |
| MOSI        | D 11       | D 16      |
| MISO        | D 12       | D 14      |     
| CLK (or SLK)| D 13       | D 15      |
| CS          | D 10*      | D 10*     |
 
*pin "chipSelect"(default: D10 ), change this to match your SD shield or module:
* Arduino Ethernet shield: pin 4
* Adafruit SD shields and modules: pin 10
* Sparkfun SD shield: pin 8
* MKRZero SD: SDCARD_SS_PIN

 
### SDI-12 (multiple SDI-12 devices supported)

 For devices returning long data strings, please note [this advice](#SDI12_length).
 
| SDI-device  | Uno, Nano | Pro Micro |
| :---------- | :-------: | --------: |
| GND         | GND       | GND       |
| +Vbat \*    | 5 V  **   | 5 V **    |    
| SDI-12 data | D 7       | D 8       |    
 
 \* if the external device needs more than 5 V or exceeds the current that can be provided by the Arduino, connect it to external battery instead of the Arduino-pin).
 
 ** if you want to switch off the power inbetween measurements, use the pin specified as POWER_PIN
 
 e.g. 
 
 Truebner SMT100: data: green; ground: white; V+: brown
 
 delta_T PR2: data: black; ground: blue; V+: white (needs external 12 V)
 

### message LED (optional)

 Uno: Connect LED with appropriate resistor to ```message_pin``` (default: pin 3) and ground (internal LED cannot be used).
  Pro Micro:  use internal LED.
  
  The LED  can issue the following codes:
  
  off: sleep mode
  
  slow (1 s) blinking: wating for sensor reading
  
  2 short flashes every 2 secs: no SD-card found
  
  3 short flashes every 2 secs: SD-card write error
  
  4 short flashes every 2 secs: no data from sensor
  
![Example](doc/wiring_shield.JPG)
An Arduino Uno equipped with a Snootlab Shield. Pin D2 has been connected to a wire soldered to SQW on the shield. The blue LED of the shield in the lower right (actually a power LED) has been re-configured to serve as the message LED: The left side of the adjacent resistor was soldered off the shield and instead connected to the wire leading to D3. 

Both wires still use the connector plugs, but could likewise also be soldered directly to the contact line on the shield.

## FAQs <a name="faqs"> </a> 
### Arduino gets stuck after message "going 2 sleep"
- Some boards (e.g. Pro Micro) do no reconnect USB after sleep mode. The logger may still be running fine. Verify this by checking the status LED or the records on the SD-card.

- Unfortunately, sleep mode is not supported by all chips. Disable it by setting AWAKE_TIME to a value greater than INTERVAL to disable sleeping.

### Message "SD-card write error"
Appears erratically. Try:
- restart Arduino

- remove and reinsert SD-card (check write-protection), then restart Arduino

- format SD-card

- check data of RTC: if the date is wrong, an invalid filename is generated 

- check if the voltage of the SD-card module is sufficient (it may require 5 instead of 3 V)

- upload the Arduino sketch with another computer - once, this really fixed the problem for us

### Power consumption 
| Arduino    | awake [mA] | reading [mA] | asleep [mA]  | peripherals        | connected via |
| :--------- | :--:       | :-------:    | :---------:  | :--------:         | ------------: |
| UNO        | 55         | ?            | 37           | none               | coax jack     |
| Pro Micro  | 11         | ?            | 1.4          | none               | Vcc           |
| Nano       | 41         | 74           | 35           | 12xSMT100, on      | +5V pin*      |
| Nano       | 19         | 24           | 8            | none               | +5V pin*      |
| Nano       | 28         | 64           | 22           | 12xSMT100, on      | Vin pin       |
| Nano       | 13         | 16           | 6            | none               | Vin pin       |
| Nano       | 19         | 74           | 8            | 12xSMT100, switched | +5V pin*     |
| Nano       | 13         | 74           | 6            | 12xSMT100, switched | Vin pin*     |

\* strange, but true: the Nano drew more current when powered via +5V pin than via Vin.

### SDI-12 device not returning any or corrupted data <a name="SDI12_length"> </a> 
SDI-12-devices which return long data strings (e.g. many values returned in one measurement) may cause memory issues. These can cause corrupted or no data being returned or other unexpected behaviour. As of ver. 1.31, return strings of at least 72 chars per device seem to be ok.

### SDI-12 device not returning any or zero data 
When using power switching (POWER_PIN), the warmup time may be to short. Increase AWAKE_TIME.

### Logger recording at irregular intervals, irrespective of interval settings 
We found this behaviour with one particular computer and IDE. When uploading the same scripts from another computer, everything worked correctly. Re-installing the IDE also solved this problem.

## Tweaking shields <a name="tweaking"> </a> 
### Preparation / alterations to Keyes Date logging Shield for Arduino Uno for Data Logging

- aim: replace poor DS1307 RTC with DS3231, use power LED as message LED

- Disconnect / scratch conductor paths
	- between "+" of Li-cell and adjacent unneeded RTC-chip (8 legs)
	- between resistor next to power LED and capacitor (side of analog pins)
	- on backside: L-shaped track from 5-V-pin to unneeded RTC
	
- solder D3 to resistor next to power LED (side of analog pins) (blue)
- RTC
		- remove Vcc-pin
		- solder cable to outer battery pin (+) (green)
		- solder to holes next to power LED
- solder cable to + of Li-cell (green)
- solder cable to VCC of RTC and 5V-hole next to 3V3 hole  (green)
- solder D2 to third pin of RTC, counting from C1 (blue)

