//Data logger for logging SDI-sensor data to SD-card with RTC-timestamp
//Till Francke, 2021

//see instructions at https://github.com/TillF/SDI-logger
#define ver_string F("1.39")
#define branch F("master")
#include "setup_general.h" //adjust your board settings in this file
#include "misc_functions.h" //general functions
#include "setup_sdi.h"     //include this if you want to use SDI-12-devices

// Begin code section - no user changes required below (sic!)
char logfile_name[12]="";

long timestamp_next=0; //timestamp of next reading 
long timestamp_curr; //current timestamp



void setup_sdcard(byte repeat)
{
  Serial.print(F("Init SD card..."));
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    error_message(2, repeat);
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
  
  sprintf(logfile_name, "%4d%02d%02d.", now.year(), now.month(),now.day()); //generate name of logfile
  //return (DateAndTimeString); 
}

void setup_logfile(String headerstr)
{
  //assemble name of logfile to be created
   //logfile_name = DateAndTimeString+".";
  //sprintf(logfile_name, "%s.", DateAndTimeString);
  //Serial.println(logfile_name); 
  
  //set file extension from logger ID read from EEPROM
  char logger_id[3];  //Variable to store contents from EEPROM.
  int eeAddress = 0;   //Location we want the data from
  EEPROM.get(eeAddress, logger_id);
  if ( (logger_id[0] < '0') | (logger_id[0] > 'z')) //this doesn't seem to be a proper string
    //logfile_name += "log"; else //revert to default
    sprintf(logfile_name, "%slog",logfile_name); else //revert to default
   //logfile_name += (String)logger_id;
    sprintf(logfile_name, "%s%s",logfile_name, logger_id); 
    
  Serial.print(F("logfile:"));
  Serial.println(logfile_name); 
  
  //write header line for output file
    
  File dataFile = SD.open(logfile_name, FILE_WRITE);
  if (!dataFile) 
     error_message(3, -1); //blink LED 3 times
  dataFile.print("#"); //open first line
  dataFile.print(ver_string); //write SDI script version to top of output file
  dataFile.print(branch);     //write SDI script branch to top of output file
  dataFile.print(": "+headerstr); //write SDI sensor ID to top of output file
  dataFile.println(""); //add line break to header line
  dataFile.close();
}

void setup() { //this function is run once on power-up
  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_BAUD);
    //while(!Serial);// wait for serial port to connect. Needed for native USB port only

  if (messagePin !=0)  //initialize message LED to ON during Init, if present
  {
    pinMode(messagePin, OUTPUT);
    digitalWrite(messagePin, HIGH);
  }  

  Serial.println(ver_string);
  if (wakeUpPin !=0)  //wakeUpPin pin, if present
  {
    pinMode(wakeUpPin, INPUT_PULLUP);   //the RTC then will draw this pin from high to low to denote an event
  }  

  String tmp_str2; //holds IDs of devices
             setup_clock();
             setup_sdcard(-1);
  tmp_str2 = setup_sdi(); //setup SDI devices and retrieve their names
             setup_logfile(tmp_str2);

  if (messagePin !=0)  //switch off message LED
    digitalWrite(messagePin, LOW);
           
}


void read_sensors(File dataFile)
{
  //digitalWrite(messagePin, HIGH); //enable message LED to indicate reading of sensors
  
  int char_counter = read_all_SDI(dataFile); //read SDI and write to file
 if (char_counter==0) //no data from sensor
     error_message(4, 5); //blink LED 4 times, repeat 5 times, then keep going
  
 // output_string +=  String(F("\t"))+(String)Clock.getTemp(); //read temperature of RTC: sadly, only works with rinkydinks library, which in turn does not support alarms 
  getVoltage(dataFile); //get internal voltage of board, may help detecting brownouts
  //Serial.println("\t"+output_string);
 sensor_power(LOW); //power off the sensors, if enabled
 digitalWrite(messagePin, LOW); //disable message LED to indicate end of reading of sensors

}

 void getVoltage(File dataFile) // Returns actual value of Vcc (x 100), i.e. internal voltage to processor (http://forum.arduino.cc/index.php?topic=88935.0)
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
     int results2 = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // calculates for straight line value
     
     String result = "\t"+(String)results2; //add field separator
     Serial.print(result); //write results to console
     dataFile.print(result); // write to file
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
  //Serial.print(F("Time2wait1:"));
  //Serial.println(awake_time_current);
  //wait(awake_time_current);
  
  //sleep
  timestamp_curr = RTC.now().unixtime();
  time2next = timestamp_next - timestamp_curr - AWAKE_TIME;  //compute time to spend in sleep mode
  Serial.print(F("Time2sleep:"));
  Serial.println((String)time2next);
  //delay(time2next*1000); 
  blink_led(8, 33); //flicker LED twice to indicate going to sleep
  delay(300);
  blink_led(8, 33); 
  
  sleep(time2next);
  
  //wait after sleep
  sensor_power(HIGH); //power on the sensors, if enabled
  timestamp_curr = RTC.now().unixtime(); //we check the clock, as we may have woken up by mistake
  //Serial.print(F("Current timestamp:" ));
  //Serial.println(timestamp_curr);
  time2next = (timestamp_next - timestamp_curr);  //compute time to spend in wait mode
  Serial.print(F("Time2wait2:"));
  Serial.println(time2next);
  wait(max(AWAKE_TIME, time2next)); //wait at least the required waiting time
  
}

/*
void wait(long interval) //wait for the requested number of secs while still showing output/LED-activity at certain times
{
  const int blink_interval = 1000; //blink LED every nn msecs. Should be a multiple of "interval"
  int led_state=HIGH;
  
  for(long i=interval*1000; i >= 0; i -= blink_interval)
  {
    Serial.print(F("wait for "));
    Serial.println(String(i/1000) +" s"); 
    if (messagePin !=0)
    {
      digitalWrite(messagePin, led_state);
      led_state = !led_state;  //invert LED-state
    } 
    delay(blink_interval);
  }
}
*/

void sleep(long time2sleep)
{
  //byte t=LOW; //rr
  digitalWrite(messagePin, LOW); //switch off light while sleeping
  if (time2sleep < 1) return; //no short naps!
  DS3231 Clock; 

  // Set alarm
  //Serial.println(F("Setting alarm"));
  pinMode(wakeUpPin, INPUT_PULLUP);   //the RTC then will draw this pin from high to low to denote an event
  reset_alarm_pin(); //otherwise, voltage stays low there
  delay(10);
   
  //compute time of next wake up
  now = RTC.now(); //get current time
  byte secs =now.second(); //extract components
  byte mins =now.minute();
  byte hours=now.hour();
 Serial.print(F("cur time:"));
 Serial.println((String)hours+":"+(String)mins+":"+(String)secs);

//  long tstart = (long)hours*3600 + (long)mins * 60 + secs; //convert start time as timestamp
  long tend = (long)hours*3600 + (long)mins * 60 + secs + time2sleep; //compute end time as timestamp

//convert timestamp into time (day disregarded)
secs = tend % 60;
tend /= 60;              
mins = tend % 60;
tend /= 60;
hours = tend % 24;

  //Serial.print(F("alarm to set:"));
  //Serial.println((String)hours+":"+(String)mins+":"+(String)secs);

   // This is the interesting part which sets the AlarmBits and configures, when the Alarm be triggered
  byte ALRM1_SET = ALRM1_MATCH_HR_MIN_SEC; // trigger A1 when hours, minute and second match

  // combine the AlarmBits (only Alarm 1 used here)
  int ALARM_BITS = 0;
  //ALARM_BITS <<= 4; //for combining both alarm1 and alarm 2
  ALARM_BITS |= ALRM1_SET;
  
  //Serial.print(F("Status Alarm 1:")); Serial.println(Clock.checkAlarmEnabled(1));
  
  // Trigger Alarm when Minute == 30 or 0
  // Clock.setA1Time(Day, Hour, Minute, Second, AlarmBits, DayOfWeek, 12 hour mode, PM)
  Clock.setA1Time(10, hours, mins, secs, ALARM_BITS, false, false, false);  //the day doesn't matter here
 
  // Turn on Alarm
  Clock.turnOnAlarm(1);
  //Serial.print(F("Status Alarm 1:")); Serial.println(Clock.checkAlarmEnabled(1));
  //Serial.println(F("Status Alarm 2:")+(String)Clock.checkAlarmEnabled(2));
  
  // sleep
  Serial.println(F("going 2 sleep"));
  //adjust pin states
  digitalWrite(messagePin, LOW); //switch off LED
      
  Serial.flush();
  delay(50);
 
  noInterrupts ();          // make sure we don't get interrupted before we sleep  
  EIFR = bit (digitalPinToInterrupt(wakeUpPin));  // clear flag for interrupt (deletes any pending interrupt calls) 
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);
  //Serial.flush();
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  //go to sleep
  
  Serial.println(F("Woken")); //rr
  Serial.flush();
}

void wakeUp() {
  detachInterrupt(digitalPinToInterrupt(wakeUpPin)); //prevent multiple invocations of interrupt 
  digitalWrite(messagePin, HIGH);
}

void loop() { //this function is called repeatedly as long as the arduino is running
  
  setup_sdcard(3); //re-initialize the SD-card in case it has been removed
  File dataFile = SD.open(logfile_name, FILE_WRITE);
 
  // if the file is available, write to it:
  if (dataFile) 
  {
    char DateAndTimeString[22]; //19 digits plus the null char
    // assemble timestamp 
    now = RTC.now();
   //Serial.println("7");Serial.flush(); 

   sprintf(DateAndTimeString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
  
   Serial.print(F("logged:"));
   dataFile.print(DateAndTimeString); //write to file
   Serial.print(DateAndTimeString);
   
   read_sensors(dataFile); //measure and read data from sensor 
   dataFile.println(); //next line in file
   dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.print(F("error opening "));
    //Serial.println(logfile_name);
    error_message(3, 3); //blink LED 3 times, repeat 3 times
  }
    timestamp_next = time_next_reading(timestamp_curr);

  Serial.println(F("wait&sleep.."));
  sleep_and_wait(); 
  
}
