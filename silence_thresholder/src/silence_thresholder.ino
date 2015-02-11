/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

/* DECLARATIONS */

/* Discretize audio input to one sample every SAMPLE_WINDOW mS:
 * e.g. 50mS = 20Hz sample rate. */
const int sampleWindow = 50; 

/* After TIME_THRESH sampleWindows below VOL_THRESH, trigger event
 * 20Hz * 5S wait time = 100 */
const int timeThresh = 100;
int timeThreshCount;

/* Threshold for counting sample voltage as true noise. WARNING: need
 * to adjust based on audio input's hardware. */
const float volThresh = 1.0f;

/* Function pointer to a function to activate after silence threshold
 * is achieved. */
void (*threshFunc)(void) = NULL;

/* Threshold function declarations */
void blinkLed();
void printThresh();

unsigned int sample;

/* Pin declarations */
int led = 13;

/* MAIN CODE */

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(9600);
    timeThreshCount = 0;
    // threshFunc = &printThresh;
    threshFunc = &blinkLed;
}


void loop() {
    unsigned long startMillis= millis();  // Start of sample window
    unsigned int peakToPeak = 0;   // peak-to-peak level

    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow)
    {
        sample = analogRead(0);
        if (sample < 1024)  // toss out spurious readings
        {
            if (sample > signalMax)
            {
                signalMax = sample;  // save just the max levels
            }
            else if (sample < signalMin)
            {
                signalMin = sample;  // save just the min levels
            }
        }
    }
    peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

    Serial.println(volts);

    if (volts <= volThresh) {
        timeThreshCount++;
    } else {
        timeThreshCount = 0;
    }

    if (timeThreshCount >= timeThresh) {
        threshFunc();
        timeThreshCount = 0;
    }
        
}

void blinkLed() {
    /* Intentionally block thread, do not listen during blocking */
    digitalWrite(led, HIGH);
    delay(2000);
    digitalWrite(led, LOW);
}

void printThresh() {
    Serial.println("*********REACHED A THRESHOLD!**********");
}
