//Data logger for logging SDI-sensor data to SD-card with RTC-time stamp
//Till Francke, 2019
//ver 1.0

//Usage:
//- install hardware required (see "Hardware required" and "Wiring")
//- install libraries (see "Software required")
//- verify/set clock using separate script (e.g. set_clock.ino)
//- adjust settings (see "Settings"

//Hardware required:
//D3231 real time clock, SD-card slot (or both combined in Shield, e.g. https://snootlab.com/lang-en/shields-snootlab/1233-memoire-20-ds3231-fr.html)
//for wiring details, see below

//Software required:
//Please install additional libraries via "Tools" -> "Manage Libraries" or download and extract libraries to c:\Program Files (x86)\Arduino\libraries\
// SDI: The library is available at: https://github.com/EnviroDIY/Arduino-SDI-12
// RTC: DS3231 https://github.com/NorthernWidget/DS3231; delete any existing library
// sleep mode: https://github.com/rocketscream/Low-Power V1.8

//Wiring
//RTC (no wiring required when using Shield, except for SWQ (between battery and SD-card slot))
// SD-card-pin ->  Arduino-pin
// Vin -> 5 V
// GND -> GND
// SCL -> Analog 5
// SDA -> Analog 4
// SQW -> pin (default: 2)  (green)

// SD card (no wiring required when using Shield):
// SD-card-pin ->  Arduino-pin
// ** MOSI -> pin D11
// ** MISO -> pin D12
// ** CLK (or SLK) -> pin D13
// ** CS -> pin (default: D10 )

//SDI-12:
//  You should not have more than one SDI-12 device attached for this example.
// https://raw.github.com/Kevin-M-Smith/SDI-12-Circuit-Diagrams/master/basic_setup_no_usb.png
// SD-card-pin ->  Arduino-pin 
// GND -> GND
// +Vbat -> 5 V (if the external device needs more than 5 V, connect it to external battery instead of arduino)
// SDI-12 Data -> pin (default: D7) 
// Truebner SMC: data: green; ground: white; V+: brown
// delta_T PR2: data: black; ground: blue; V+: white (needs external 12 V)
 

// Settings:
#define SERIAL_BAUD 9600  // The baud rate for the output serial port
#define INTERVAL 8000 //interval between measurements [m sec]

//for SDI-12 sensor
#define DATA_PIN 7         // The pin of the SDI-12 data bus
#define POWER_PIN -1       // The sensor power pin (or -1 if not switching power)
//#define sdi_address '3'; //SDI-address of attached sensor (Truebner)
#define sdi_address '1' //SDI-address of attached sensor (PR2/6 SDI)

//for SD-card
#define chipSelect 10  //pin used by SD-card slot. For the Snootlab-Shield, this is fixed to 10

//for SDI-12 sensor
#include <SDI12.h>
SDI12 mySDI12(DATA_PIN); // Define the SDI-12 bus

// Begin code section - no user changes required below (sic!)
//error codes issued by RX-LED onboard the Arduino
String errors[4] = { "", //short flashes every 2 secs
                     "",
                     "no SD-card found",
                     "SD-card write error"
                      };


//for SD-card
#include <SPI.h>
#include <SD.h>

// clock
#include <DS3231.h> // https://github.com/NorthernWidget/DS3231; delete any existing library
#include <Wire.h>
//DS3231 Clock; //63 77
RTClib RTC;
DateTime now;

//sleep mode
#include <LowPower.h> // https://github.com/rocketscream/Low-Power V1.8

String logfile_name="";
char DateAndTimeString[22]; //19 digits plus the null char

void setup_sdi(){
  Serial.print(F("Init SDI-12 bus..."));
  mySDI12.begin();
  delay(500); // allow things to settle

  // Power the sensors;
  if(POWER_PIN > 0){
    Serial.println(F("power up sensors..."));
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200);
  }
}

void setup_sdcard()
{
  Serial.print(F("Init SD card..."));
  delay(500);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    
    Serial.println(F("Card failed, or not present"));
    error_message(2);
    // don't do anything more
  }
   delay(100);
   Serial.println(F("ok."));
  
}

void setup_clock()
{
   // Initialize the rtc object
  Serial.print(F("Init clock..."));
  // Start the I2C interface
  Wire.begin();
 
//  bool Century=false;
//  bool h12;
//  bool PM;

  now = RTC.now();
  sprintf(DateAndTimeString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
  // Serial.println(DateAndTimeString);
  
  //Serial.println(F("RTC-time: ")+(String)now.year()+"/"+ (String)now.month()+"/"+ (String)now.day()+" "+ (String)now.hour()+":"+ (String)now.minute()+":"+ (String)now.second());
  Serial.print(F("RTC-time: "));
  Serial.println(DateAndTimeString);

  //logfile_name = (String)Clock.getYear()+(String)Clock.getMonth(Century)+(String)Clock.getDate();

  //assemble name of logfile to be created
  sprintf(DateAndTimeString, "%4d%02d%02d", now.year(), now.month(),now.day()); 
  logfile_name = (String)DateAndTimeString + ".log";
  Serial.print(F("logfile:"));
  Serial.println(logfile_name); 
}

void setup() { //this function is run once on power-up
  // Open serial communications and wait for port to open:
  Serial.begin(SERIAL_BAUD);
    while(!Serial);// wait for serial port to connect. Needed for native USB port only

  setup_clock();
 
  setup_sdcard();
 
  setup_sdi();
}

String read_sensor()
{
  String response = takeMeasurement(sdi_address); 
  return(response);
}

int count_values(String sdi_string) //return number of values in String by counting separating Tabs
{
  byte tab_counter=0;
  for (int i=0; i< sdi_string.length(); i++)
    if (sdi_string[i]=='\t') tab_counter++;
 //Serial.println(F("tab_counter: ")+(String)tab_counter);
  return(tab_counter);
}
  
String takeMeasurement(char i){
  //Serial.print(F("reading from")+(String)i);

  String command = "";
  command += i;
  command += "M!"; // SDI-12 measurement command format  [address]['M'][!]
  mySDI12.sendCommand(command); //send command via SDI
  delay(30);

  // wait for acknowlegement with format [address][ttt (3 char, seconds)][number of measurments available, 0-9]
  String sdiResponse = "";
  delay(30);
  while (mySDI12.available())  // build response string
  {
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r'))
    {
      sdiResponse += c;
      delay(5);
    }
  }
  mySDI12.clearBuffer();

  //Serial.println(F("response: ")+sdiResponse);

  // find out how long we have to wait (in seconds).
  uint8_t wait = 0;
  wait = sdiResponse.substring(1,4).toInt();
  //Serial.print(F("wait: ")+(String)wait); //print required time for measurement [s]

  // Set up the number of results to expect
  int numMeasurements =  sdiResponse.substring(4,5).toInt();

  unsigned long timerStart = millis();
  while((millis() - timerStart) < (1000 * wait)){
    if(mySDI12.available())  // sensor can interrupt us to let us know it is done early
    {
      mySDI12.clearBuffer();
      break;
    }
  }
  // Wait for anything else and clear it out
  delay(30);
  mySDI12.clearBuffer();
//Serial.print(F("waited "));

  // iterate through all D-options until the expected number of values have been obtained
  String result="";
  int dataOption=0; //number of "D-channel" to access/iterate
  while(dataOption < 10)
  {
    command = "";
    command += i;
    command += "D"+(String)dataOption+"!"; // SDI-12 command to get data [address][D][dataOption][!]
    mySDI12.sendCommand(command);
  //  Serial.print(F("requested data "));
    while(!mySDI12.available()>1); // wait for acknowlegement
    delay(300); // let the data transfer
    result += readSDIBuffer();
 //   Serial.print(F("data:")+(String)dataOption+":"+(String)result);
    if (count_values(result) >= numMeasurements) break; //exit loop when the required number of values have been obtained
    dataOption++; //read the next "D-channel" during the next loop
  }

  
  mySDI12.clearBuffer();
  return(result);
}

String readSDIBuffer(){
  String buffer = "";
  mySDI12.read(); // consume address
  while(mySDI12.available()){
    char c = mySDI12.read();
    if(c == '+'){
      buffer += '\t';
    }
    else if ((c != '\n') && (c != '\r')) {
      buffer += c;
    }
    delay(50);
  }
 //Serial.print(buffer);
 return(buffer);
}

void blink_rx_led(int times, String msg) //write <times> blocks of char to serial port using msg to let the RX led flash
{
  const long blink_length = 300; //duration of blinks and pauses
  for (int i=0;i < times; i++) 
  {
    for (int j=0; j < (blink_length * (SERIAL_BAUD/8) /1000)/msg.length(); j++)
        Serial.println(msg); 
    delay(blink_length);
  }  
}

void error_message(int error_id) //keep blinking and issuing message
{
  while (1) 
  {
    Serial.println(errors[error_id]); 
    blink_rx_led(error_id, errors[error_id] );
    delay(2000);
  }  
}

void wait(long interval) //wait for the requested number of millisec while still showing output/TX-activity at certain times
{
  const int blink_interval = 2000; //blink TX/output every nn msecs. Should be a multiple of "interval"
  for(long i=interval; i >= 0; i -= blink_interval)
  {
    Serial.print(F("next reading in "));
    Serial.println(String(i/1000) +" s"); 
    delay(blink_interval);
  } 
  
}

void loop() { //this function is called repeatedly as long as the arduino is running
  
  // make a string for assembling the data to log:
  String output_string = "";
  
// assemble timestamp 
  /*bool Century=false;
  bool h12;
  bool PM;

  output_string = (String)Clock.getYear()+"-"+ (String)Clock.getMonth(Century)+"-"+ (String)Clock.getDate()+" "+ 
              (String)Clock.getHour(h12, PM)+":"+ (String)Clock.getMinute()+":"+ (String)Clock.getSecond() + "\t";
  */
  now = RTC.now();
  sprintf(DateAndTimeString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(),now.day(),now.hour(),now.minute(),now.second());
  output_string = (String)DateAndTimeString;
  
  output_string += read_sensor(); //measure and read data from sensor
    
  
  //Serial.println(F("string to log:")+(String)output_string); 
  
  File dataFile = SD.open(logfile_name, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(output_string);
    dataFile.close();
    // print to the serial port too:
    Serial.print(F("string logged:"));
    Serial.println(output_string);

    // Wait for next sensor reading
    wait (INTERVAL);   
    //delay(INTERVAL);  //pause for "INTERVAL" msecs
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print(F("error opening "));
    Serial.println(logfile_name);
    error_message(3); //blink LED 3 times
  }
  
}
