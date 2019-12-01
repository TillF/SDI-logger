# SDI-logger
Arduino-based data logger for logging SDI-sensor data to SD-card with RTC-time stamp.

## Features
- requires low-cost datahardware (starting from 15 € [2019], excluding power supply, SD-card and casing)
- data logging to SD-card
- use of low-drift clock (D3231)
- supports reading of multiple SDI-sensors
- optional sleep mode to save power
- optional support of message LED for indicating logger status
- optional assignment of unique logger-ID for managing multiple loggers
- optional logging of (pseudo-)voltage at controler for brownout-detection


## Usage
- install hardware required (see "Hardware required" and "Wiring")
- install libraries (see "Software required")
- verify/set clock using separate script (e.g. ```set_clock.ino```)
- verify/set logger-id using separate script (e.g. ```set_id.ino```)
- adjust settings (see section "Settings" in ```SDI-logger.ino```)

## Hardware required
- tested with Arduino Uno, rev. 3
- D3231 real time clock, SD-card slot (or both combined in a Shield, e.g. https:snootlab.com/lang-en/shields-snootlab/1233-memoire-20-ds3231-fr.html [no longer produced] or Keyes Date logging Shield [reconstruction steps below])

for wiring details, see below

## Software required:
Please install additional libraries via "Tools" -> "Manage Libraries" or download and extract libraries to c:\Program Files (x86)\Arduino\libraries\

 SDI: https://github.com/EnviroDIY/Arduino-SDI-12 (tested: 29 Oct 2018)
 
 RTC: https://github.com/NorthernWidget/DS3231 (tested: 31 Oct 2018) delete/rename any existing library to avoid conflicts
 
 sleep mode: https://github.com/rocketscream/Low-Power (tested: V1.8)

## Wiring

### RTC (when using Shield, only SWQ (between battery and SD-card slot) required)

 SD-card-pin ->  Arduino-pin
 
 Vin -> 5 V
 
 GND -> GND
 
 SCL -> Analog 5
 
 SDA -> Analog 4
 
 SQW -> pin (default: 2 on Uno, 7 on Pro Micro)  (optional, required only when using sleep mode)


### SD card (no wiring required when using Shield)

 SD-card-pin ->  Arduino-pin
 
 MOSI -> pin D11
 
 MISO -> pin D12
 
 CLK (or SLK) -> pin D13
 
 CS -> pin (default: D10 )
 

### SDI-12 (multiple SDI-12 devices supported but not yet tested)

 SDI-device ->  Arduino-pin 
 
 GND -> GND
 
 +Vbat -> 5 V (if the external device needs more than 5 V, connect it to external battery instead of the Arduino-pin)
 
 SDI-12 Data -> pin (default: D7) 
 
 e.g. 
 
 Truebner SMC: data: green; ground: white; V+: brown
 
 delta_T PR2: data: black; ground: blue; V+: white (needs external 12 V)
 

### message LED (optional)

  Connect LED with appropriate resistor to pin 3 (default) and ground.
  
  The LED  can issue the following codes:
  
  off: sleep mode
  
  slow (1 s) blinking: wating for sensor reading
  
  2 short flashes every 2 secs: no SD-card found
  
  3 short flashes every 2 secs: SD-card write error
  
  4 short flashes every 2 secs: no data from sensor
  
![Example](doc/wiring_shield.JPG)
An Arduino Uno equipped with a Snootlab Shield. Pin D2 has been connected to a wire soldered to SQW on the shield. The blue LED of the shield in the lower right (actually a power LED) has been re-configured to serve as the message LED: The left side of the adjacent resistor was soldered off the shield and instead connected to the wire leading to D3. 

Both wires still use the connector plugs, but could likewise also be soldered directly to the contact line on the shield.

## FAQs
### Arduino gets stuck after message "going 2 sleep"
- Unfortunately, sleep mode is still experimental and buggy. Disable it by setting AWAKE_TIME to a value greater than INTERVAL to disable sleeping.

### Message "SD-card write error"
Appears erratically. Try:
- restart Arduino

- remove and reinsert SD-card (check write-protection), then restart Arduino

- format SD-card

- check data of RTC: if the date is wrong, an illegal filename is generated 

## Preparation / alterations to Keyes Date logging Shield for Arduino Uno for Data Logging

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

