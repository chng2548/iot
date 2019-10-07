#include <avr/sleep.h>
#include <avr/interrupt.h>

int ServoPin = 2;
int SpeakerPin = 1;
int InfraPin = 0;


void sleep1() {
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT0);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement
  
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
//beep();
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT0);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  //ADCSRA |= _BV(ADEN);                    // ADC on
  sei();                                  // Enable interrupts
} // sleep


ISR(PCINT0_vect) { 
}

void setup() {  
  pinMode(ServoPin, OUTPUT);
  pinMode(SpeakerPin, OUTPUT);
  pinMode(InfraPin, INPUT_PULLUP);


}

void loop() {
delay(200);
  Servo0();

  sleep1();
  Servo1();
  beep();
//if (digitalRead(0)==0) {
 // beep();
//}
}



void beep() {
  digitalWrite(SpeakerPin, HIGH);
  delay(300);
  digitalWrite(SpeakerPin, LOW);
  delay(300);
}

void Servo0() {
  // keep sending 0 position for about 400ms
  for(int i = 0; i<40; ++i) {
    digitalWrite(ServoPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(ServoPin, LOW);
    delay(19);
  }  
}

void Servo1() {
  // keep sending second position for about 400ms
  for(int i = 0; i<20; ++i) {
    digitalWrite(ServoPin, HIGH);
    delayMicroseconds(2000);
    digitalWrite(ServoPin, LOW);
    delay(19);
  }  
}
