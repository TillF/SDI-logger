/*
Set time of D3231 RTC
needs library
DS3231 // https://github.com/NorthernWidget/DS3231; delete/rename any other existing library to avoid conflicts
*/

#include <DS3231.h>
#include <Wire.h>

DS3231 Clock;

byte Year;
byte Month;
byte Date;
//byte DoW;
byte Hour;
byte Minute;
byte Second;
    bool Century=false;
    bool h12;
    bool PM;


void GetDateStuff(byte& Year, byte& Month, byte& Day,// byte& DoW, 
		byte& Hour, byte& Minute, byte& Second) {
	// Call this if you notice something coming in on 
	// the serial port. The stuff coming in should be in 
	// the order YYMMDDwHHMMSS, with an 'x' at the end.
	boolean GotString = false;
	char InChar;
	byte Temp1, Temp2;
	char InString[20];

	byte j=0;
	while (!GotString) {
		if (Serial.available()) {
			InChar = Serial.read();
			InString[j] = InChar;
			j += 1;
			if (InChar == 'x') {
				GotString = true;
       InString[j] = '\0';
			}
		}
	}
	Serial.println(InString);
	// Read Year first
	Temp1 = (byte)InString[0] -48;
	Temp2 = (byte)InString[1] -48;
	Year = Temp1*10 + Temp2;
	// now month
	Temp1 = (byte)InString[2] -48;
	Temp2 = (byte)InString[3] -48;
	Month = Temp1*10 + Temp2;
	// now date
	Temp1 = (byte)InString[4] -48;
	Temp2 = (byte)InString[5] -48;
	Day = Temp1*10 + Temp2;
	// now Day of Week
//	DoW = (byte)InString[6] - 48;		
	// now Hour
	Temp1 = (byte)InString[7-1] -48;
	Temp2 = (byte)InString[8-1] -48;
	Hour = Temp1*10 + Temp2;
	// now Minute
	Temp1 = (byte)InString[9-1] -48;
	Temp2 = (byte)InString[10-1] -48;
	Minute = Temp1*10 + Temp2;
	// now Second
	Temp1 = (byte)InString[11-1] -48;
	Temp2 = (byte)InString[12-1] -48;
	Second = Temp1*10 + Temp2;
}



int serial_putchar(char c, FILE* f) {
    if (c == '\n') serial_putchar('\r', f);
    return Serial.write(c) == 1? 0 : 1;
}

void setup() {
	// Start the serial port
	Serial.begin(9600);
    while(!Serial);// wait for serial port to connect. Needed for native USB port only
  Serial.println("Initialize RTC...");
 
  
	// Start the I2C interface
	Wire.begin();
  Serial.println("OK");
 Serial.flush();
delay(1000);


// Set up printf to directly print to SDout
FILE serial_stdout;
fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
stdout = &serial_stdout;

  Serial.print((String)"Current RTC-time: "); 
  printf("%02d-", Clock.getYear());
  printf("%02d-", Clock.getMonth(Century));
  printf("%02d ", Clock.getDate());
  printf("%02d:", Clock.getHour(h12, PM));
  printf("%02d:", Clock.getMinute() );
  printf("%02d\n", Clock.getSecond());

  
  //dtostrf(Clock.getYear(),2,0)
  //Serial.println((String)"Current RTC-time: " + Clock.getYear()+"-"+ (String)Clock.getMonth(Century)+"-"+ (String)Clock.getDate()+" "+ 
 //             (String)Clock.getHour(h12, PM)+":"+ (String)Clock.getMinute()+":"+ (String)Clock.getSecond());

  Serial.println("Set Serial Monitor window to \"newline\" in the dropdown at the bottom!");
  Serial.println("Enter new time in the format YYMMDDHHMMSS, with an \'x\' at the end.");
 Serial.flush();
delay(1000);


}

void loop() {

	// If something is coming in on the serial line, it's
	// a time correction so set the clock accordingly.
	if (Serial.available()) {
		GetDateStuff(Year, Month, Date, Hour, Minute, Second);

		Clock.setClockMode(false);	// set to 24h
		//setClockMode(true);	// set to 12h

    Serial.println("Components read:");
    Serial.println(Year);
    Serial.println(Month);
    Serial.println(Date);
    Serial.println(Hour);
    Serial.println(Minute);
    Serial.println(Second);

//set the clock to extracted values
		Clock.setYear(Year);
		Clock.setMonth(Month);
		Clock.setDate(Date);
	//	Clock.setDoW(DoW);
		Clock.setHour(Hour);
		Clock.setMinute(Minute);
		Clock.setSecond(Second);
    
FILE serial_stdout;
fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
stdout = &serial_stdout;
  
  Serial.print((String)"Current RTC-time: "); 
  printf("%02d-", Clock.getYear());
  printf("%02d-", Clock.getMonth(Century));
  printf("%02d ", Clock.getDate());
  printf("%02d:", Clock.getHour(h12, PM));
  printf("%02d:", Clock.getMinute() );
  printf("%02d\n", Clock.getSecond());

    Serial.println("Repeat if required.");
    Serial.println("If this yields no changes or implausible values, please check the wiring to the RTC.");
    
    
	}
	delay(1000);
}
