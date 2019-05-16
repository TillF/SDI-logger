//Data logger for logging SDI-sensor data to SD-card with RTC-timestamp
//Till Francke, 2019
//ver 1.20

//see instructions at https://github.com/TillF/SDI-logger


// Begin settings -------------------------------------------------
#define SERIAL_BAUD 9600  // The baud rate for the output serial port (only relevant when conected to computer)

  
//for SD-card
#define chipSelect 10  //pin used by SD-card slot. For the Snootlab-Shield, this is fixed to 10

//further pins
#define wakeUpPin 2 // Interrupt Pin used (should be 2 on UNO, 7 on Pro Micro) 
#define messagePin 3 // (optional) pin for connecting LED indicating messages (UNO: don't use 0 or 1 when connected to USB; Pro Micro: 17)

//time settings
#define INTERVAL 15 //interval between measurements [sec]. Must result in an integer number of intervals per day.
#define AWAKE_TIME 15 //time for being awake before and after actual measurement [sec].

  //start time of reading. All successive readings will be made at multiples of INTERVAL after/before this time
#define HOUR_START 2   
#define MINUTE_START 20
#define SEC_START 0
#define TIMESTAMP_START (HOUR_START*3600 + MINUTE_START*60 + SEC_START) //don't change this

// end settings --------------------------------------------------------

// Begin code section - no user changes required below (sic!)

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

String logfile_name="";

long timestamp_next=0; //timestamp of next reading 
long timestamp_curr; //current timestamp



void setup_sdcard()
{
  Serial.print(F("Init SD card..."));
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    error_message(2, -1);
  }
   
   Serial.println(F("ok."));
  Serial.flush();
}

void setup_clock()
{
  char DateAndTimeString[22]; //19 digits plus the null char
  // Initialize the rtc object
  Serial.print(F("Init clock..."));
  // Start the I2C interface
  Wire.begin();

  now = RTC.now();
  sprintf(DateAndTimeString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
  // Serial.println(DateAndTimeString);
  
  //Serial.println(F("RTC-time: ")+(String)now.year()+"/"+ (String)now.month()+"/"+ (String)now.day()+" "+ (String)now.hour()+":"+ (String)now.minute()+":"+ (String)now.second());
  Serial.print(F("RTC-time: "));
  Serial.println(DateAndTimeString);

  //logfile_name = (String)Clock.getYear()+(String)Clock.getMonth(Century)+(String)Clock.getDate();

  //assemble name of logfile to be created
  sprintf(DateAndTimeString, "%4d%02d%02d", now.year(), now.month(),now.day()); 
  logfile_name = (String)DateAndTimeString+".";
  
  //set file extension from logger ID read from EEPROM
  char logger_id[3] = "L01";  //Variable to store in EEPROM.
  int eeAddress = 0;   //Location we want the data to be put.
  EEPROM.get(eeAddress, logger_id);
  if ( (logger_id[0] < '0') | (logger_id[0] > 'z')) //this doesn't seem to be a proper string
    logfile_name += "log"; else //revert to default
    logfile_name += (String)logger_id;
    
  Serial.print(F("logfile:"));
  Serial.println(logfile_name); 
}

void setup() { //this function is run once on power-up
  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_BAUD);
    while(!Serial);// wait for serial port to connect. Needed for native USB port only

  if (messagePin !=0)  //initialize message LED, if present
  {
    pinMode(messagePin, OUTPUT);
    digitalWrite(messagePin, LOW);
  }  
  setup_clock();
  setup_sdcard();
  setup_sdi();
  setup_imu();
}


String read_sensors()
{
 String output_string;
 //output_string += read_all_SDI();
 output_string += read_imu();
  
 // output_string +=  String(F("\t"))+(String)Clock.getTemp(); //read temperature of RTC: sadly, only works with rinkydinks library, which in turn does not support alarms 
  output_string +=  String(F("\t"))+(String)getVoltage(); //get internal voltage of board, may help detecting brownouts
  //Serial.println("V"+output_string);
 //if (output_string=="") //no data from sensor
 //    error_message(4, 5); //blink LED 4 times, repeat 5 times, then keep going
 return(output_string);
}

 int getVoltage(void) // Returns actual value of Vcc (x 100), i.e. internal voltage to processor (http://forum.arduino.cc/index.php?topic=88935.0)
    {
       
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
     // For mega boards
     const long InternalReferenceVoltage = 1115L;  // Adjust this value to your boards specific internal BG voltage x1000
        // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc reference
        // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)         -Selects channel 30, bandgap voltage, to measure
     ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
 
#else
     // For 168/328 boards
     const long InternalReferenceVoltage = 1056L;  // Adjust this value to your boards specific internal BG voltage x1000
        // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
        // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
     ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
       
#endif
     delay(50);  // Let mux settle a little to get a more stable A/D conversion
        // Start a conversion 
     ADCSRA |= _BV( ADSC );
        // Wait for it to complete
     while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
        // Scale the value
     int results = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // calculates for straight line value
     return results;

    }
  


void blink_led(int times, String msg) //blink message LED, if exists
{
  Serial.println(msg);
  if (messagePin==0) 
    return; //no pin selected
  const long blink_length = 300; //duration of blinks and pauses
  for (int i=0;i < times; i++) 
  {
    digitalWrite(messagePin, HIGH); 
    delay(blink_length);
    digitalWrite(messagePin, LOW); 
    delay(blink_length);
  }  
}

void error_message(byte error_id, int8_t times) //blink and issue message for <time> times. For negative values, blink infinitely
{
  String errors[5] = { "", //short flashes every 2 secs
                     "",
                     "no SD-card found",  //2
                     "SD-card write error", //3
                     "no data from sensor" //4
                      };
  int8_t i;
  while(1)
  {
    blink_led(error_id, errors[error_id] );
    delay(2000);
    i = (i+1) % 120; //increment but prevent rollover to negative values
    if (i== times) break;
  }  
}

long time_next_reading(long &timestamp_curr_i) //compute unix time of next reading
{
  timestamp_curr_i = RTC.now().unixtime();
  //Serial.println((String)timestamp_curr_i);
  long time2nextreading = INTERVAL - ( (timestamp_curr_i - (TIMESTAMP_START)) % INTERVAL);
  return(timestamp_curr_i + time2nextreading);
}


// Those are the ALARM Bits that can be used
#define ALRM1_MATCH_HR_MIN_SEC 0b1000  // when hours, minutes, and seconds match



void reset_alarm_pin()
{
  DS3231 Clock; 
  bool t=Clock.checkIfAlarm(1) & Clock.checkIfAlarm(2); //seems to be necessary to reset alarm pin status
}


void sleep_and_wait()  //idles away time until next reading by a) sleeping (saving energy) and b) waiting (showing signs of life)
{
   timestamp_next = time_next_reading(timestamp_curr); //set time of next reading and update timestamp (side effect)
//  Serial.println(F("Next reading timestamp:")+(String)timestamp_next);
//  Serial.println(F("Current timestamp:")+(String)timestamp_curr);

  
//  char DateAndTimeString[22]; //19 digits plus the null char
 // sprintf(DateAndTimeString, "current time: %4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
 // Serial.println(DateAndTimeString);
  
  long time2next;

  //wait before sleep
  Serial.print(F("Time to wait1:"));
  Serial.println(AWAKE_TIME);
  wait(AWAKE_TIME*1000);
  
  //sleep
  timestamp_curr = RTC.now().unixtime();
  time2next = timestamp_next - timestamp_curr - AWAKE_TIME;  //compute time to spend in sleep mode
  Serial.print(F("Time to sleep:"));
  Serial.println((String)time2next);
  //delay(time2next*1000); 
  sleep(time2next);
  
  //wait after sleep
  timestamp_curr = RTC.now().unixtime();
  //Serial.print(F("Current timestamp:" ));
  //Serial.println(timestamp_curr);
  time2next = (timestamp_next - timestamp_curr);  //compute time to spend in wait mode
  Serial.print(F("Time to wait2:"));
  Serial.println(time2next);
  wait(time2next*1000);
  
}

void wait(long interval) //wait for the requested number of millisec while still showing output/LED-activity at certain times
{
  const int blink_interval = 2000; //blink LED every nn msecs. Should be a multiple of "interval"
  int led_state=HIGH;
  
  for(long i=interval; i >= 0; i -= blink_interval)
  {
    Serial.print(F("reading in "));
    Serial.println(String(i/1000) +" s"); 
    if (messagePin !=0)
    {
      digitalWrite(messagePin, led_state);
      led_state=!led_state;  //invert LED-state
    } 
    delay(blink_interval);
  }
}

void sleep(long time2sleep)
{
  if (time2sleep < 1) return; //no short naps!
  DS3231 Clock; 

  // Set alarm
  Serial.println(F("Setting alarm"));
  reset_alarm_pin(); //otherwise, voltage stays high there

  //compute time of next wake up
  now = RTC.now(); //get current time
  byte secs =now.second(); //extract components
  byte mins =now.minute();
  byte hours=now.hour();
 Serial.print(F("current time:"));
 Serial.println((String)hours+":"+(String)mins+":"+(String)secs);

  long tstart = (long)hours*3600 + (long)mins * 60 + secs; //convert start time as timestamp
  long tend = (long)hours*3600 + (long)mins * 60 + secs + time2sleep; //compute end time as timestamp

//convert timestamp into time (day disregarded)
secs = tend % 60;
tend /= 60;              
mins = tend % 60;
tend /= 60;
hours = tend % 24;

  Serial.print(F("alarm to set:"));
  Serial.println((String)hours+":"+(String)mins+":"+(String)secs);

   // This is the interesting part which sets the AlarmBits and configures, when the Alarm be triggered
  byte ALRM1_SET = ALRM1_MATCH_HR_MIN_SEC; // trigger A1 when hours, minute and second match

  // combine the AlarmBits (only Alarm 1 used here)
  int ALARM_BITS = 0;
  ALARM_BITS <<= 4;
  ALARM_BITS |= ALRM1_SET;
  
  // Trigger Alarm when Minute == 30 or 0
  // Clock.setA1Time(Day, Hour, Minute, Second, AlarmBits, DayOfWeek, 12 hour mode, PM)
  Clock.setA1Time(10, hours, mins, secs, ALARM_BITS, false, false, false);  //the day doesn't matter here
 
  // Turn on Alarm
  Clock.turnOnAlarm(1);
  //Clock.turnOffAlarm(2);
  
  //Serial.println(ALARM_BITS,BIN);
  //Serial.print(F("Status Alarm 1:")); Serial.println(Clock.checkAlarmEnabled(1));
  //Serial.println(F("Status Alarm 2:")+(String)Clock.checkAlarmEnabled(2));
  
  // sleep
  Serial.println(F("going 2 sleep"));
  Serial.flush();
  digitalWrite(messagePin, 0);
  delay(500);
  

  noInterrupts ();          // make sure we don't get interrupted before we sleep  
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  //go to sleep
  Serial.println(F("Woken")); //rr
  Serial.flush();
}

void wakeUp() {
  detachInterrupt(digitalPinToInterrupt(wakeUpPin)); //prevent multiple invocations of interrupt 
  digitalWrite(messagePin, 1);
}

void loop() { //this function is called repeatedly as long as the arduino is running
  //Serial.println("6");Serial.flush(); 
 
  char DateAndTimeString[22]; //19 digits plus the null char
  
  // make a string for assembling the data to log:
  String output_string = "";
  
// assemble timestamp 

  now = RTC.now();
 //Serial.println("7");Serial.flush(); 
 
 sprintf(DateAndTimeString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
  output_string = (String)DateAndTimeString;
 
  output_string += read_sensors(); //measure and read data from sensor 
  
  Serial.print(F("string to log:"));Serial.println((String)output_string); 
 
 File dataFile = SD.open(logfile_name, FILE_WRITE);

 
  // if the file is available, write to it:
  if (dataFile) 
  {
    dataFile.println(output_string);
    dataFile.close();
    // print to the serial port too:
    Serial.print(F("logged:"));
    Serial.println(output_string);
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.print(F("error opening "));
    //Serial.println(logfile_name);
    error_message(3, -1); //blink LED 3 times
  }
    timestamp_next = time_next_reading(timestamp_curr);

  Serial.println(F("preparing sleep and wait..."));
  sleep_and_wait(); 
  
}
