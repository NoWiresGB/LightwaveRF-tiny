#include <LightwaveRF.h>
#include <avr/interrupt.h>;
#include <avr/sleep.h>;
#include <avr/power.h>;  

//LightwaveRF constants for dimming.
#define FULL_BRIGHTNESS    31   // This is max power  8 ->31 usable.
#define HIGH_BRIGHTNESS    22
#define MID_BRIGHTNESS     19
#define LOW_BRIGHTNESS     15
#define MOVIE_MODE         9

// constants won't change. They're used here to set pin numbers:                     
byte myid[] = {0x6F,0xEB,0xBE,0xED,0xB7,0x7B};
const int lwrfPin = 0;
const int buttonPin = 1;     // the number of the pushbutton pin
const int ledPin =  2;      // the number of the LED pin
const long debounceDelay = 100;

// variables will change:
volatile bool ledState = LOW; // variable for led status
volatile bool lightStateChange = LOW;
byte lightState = 0;
String inString = "";    // string to hold input
int level = 31;
bool buttonState = HIGH;
long lastDebounceTime = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lw_tx_setup(lwrfPin);
  PCMSK |= bit (PCINT1); // pin change interrupt enabled for PCINT1/PB1/Pin 1
  GIFR |= bit (PCIF);    // clear outstanding interrupts
  GIMSK = bit (PCIE);    // turns on pin change interrupts
  // sei();                 // enables interrupts   
}

void loop() {
//  buttonState = digitalRead(buttonPin); // anything going on?
//
//  //filter out any noise by setting a time buffer
//  if ( (millis() - lastDebounceTime) > debounceDelay) {
//    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
//    if (buttonState == LOW) {
//      ledState = !ledState;
//      lightStateChange =!lightStateChange;
//      lightState++;
//      digitalWrite(ledPin, ledState);
//      lastDebounceTime = millis();
//    } // close button loop
//  } // close debounce loop
digitalWrite(ledPin, ledState);

  if (lightStateChange == HIGH) {
    // reset ready for next time
    lightStateChange = LOW;
    lightState++;
    digitalWrite(ledPin, ledState);
    switch (lightState) {
      case 1:
        turnOn();
        break;
      case 2:
        dimLevel(FULL_BRIGHTNESS);
        break;
      case 3:
        dimLevel(HIGH_BRIGHTNESS);
        break;
      case 4:
        dimLevel(MID_BRIGHTNESS);
        break;
      case 5:
        dimLevel(LOW_BRIGHTNESS);
        break;
      case 6:
        dimLevel(MOVIE_MODE);
        break;
      case 7:
        lightState = 0;
        turnOff();
        break;
    } // switch case
  } // light state change

  myPowerDown();  // We're done.  Go to sleep and wait for button.
}

void myPowerDown() 
{  

  // digitalWrite(PB0, LOW); // power down leds
  // digitalWrite(PB1, LOW); // power down leds
  ADCSRA = 0;
  power_all_disable(); // shut down ADC, Timer 0 and 1, serial etc.
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); 
  sleep_enable();
//  sei();  // interrupts already running
  sleep_bod_disable();
  sleep_cpu();
  sleep_disable();
  power_all_enable();
  delay(200);
  digitalWrite(ledPin, HIGH);
  delay(200);
  ledState = LOW;    

} 

ISR(PCINT0_vect) //PIN CHANGE INTERRUPT ROUTINE
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > debounceDelay)
  {
    ledState = !ledState;
    lightStateChange = HIGH;
  }
  last_interrupt_time = interrupt_time;
  
}


// https://wiki.somakeit.org.uk/wiki/LightwaveRF_RF_Protocol

void turnOff() {
  // lw_cmd(0xA0,6,LW_OFF,myid);  // reduce brightness
  lw_cmd(0x7F,6,LW_OFF,myid);  // turn OFF
}

void turnOn() {
  lw_cmd(0x00,6,LW_ON,myid);  // turn oN TO LAST LEVEL
}

void dimLevel(byte level) {
  lw_cmd(0x80 + level,6,LW_ON,myid);
}


 
