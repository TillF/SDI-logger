/***
to be run once:
Write a variable to EEPROM (permanent) used as Logger-ID by SDI-logger.ino)
***/

#include <EEPROM.h>

  char logger_id[3] = "";  //Variable to store in EEPROM.
  int eeAddress = 0;   //Location we want the data to be put.


void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  EEPROM.get(eeAddress, logger_id);
  Serial.println("current ID: "+(String)logger_id);

  Serial.println("Set Serial Monitor window to \"newline\" in the dropdown at the bottom!");
  Serial.println("Enter new logger-ID (3 characters, terminated with \"x\"), e.g. \"L01x\" (without the quotes)");

 
 
 }

void loop() {
  if (Serial.available()) {
  char InString[5];
  char InChar;
    byte j=0;

    for (j=0;j<5;j++)
      InString[j]='\0';

     j=0;
      
    while (j<5) {
      if (Serial.available()) {
        InChar = Serial.read();
        if (InChar == 'x' || InChar == '\n')
          break;
        else
        InString[j] = InChar;
        j += 1;
      }
    }
   InString[j] = '\0';
   
    delay(100); //wait for buffer
    while (Serial.available())  //clear remaining buffer
      InChar = Serial.read();

    if (j > 3)
    {
      Serial.println("Please use 3 characters plus a terminating \"x\"");
      return;
    }
     for (j=0;j<3;j++)
      logger_id[j] = InString[j];
     Serial.println(InString);
     EEPROM.put(eeAddress, logger_id);

      Serial.println("Written ID:"+(String)logger_id);

  Serial.println("Checking success, reading ID...");
  logger_id[0] ='\0';
  //logger_id = EEPROM.read(eeAddress);
  EEPROM.get(eeAddress, logger_id);

  Serial.println("read ID:"+(String)logger_id);
  Serial.println("repeat if required");

  }
}
