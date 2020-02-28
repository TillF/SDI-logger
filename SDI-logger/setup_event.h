//initialize and read events (contacts) on designated pins
#define EVENT_PIN 1         // The pin connected to the reed-relais (default: pin 1 on Pro Micro) 

volatile int event_counts = 0; 

void count_event() //count number of events
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 150ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 150)
 {
  detachInterrupt(digitalPinToInterrupt(EVENT_PIN)); //prevent multiple invocations of interrupt 
  EIFR = bit (digitalPinToInterrupt(EVENT_PIN));  // clear flag for interrupt (deletes any pending interrupt calls) 
  attachInterrupt(digitalPinToInterrupt(EVENT_PIN), count_event, HIGH); //re-initiate interrupt, when pin is high again
  event_counts++; //increase event counter
  last_interrupt_time = interrupt_time;
 }
}

void setup_event(){
  pinMode(EVENT_PIN, INPUT_PULLUP);   //the event then will draw this pin from high to low to denote an event
  noInterrupts ();          // make sure we don't get interrupted before finished  
  EIFR = bit (digitalPinToInterrupt(EVENT_PIN));  // clear flag for interrupt (deletes any pending interrupt calls) 
  attachInterrupt(digitalPinToInterrupt(EVENT_PIN), count_event, LOW);
  interrupts ();           
}
