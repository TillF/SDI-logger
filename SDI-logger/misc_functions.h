// misc functions to be used from various routines
void blink_led(int times, long blink_length) //blink message LED, if exists
{
  if (messagePin==0) 
    return; //no pin selected
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
  const long blink_length=300; //duration of blinks and pauses
  String errors[5] = { "", //short flashes every 2 secs
                     "",
                     "no SD-card found",  //2
                     "SD-card write error", //3
                     "NA-read" //4
                      };
  Serial.println(errors[error_id]);
  int8_t i;
  for(i=0; i < times; i++)
  {
    blink_led(error_id, blink_length);
    delay(2000);
  }  
}

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
