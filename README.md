# SDI-logger
Arduino-based data logger for logging SDI-12 (and other) sensor data to SD-card with RTC-time stamp.

[ALog](https://github.com/NorthernWidget/ALog) may be a more advanced alternative you could also check out.

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
- install hardware (see "Hardware required" and "Wiring")
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
- upload scripts to Arduino and run!	

## Hardware required<a name="hardware"> </a> 
- Arduino (tested with Arduino Uno-rev. 3, Pro Micro, Nano)
- D3231 real time clock, SD-card slot (or both combined in a Shield (currently none available) or customize a Shield, e.g. Keyes Date logging Shield [reconstruction steps below])

for wiring details, see below

## Software required <a name="software"> </a> 
[Arduino IDE](https://www.arduino.cc/en/software)

Please install additional libraries via "Tools" -> "Manage Libraries" or download and extract libraries to the IDE-installation directory, e.g. "c:\Program Files (x86)\Arduino\libraries\"

 SDI: https://github.com/EnviroDIY/Arduino-SDI-12 (tested: 29 Oct 2018)
 
 RTC: https://github.com/NorthernWidget/DS3231 (tested: 31 Oct 2018) delete/rename any existing library to avoid conflicts (Version 1.0.2 and 1.0.7 are working, but 1.1.0 does not work!)
 
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
 
 e.g. 
 
 Truebner SMT100: data: green; ground: white; V+: brown
 
 delta_T PR2: data: black; ground: blue; V+: white (needs external 12 V)
 
 \* if the external device needs more than 5 V or exceeds the current that can be provided by the Arduino, connect it to external battery instead of the Arduino-pin).
 
 ** if you want to switch off the power between measurements, use the pin specified as POWER_PIN
 

### Message LED (optional)

 Uno: Connect LED with appropriate resistor to ```message_pin``` (default: pin 3) and ground (internal LED cannot be used).
 
 Pro Micro:  use internal LED.
 
 Shield: Connect appropriate pin to LED on shield ()
  
  The LED can issue the following codes:
  
  off: sleep mode or everything is broken

  on: power-up at initialisation

  slow (1 s) blinking: waiting
  
  medium (0.3 s):
  2 flashes: no SD-card found
  
  3 flashes: SD-card write error
  
  4 flashes: no data from sensor

  flicker (0.033 s):
  1 s burst: reading from sensor
  2 x 0.5 s bursts: going to sleep
  
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

- use different SD card: even without apparent errors, there seem to be difference

### SDI-12 device not returning any or corrupted data <a name="SDI12_length"> </a> 
SDI-12-devices which return long data strings (e.g. many values returned in one measurement) may cause memory issues. These can cause corrupted or no data being returned or other unexpected behaviour. As of ver. 1.31, return strings of at least 72 chars per device seem to be ok.

### SDI-12 device not returning any or zero data 
When using power switching (POWER_PIN), the warmup time may be to short. Increase AWAKE_TIME. The Nano also seems to be affected by low temperatures: switching via POWER_PIN showed only a slow rise to the desired output voltage. Again, increasing the waiting time or using a relais may help.

### Logger recording at irregular intervals, irrespective of interval settings 
We found this behaviour with one particular computer and IDE. When uploading the same scripts from another computer, everything worked correctly. Re-installing the IDE also solved this problem.

### Power consumption
Different Arduino boards vary greatly in power consumption, even within the same type of Arduino. Furthermore, attached peripherals have an influence, again, varying between manufacturers.
Generally, the Uno is not the best choice for power saving, but even the Uno can be tweaked to considerably lower consumption with some soldering (disable LEDs and USB-chip, see [these instructions ](https://www.defproc.co.uk/tutorial/how-to-reduce-arduino-uno-power-usage-by-95/)).
To get some general idea, here are some of our measurements:
| Arduino    | awake [mA] | reading² [mA] | asleep [mA]  | shield | peripherals        | connected via | voltage [V] |
| :--------- | :--:       | :-------:    | :---------:  | :----: | :--------:         | :-----------: | ------------:
| UNO        | 55         | ?            | 37           | no     | none               | coax jack     | 12
| Pro Micro  | 11         | ?            | 1.4          | no     | none               | Vcc           | 5
| Nano       | 41         | 74           | 35           | no     | 12xSMT100, on      | +5V pin*      | 5
| Nano       | 19         | 24           | 8            | no     | none               | +5V pin*      | 5
| Nano       | 28         | 64           | 22           | no     | 12xSMT100, on      | Vin pin       | 5
| Nano       | 13         | 16           | 6            | no     | none               | Vin pin       | 5
| Nano       | 19         | 74           | 8            | no     | 12xSMT100, switched | +5V pin*     | 5
| Nano       | 13         | 74           | 6            | no     | 12xSMT100, switched | Vin pin*     | 5
| Nano, RTC1 | ?          | ~50          | 7.2          | no     | 12xSMT100, switched | Vin pin      | 5
| Nano, RTC2 | ?          | ~50          | 10.6         | no     | 12xSMT100, switched | Vin pin      | 5
| Nano, RTC2 LED off | ?  | ~50          | 8.1          | no     | 12xSMT100, switched | Vin pin      | 5
| Nano  RTC2  | ?         | ~62          | 11.5         | yes    | 12xSMT100, switched | Vin pin      | 5
| Nano, RTC2 LED off | ?  | ~62          | 10.3         | yes    |12xSMT100, switched | Vin pin      | 5
| Nano, RTC1 |         ?  | ~62          | 9.7          | yes    |12xSMT100, switched | Vin pin      | 5
| Nano, RTC1 |         ?  | ~62          | 7.0          | yes, LEDs off    |12xSMT100, switched | Vin pin      | 5
| Nano LEDs off, RTC1 |?  | ~62          | 4.2          | yes, LEDs off    |12xSMT100, switched | Vin pin      | 5
| Nano2 LEDs off, RTC2 |? | ~62          | 18.8         | yes, LEDs off    |12xSMT100, switched | Vin pin      | 5
| Nano2 LEDs, USB off, RTC2 |? | ~62     | 8.8          | yes, LEDs off    |12xSMT100, switched | Vin pin      | 5
| Nano3** LEDs off, RTC2 | 33 | 50	 | 7.4		| yes, LEDs off	   |12xSMT100, switched	| Vin pin	| 5
| UNO³       | ?          | ?            | 25           | yes               | PR2               | coax jack    | 12
| UNO³, LEDs off | ?          | ?        | 18.8     | yes, LEDs off     | PR2               | coax jack    | 12
| UNO³, LEDs, USB off | ?          | ?   | 7.8     | yes, LEDs off     | PR2               | coax jack    | 12

² highly fluctuating

³ manufacturer: Himalaya

\* strange, but true: the Nano drew more current when powered via +5V pin than via Vin.

\** manufacturer TRUcomponents, ATMEGA328


## Tweaking shields <a name="tweaking"> </a> 
Customizing shields serves to accommodate a better clock chip and to save energy.

### Preparation / alterations to Keyes Date logging Shield for Arduino Uno

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
- for even more power savings: [disable USB](https://www.defproc.co.uk/tutorial/how-to-reduce-arduino-uno-power-usage-by-95/)).

### Preparation / alterations to Deek-Robot Data Logging Shield for Arduino
- ...
