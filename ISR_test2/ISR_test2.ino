#include <stdlib.h>
#include <stdint.h>

unsigned int ledPin = 13;

double clock_freq = 16000000;

volatile boolean doStim = false;
unsigned long startStimulation_ms = 1*1000;
unsigned long stopStimulation_ms = 1800*1000;
unsigned long timerTicks = 0;

// Same struct lives in the OE code and gets sent over when recording starts
// see StimControl.h there
struct StimSettings {
  uint16_t inputPin = 13;
  uint16_t outputPin = 3;
  uint16_t startTime = 1;
  uint16_t stopTime = 1800;
  uint16_t stimOnTime = 10;
  uint16_t stimOffTime = 150;
  uint16_t hasData = 1;
};
StimSettings stim;

void startCounting(unsigned long start_ms, unsigned long stop_ms)
{
  doStim = false; // time to begin not yet... 
  startStimulation_ms = start_ms; // how many 1ms counts to do before starting timers to do stimulation
  stopStimulation_ms = stop_ms;
  timerTicks = 0; // reset the interrupt counter
  // no interrupts
//  TIMSK2 = 0;
  // reset Timer 2
  TCCR2A = 0;             
  TCCR2B = 0;              
  TCCR2A |= (1 << WGM21); // CTC mode
  TCNT2 = 0; // Timer 2 to zero
  OCR2A = 124; // count up to 125
  TIMSK2 |= (1 << OCIE2A); // enable Timer 2 interrupt
  // Start Timer2
  TCCR2B |= (1 << CS20) | (1 << CS22); //  prescaler of 128
}

// this is invoked every 1ms
ISR(TIMER2_COMPA_vect) {
  timerTicks++;
  if (timerTicks < startStimulation_ms) {
    doStim = false;
    return;
  }
  if (timerTicks >= stopStimulation_ms) {
    // reset timers and terminate the ongoing ISR
    stopStimulation();
    return;
  }
  doStim = true;
}

void calculateCompareTimes(uint16_t stimOnTime, uint16_t stimDuration) {
  // Reset Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0; // Set Timer1 counter to 0
  // set mode to CTC and prescaler to 64
  TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11) | (0 << CS12); // was just (1 << CS12) - ie prescaler of 256
  // enable timer compare interrupts for both registers on Timer1 (timer1_compA and timer1_compB)
  TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);
  // When to go low:
  // This is calculated as the clock frequency on the Arduino Uno (16MHz) divided by
  // the pre-scaler divided by 1 second in ms divided by number of ms every second
  // you want the pulse to come on. So with a prescaler of 256 it looks like:
  // start = 16e6/256/(1000/150.) = 9375.0 
  float prescaler = 64.0;
  unsigned long start = floor(clock_freq / prescaler / (1000.0 / float(stimOnTime)));
  OCR1A = start;
  // the next interrupt is triggered to set the pulse pin go high:
  // This is the duration of time you want the pin to go high for (10 ms in my default case)
  // Calculated as the above variable (called start in the above comment) minus this duration
  // Duration is calculated by: clock_freq / pre-scaler / (1000/duration_in_ms).
  // So this looks like:
  // duration = 16e6 / 256 / (1000/10.) = 625.0 
  // the OCR1B number is the start - duration so here it is 
  // 9375.0 - 625.0 = 8750
  unsigned long duration = floor(clock_freq / prescaler / (1000.0 / float(stimDuration)));
  OCR1B = (start - duration);
}
      
void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  while ( ! Serial ) {
    ;//waiting for serial port to connect. Apparently needed for native USB port only
  }
}

ISR(TIMER1_COMPA_vect)
{
  if (doStim) {
    digitalWrite(ledPin, LOW);
  }
}

ISR(TIMER1_COMPB_vect)
{
  if (doStim) {
    digitalWrite(ledPin, HIGH);
  }
}

void stopStimulation() {
  noInterrupts();
  doStim = false;
  digitalWrite(ledPin, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  // disable interrupts on timer 1
  TIMSK1 = (0 << OCIE1A) | (0 << OCIE1B);
  // disable interrupts on timer 2
  TIMSK2 |= (0 << OCIE2A);
  interrupts();
}

StimSettings current_settings;

void loop() {
  // if ( Serial.available() > 0 ) {
    // if ( Serial.readBytes((char*)&current_settings, sizeof(current_settings)) == sizeof(StimSettings) ) {
      if (stim.hasData > 0) {
        noInterrupts();
        ledPin = stim.outputPin;
        pinMode(ledPin, OUTPUT);
        pinMode(LED_BUILTIN, OUTPUT);
        calculateCompareTimes(stim.stimOffTime, stim.stimOnTime);
        unsigned long start_at = (unsigned long)stim.startTime;// * 1000;
        unsigned long stop_at = (unsigned long)stim.stopTime;// * 1000;
        startCounting(start_at*1000, stop_at*1000);
        interrupts();
      }
      else {
        stopStimulation();
      }
    // }
  // }
}
