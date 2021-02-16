//settings for SDI-12 devices

// Begin settings -------------------------------------------------
#if board==uno | board==nano 
  #define DATA_PIN 7         // The pin of the SDI-12 data bus (UNO: 7; Pro Micro: 8)
#endif
#if board==promicro 
  #define DATA_PIN 8         // The pin of the SDI-12 data bus (UNO: 7; Pro Micro: 8)
#endif
  
#define POWER_PIN 4       // The sensor power pin (or -1, if not switching power)
//const char sdi_addresses[] = { '0', '1', '3'}; //list of IDs of attached SDI-sensors (single-character IDs only!)
//const char sdi_addresses[] = { '1'}; 
const char sdi_addresses[] = "012345ABCDEF"; //list of IDs of attached SDI-sensors (single-character IDs only!) 

#define WRITE_NA 1         // 1: in case of missing data from a sensor, write "NA" instead; 0: write empty string in case of missing data

// end settings --------------------------------------------------------

#include <SDI12.h> //SDI-12 library

SDI12 mySDI12(DATA_PIN); // Define the SDI-12 bus

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

void sensor_power(byte onoff) //power the SDI devices
{
 if(POWER_PIN > 0){
    //Serial.print(F("sensor power: "));
    //Serial.println(onoff);
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, onoff);
    delay(200);
  } 
}
String setup_sdi(){
  sensor_power(1); // Power the sensors;
  Serial.print(F("Init SDI-12 bus..."));
  mySDI12.begin();
  delay(500); // allow things to settle
  
  String result="";
  char temp_str[4]="aI!"; // SDI-12 identification command format  [address]['I'][!]

  //get IDs of all requested SDI-sensors
  for (byte i=0; i < strlen(sdi_addresses); i++)
  {
    temp_str[0] = sdi_addresses[i]; // SDI-12 identification command format  [address]['I'][!]
    mySDI12.sendCommand(temp_str); //send command via SDI - prevents sleep mode after second iteration 
    delay(100);
    //Serial.println(temp_str);
    
    result += readSDIBuffer()+"\t"; 
    //Serial.println("found sensors:"+result); 
    mySDI12.clearBuffer();    
  }
  Serial.println(F("ok."));
  return(result);
}


int count_values(String sdi_string) //return number of values in String by counting separating Tabs
{
  byte tab_counter=0;
  for (int i=0; i< sdi_string.length(); i++)
    if (sdi_string[i]=='\t') tab_counter++;
  return(tab_counter);
}

int read_sdi(char i, File dataFile){
   //Serial.print(F("reading from")+(String)i);

  String temp_str = "";  //generate command
  temp_str += i;
  temp_str += "M!"; // SDI-12 measurement command format  [address]['M'][!]

  mySDI12.sendCommand(temp_str); //send command via SDI - prevents sleep mode after second iteration 
  delay(30);
  //Serial.println(temp_str); //rr
  
  // wait for acknowledgement with format [address][ttt (3 char, seconds)][number of measurments available, 0-9]
  temp_str = "";
  char c;
  delay(30);
  while (mySDI12.available())  // build response string
  {
    c = mySDI12.read();
    if ((c != '\n') && (c != '\r'))
    {
      temp_str += c;
      delay(5);
    }
  }
  mySDI12.clearBuffer();

  //Serial.println(F("response: ")+sdiResponse);

  // find out how long we have to wait (in seconds).
  uint8_t wait = 0;
  wait = temp_str.substring(1,4).toInt();
  //Serial.print("wait:"+(String)wait); //print required time for measurement [s]

  // Set up the number of results to expect
  uint8_t numMeasurements =  temp_str.substring(4,5).toInt();
  //Serial.println("tstr:"+temp_str); //print number of expected measurement [s]
  //Serial.println("meas xpected:"+(String)numMeasurements); //print number of expected measurement [s]

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

  uint8_t dataOption=0; //number of "D-channel" to access/iterate
  while(dataOption < 10)
  {
    temp_str = (String)i;
    temp_str += "D"+(String)dataOption+"!"; // SDI-12 command to get data [address][D][dataOption][!]
    mySDI12.sendCommand(temp_str);
  //Serial.println(F("request data "));
    while(!mySDI12.available()>1); // wait for acknowlegement
    delay(300); // let the data transfer ii: reduce?
    result += readSDIBuffer();
    //Serial.println("data:"+(String)dataOption+":"+(String)result);
    //Serial.println("count:"+(String)count_values(result));
    if (count_values(result) >= numMeasurements) break; //exit loop when the required number of values have been obtained
    dataOption++; //read the next "D-channel" during the next loop
  }
  mySDI12.clearBuffer();

  //result= "zxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxy"; //for testing

//result = ""; //rr test if increasing wating time is working correctly
//result = "test"; //rr test if increasing wating time is working correctly

dataOption = result.length(); //store length of obtained result

#if WRITE_NA==1 
  if (dataOption == 0) result="NA"; //in case of no data from sensor, write "NA"
#endif
  
  temp_str = "\tSDI"+(String)i+"\t"; //add SDI-12-adress and field separators
 
  Serial.print(temp_str);   //write "header" to file
  Serial.print(result);   //write results to file
  dataFile.print(temp_str); // write to file
  dataFile.print(result); // write to file
  return(dataOption); //return length of string for later checking
}

int read_all_SDI(File dataFile) //read all SDI specified in list
{
  int char_counter;
  int res_length; //length of returned result string
  bool NA_read=0; //mark if any NAs have been read
  for (byte i=0; i < strlen(sdi_addresses); i++)
  {
    res_length = read_sdi(sdi_addresses[i], dataFile);     // read SDI sensor and write to file, count number of characters written
    char_counter += res_length;
    if (res_length == 0) NA_read=1; //mark that NAs have been read
  }   
  if (NA_read)
  {
    awake_time_current = awake_time_current + INCREASE_AWAKE_TIME; //increase awake time
    awake_time_current = min(10*AWAKE_TIME, awake_time_current); //not more than 10times the original value
    successful_readings = 0; //number of consequetive successful readings
  } else
  {
    successful_readings++; //number of consequetive successful readings
    if (successful_readings >= DECREASE_AWAKE_TIME_CYCLES) //decrease awake time after DECREASE_AWAKE_TIME_CYCLES of unsuccessful readings
    {
      awake_time_current=awake_time_current-INCREASE_AWAKE_TIME; //decrease awake time
      awake_time_current=max(AWAKE_TIME, awake_time_current); //at least the original value
      successful_readings = 0; //number of consequetive successful readings
    }   
  }
  String temp_str = "\tawaketime"+(String)awake_time_current+"\tsucc_readings"+(String)successful_readings; //rr remove me, for debugging only
  dataFile.print(temp_str); // write to file
  
  Serial.print(" awaketime:"); //rr remove me
  Serial.print(awake_time_current);
  Serial.print(" succ reads:"); //rr remove me
  Serial.println(successful_readings);
  
 return(char_counter);
}
