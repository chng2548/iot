#include <avr/sleep.h>
#include <avr/interrupt.h>

int PinStatus = 1;  // d6 LED
int PinSensor = 2;


void sleep() {
  digitalWrite(PinSensor, HIGH);
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);                   // Use PB3 as interrupt pin
  PCMSK |= _BV(PCINT3);
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement
  
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
beep();
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT2);                  // Turn off PB3 as interrupt pin
  PCMSK &= ~_BV(PCINT3);
  sleep_disable();                        // Clear SE bit
  //ADCSRA |= _BV(ADEN);                    // ADC on
  
  sei();                                  // Enable interrupts
} // sleep


ISR(PCINT0_vect) { 
}

void setup() {
  //delay(3000);

  Serial.println("HI!");
  pinMode(PinSensor, INPUT); // declare sensor as input
  digitalWrite(PinSensor, HIGH);
  
  pinMode(PinStatus, OUTPUT);  // declare LED as output
}

void loop(){
  sleep();
  Buzz();
}

void beep() {
digitalWrite(1, HIGH);
delay(300);
digitalWrite(1, LOW);
delay(300);
}


void Buzz() {
  for(int i1=0;i1<=20;i1++) {
    digitalWrite (PinStatus, HIGH);
    delay(150);
    digitalWrite (PinStatus, LOW);
    delay(70);    
  }
  digitalWrite (PinStatus, HIGH);
  delay(1000);
  digitalWrite (PinStatus, LOW);
}
