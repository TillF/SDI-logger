//settings for SDI-12 devices
// Begin settings -------------------------------------------------
#define debug_output 1 //enable additional screen output for debugging purposes

#if board==uno | board==nano 
  #define DATA_PIN 7         // The pin of the SDI-12 data bus (UNO: 7; Pro Micro: 8)
#endif
#if board==promicro 
  #define DATA_PIN 8         // The pin of the SDI-12 data bus (UNO: 7; Pro Micro: 8)
#endif
  
#define POWER_PIN 4       // The sensor power pin (or -1, if not switching power)
//const char sdi_addresses[] = { '0', '1', '3'}; //list of IDs of attached SDI-sensors (single-character IDs only!)
//const char sdi_addresses[] = { '1'}; 
const char sdi_addresses[] = "1"; //list of IDs of attached SDI-sensors (single-character IDs only!) 
const char channels_measurement[] = "02"; //list of channels to query in the measurement (with "<sdi_address>M<channel>"!")

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


byte count_values(String sdi_string) //return number of values in String by counting separating Tabs
{
  byte tab_counter=0;
  for (unsigned int i=0; i < sdi_string.length(); i++)
    if (sdi_string[i]=='\t') tab_counter++;
  return(tab_counter);
}

int read_sdi(char i, File dataFile, boolean last_attempt){
  #if debug_output 
    Serial.print(F("reading sensor"));  Serial.print((String)i); //rr
  #endif

  String temp_str = "";  //generate command

  String result="";
  byte dataOption=0; //number of "D-channel" to access/iterate

  for (byte j=0; j < strlen(channels_measurement); j++)
  {
    temp_str = (String)i + "M"+(String)channels_measurement[j]+"!"; // SDI-12 command to measure [address][M][channel][!]
    
    mySDI12.sendCommand(temp_str); //send command via SDI - prevents sleep mode after second iteration 
    delay(30);
  #if debug_output 
   Serial.println("\nQuery str:"+temp_str); //rr
  #endif
  
    // wait for acknowledgement with format [address][ttt (3 char, seconds)][number of measurements available, 0-9]
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

  #if debug_output 
    Serial.println("response:"+temp_str); //rr
  #endif
  
    // find out how long we have to wait (in seconds).
    byte wait = 0;
    wait = temp_str.substring(1,4).toInt();
    //Serial.print("wait:"+(String)wait); //print required time for measurement [s]
  
  // compute the number of results to expect
    byte xpectMeasurements =  temp_str.substring(4,5).toInt();
    xpectMeasurements =  max(xpectMeasurements, 1); //we will try to retrieve at least one measurement
    #if debug_output 
      Serial.println("meas xpected:"+(String)xpectMeasurements); //print number of expected measurement 
    #endif
  
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
  #if debug_output 
    Serial.print(F("waited ")); //rr
  #endif

  // iterate through all D-options or until the expected number of values have been obtained
  byte recvdMeasurements=0; //number of received values
	for(dataOption=0; (dataOption < 10) & (recvdMeasurements < xpectMeasurements); dataOption++)
    {
    temp_str = (String)i + "D"+(String)dataOption+"!"; // SDI-12 command to get data [address][D][dataOption][!]
      mySDI12.sendCommand(temp_str);
    #if debug_output 
      Serial.println("request data:"+temp_str); //rr
    #endif

      while(!mySDI12.available()>1); // wait for acknowledgement
      delay(300); // let the data transfer ii: reduce?
   temp_str = readSDIBuffer(); 
    
    //fix problem with SMT-100 returns, that miss a separator when temperature is negative
    for (timerStart=temp_str.length()-2; timerStart > 0; timerStart--) 
     if (temp_str[timerStart]!='\t' && temp_str[timerStart+1]=='-')
      temp_str = temp_str.substring(0,timerStart+1)+"\t"+temp_str.substring(timerStart+1, temp_str.length());
   
    result += temp_str;
  	recvdMeasurements += count_values(temp_str); //increase number of received values
    
    #if debug_output 
      Serial.print("data:"+(String)temp_str); //rr
      Serial.print("; countnew:"+(String)count_values(temp_str));
      Serial.println("; counttotal:"+(String)recvdMeasurements); //rr
    #endif
    }
    mySDI12.clearBuffer();
	delay(30);
  } //end loop thru channels_measurement


if (result.substring(0,3)=="\t0\t") //SMT-100 sometimes yields "0" reading. Treat this as NA.
  result="";

dataOption = result.length(); //store length of obtained result

if (dataOption == 0) //no data from sensor
{
  if (!last_attempt)  //if this is the last attempt, write as NA
    return(0);
    
  #if WRITE_NA==1 
    result="\tNA"; //in case of no data from sensor, write "NA"
  #endif
}
  
  temp_str = "\tSDI"+(String)i; //add SDI-12-adress and field separators
 
  //Serial.print(temp_str);   //write "header" to file
  //Serial.print(result);   //write results to file
  dataFile.print(temp_str); // write to file
  dataFile.print(result); // write to file
  return(dataOption); //return length of string for later checking
}

int read_all_SDI(File dataFile) //read all SDI specified in list
{
  int char_counter=0;
  int res_length; //length of returned result string
  byte failed_reading_attempts = 0; //number of consequetive unsuccessful readings from a device
  
  for (byte i=0; i < strlen(sdi_addresses); i++)
  {
    //Serial.println("sens:"+(String)i);
    res_length = read_sdi(sdi_addresses[i], dataFile, failed_reading_attempts >= MAX_READING_ATTEMPTS);     // read SDI sensor and write to file, count number of characters written
 
    if ((res_length == 0) && (failed_reading_attempts < MAX_READING_ATTEMPTS)) //no data from sensor AND maximum number of attempts to read from a sensor reached
    {
      failed_reading_attempts++;
      i--; // stay at the same sensor
      blink_led(4, "NA-read"); //indicate "no data" via message LED
      digitalWrite(messagePin, HIGH); //indication for "reading"
      wait(max(0, AWAKE_TIME - (round(4*2*300/1000)))); //wait remaining time (after substracting blinking time)
      continue; //re-do this cycle
    } 
    
    char_counter += res_length;
    
    //reading_attempts=0; //reset counter for the next sensor
  }   
  String temp_str = "\t"+(String)failed_reading_attempts; //rr remove me, for debugging only
  dataFile.print(temp_str); // write to file
  
  /*
  Serial.print(" awaketime:"); //rr remove me
  Serial.print(awake_time_current);
  Serial.print(" succ reads:"); //rr remove me
  Serial.println(successful_readings);
  */
 return(char_counter);
}
