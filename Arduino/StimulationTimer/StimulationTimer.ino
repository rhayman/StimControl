bool debug = false;
double clock_freq = 16000000;
unsigned int ledPin = 13;

volatile boolean doStim = false;
unsigned long startStimulation_ms = 1*1000;
unsigned long stopStimulation_ms = 100*1000;
unsigned long timerTicks = 0;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

      // variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0;

boolean newData = false;

struct __attribute__((packed)) StimBinaryPayload {
  uint8_t magic0;
  uint8_t magic1;
  uint8_t version;
  uint8_t payloadSize;
  uint16_t inputPin;
  uint16_t gatePin;
  uint16_t outputPin;
  uint16_t startTime;
  uint16_t stopTime;
  uint16_t stimOnTime;
  uint16_t stimOffTime;
  uint16_t hasData;
  uint16_t startRunning;
};

const uint8_t binaryMagic0 = 0xA5;
const uint8_t binaryMagic1 = 0x5A;
const uint8_t binaryProtocolVersion = 1;
const unsigned long maxTimer1PeriodMs = 4194UL;

int inputPin = 0;
int gatePin = 0;
int startTime = 600;
int stopTime = 1800;
int duration = 10;
int interval = 150;
int startRunning = 0;
int outputPin = 3;

// ==================================

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

void configureTimer1Prescaler(uint16_t prescaler) {
  switch (prescaler) {
    case 1:
      TCCR1B = (1 << WGM12) | (1 << CS10);
      break;
    case 8:
      TCCR1B = (1 << WGM12) | (1 << CS11);
      break;
    case 64:
      TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11);
      break;
    case 256:
      TCCR1B = (1 << WGM12) | (1 << CS12);
      break;
    default:
      TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
      break;
  }
}

uint16_t chooseTimer1Prescaler(unsigned long totalPeriodMs) {
  const uint16_t prescalers[] = {1, 8, 64, 256, 1024};

  for (uint8_t idx = 0; idx < 5; ++idx) {
    uint16_t prescaler = prescalers[idx];
    unsigned long ticks =
        floor(clock_freq / prescaler / (1000.0 / float(totalPeriodMs)));

    if (ticks > 0 && ticks <= 65535) {
      return prescaler;
    }
  }

  return 1024;
}

void calculateCompareTimes(uint16_t stimOffTime, uint16_t stimDuration) {
  // Reset Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0; // Set Timer1 counter to 0

  unsigned long onTimeMs = min(maxTimer1PeriodMs - 1, (unsigned long)max(1, stimDuration));
  unsigned long totalPeriodMs =
      min(maxTimer1PeriodMs, max(2UL, onTimeMs + stimOffTime));
  uint16_t prescaler = chooseTimer1Prescaler(totalPeriodMs);

  // set mode to CTC and choose a prescaler that keeps OCR1A in range
  configureTimer1Prescaler(prescaler);
  // enable timer compare interrupts for both registers on Timer1 (timer1_compA and timer1_compB)
  TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);

  unsigned long periodTicks =
      floor(clock_freq / prescaler / (1000.0 / float(totalPeriodMs)));
  unsigned long durationTicks =
      floor(clock_freq / prescaler / (1000.0 / float(onTimeMs)));

  if (periodTicks < 2) {
    periodTicks = 2;
  }

  if (durationTicks == 0) {
    durationTicks = 1;
  }

  if (durationTicks >= periodTicks) {
    durationTicks = periodTicks - 1;
  }

  OCR1A = (uint16_t)periodTicks;
  OCR1B = (uint16_t)(periodTicks - durationTicks);
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
  TIMSK1 &= ~((1 << OCIE1A) | (1 << OCIE1B));
  // disable interrupts on timer 2
  TIMSK2 &= ~(1 << OCIE2A);
  TCCR1B = 0;
  TCCR2B = 0;
  interrupts();
}

//============

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

//============

void loop() {
    if (readBinarySettings()) {
        doStimulation();
    }
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        doStimulation();
        newData = false;
    }
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        if (recvInProgress == false && Serial.peek() != startMarker) {
            return;
        }

        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void trimWhitespace(char * value) {
    if (value == NULL) {
        return;
    }

    char * original = value;

    while (*value == ' ' || *value == '\t') {
        ++value;
    }

    char * start = value;
    char * end = value + strlen(value);

    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t')) {
        --end;
    }

    *end = '\0';

    if (start != original) {
        memmove(original, start, strlen(start) + 1);
    }
}

bool readBinarySettings() {
    while (Serial.available() > 0) {
        int nextByte = Serial.peek();

        if (nextByte == '<') {
            return false;
        }

        if (nextByte != binaryMagic0) {
            Serial.read();
            continue;
        }

        if (Serial.available() < (int)sizeof(StimBinaryPayload)) {
            return false;
        }

        StimBinaryPayload payload;
        size_t bytesRead =
            Serial.readBytes((char *)&payload, sizeof(StimBinaryPayload));

        if (bytesRead != sizeof(StimBinaryPayload)) {
            return false;
        }

        if (payload.magic0 != binaryMagic0 || payload.magic1 != binaryMagic1 ||
            payload.version != binaryProtocolVersion ||
            payload.payloadSize != sizeof(StimBinaryPayload)) {
            continue;
        }

        inputPin = payload.inputPin;
        gatePin = payload.gatePin;
        outputPin = payload.outputPin;
        startTime = payload.startTime;
        stopTime = payload.stopTime;
        duration = payload.stimOnTime;
        interval = payload.stimOffTime;
        startRunning = payload.startRunning;
        return true;
    }

    return false;
}

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index
    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    if (strtokIndx == NULL) {
      return;
    }
    trimWhitespace(strtokIndx);
    strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
    
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    if (strtokIndx == NULL) {
      return;
    }
    trimWhitespace(strtokIndx);
    integerFromPC = atoi(strtokIndx);     // convert this part to an integer
 
    if ( strcmp(messageFromPC, "Start") == 0 ) {
      startTime = integerFromPC;
      if (debug) {
        Serial.println("startTime:");
        Serial.println(startTime);
      }
    }
    else if (strcmp(messageFromPC, "Stop") == 0){
      stopTime = integerFromPC;
      if (debug) {
        Serial.println("stopTime:");
        Serial.println(stopTime);
      }
    }
    else if (strcmp(messageFromPC, "InputPin") == 0){
      inputPin = integerFromPC;
      if (debug) {
        Serial.println("inputPin:");
        Serial.println(inputPin);
      }
    }
    else if (strcmp(messageFromPC, "GatePin") == 0){
      gatePin = integerFromPC;
      if (debug) {
        Serial.println("gatePin:");
        Serial.println(gatePin);
      }
    }
    else if (strcmp(messageFromPC, "OutputPin") == 0){
      outputPin = integerFromPC;
      if (debug) {
        Serial.println("outputPin:");
        Serial.println(outputPin);
      }
    }
    else if (strcmp(messageFromPC, "Duration") == 0){
      duration = integerFromPC;
      if (debug) {
        Serial.println("duration");
        Serial.println(duration);
      }
    }
    else if (strcmp(messageFromPC, "Interval") == 0){
      interval = integerFromPC;
      if (debug) {
        Serial.println("interval:");
        Serial.println(interval);
      }
    }
    else if (strcmp(messageFromPC, "StartRunning") == 0){
      startRunning = integerFromPC;
      if (debug) {
        Serial.println("startRunning");
        Serial.println(startRunning);
      }
    }
}

//============

void doStimulation() {
    if (startRunning != 0) {
      noInterrupts();
      ledPin = outputPin;
      pinMode(ledPin, OUTPUT);
      pinMode(LED_BUILTIN, OUTPUT);
      calculateCompareTimes(interval, duration);
      unsigned long start_at = (unsigned long)max(startTime, 0);
      unsigned long stop_at = (unsigned long)max(stopTime, startTime + 1);
      startCounting(start_at*1000, stop_at*1000);
      interrupts();
    }
    else {
      stopStimulation();
    }
}
