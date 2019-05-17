//initialize and use MPU9250 (gyro-magnetic-temperature sensor)
// needs library https://github.com/sparkfun/SparkFun_MPU-9250_Breakout_Arduino_Library/
//wiring: I2C:
// Gyro -> UNO:
//Vin -> 5V or 3.3V (red)
//GND -> GND (blue)
//SCL -> pin 17 (SCL) (orange)
//SDA -> pin 16 (SDA) (magenta)
//SDO/SAO -> 5V or 3.3V (needed to change the address to avoid conflicts with RTC)


#define I2Cclock 400000
#define I2Cport Wire
//#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0   // Use either this line or the next to select which I2C address your device is using
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD1   //put 5 V on SDO/SAO - pin

#include "MPU9250.h"
MPU9250 myIMU(MPU9250_ADDRESS, I2Cport, I2Cclock);


void setup_imu()
{
  Serial.println(F("Init IMU..."));
 Wire.begin();

    byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    if (c != 0x73) 
    {
      Serial.println(F("MPU9250 not found."));
      while(1);
    }
    myIMU.MPU9250SelfTest(myIMU.selfTest);

   // Calibrate gyro and accelerometers, load biases in bias registers
    //myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    Serial.println(F("ok"));

    
       // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.factoryMagCalibration);

    
     // Get sensor resolutions, only need to do this once
    //myIMU.getAres();
    //myIMU.getGres();
    myIMU.getMres();
   //Serial.print("Z-mag");Serial.print(myIMU.magCount[2]); Serial.print(myIMU.mRes); Serial.print(myIMU.factoryMagCalibration[2]); Serial.print(myIMU.magBias[2]);  Serial.println(myIMU.mz);
 
}

String read_imu()
{
  String resString="NA  NA  NA"; 
  uint8_t i;
  for (i=0; (!(myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)) &&  (i<255); i++)
  {
    delay(10); //wait until ready
    //Serial.println("w"+(String)i);
    //Serial.flush();
  }
  //Serial.println("waited");Serial.flush();
  if (i<255)
  {
     myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
               * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
               * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
               * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];

    //Serial.print("Z-mag");Serial.print(myIMU.magCount[2]); Serial.print(myIMU.mRes); Serial.print(myIMU.factoryMagCalibration[2]); Serial.print(myIMU.magBias[2]);  Serial.println(myIMU.mz);
    //Serial.println(myIMU.mx); Serial.println(myIMU.my);  Serial.println(myIMU.mz);
    
    resString = +"\t"+(String)round(myIMU.mx)+"\t"+(String)round(myIMU.my)+"\t"+(String)round(myIMU.mz);
        
  }
  return(resString);
}
