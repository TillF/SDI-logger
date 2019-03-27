# SDI-logger
Data logger for logging SDI-sensor data to SD-card with RTC-time stamp
(detailed instructions in header of SDI-logger.ino)

**Usage:
- install hardware required (see "Hardware required" and "Wiring")
- install libraries (see "Software required")
- verify/set clock using separate script (e.g. set_clock.ino)
- verify/set logger-id using separate script (e.g. set_id.ino)
- adjust settings (see section "Settings" below)

**Hardware required:
D3231 real time clock, SD-card slot (or both combined in Shield, e.g. https:snootlab.com/lang-en/shields-snootlab/1233-memoire-20-ds3231-fr.html)
for wiring details, see below

**Software required:
Please install additional libraries via "Tools" -> "Manage Libraries" or download and extract libraries to c:\Program Files (x86)\Arduino\libraries\
 SDI: The library is available at: https:github.com/EnviroDIY/Arduino-SDI-12
 RTC: DS3231 https:github.com/NorthernWidget/DS3231; delete any existing library
 sleep mode: https:github.com/rocketscream/Low-Power V1.8
