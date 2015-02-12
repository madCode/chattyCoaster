/* Simple test of the functionality of the photo resistor

Connect the photoresistor one leg to pin 0, and pin to +5V
Connect a resistor (around 10k is a good value, higher
values gives higher readings) from pin 0 to GND. (see appendix of arduino notebook page 37 for schematics).

----------------------------------------------------

           PhotoR     10K
 +5    o---/\/\/--.--/\/\/---o GND
                  |
 Pin 0 o-----------

----------------------------------------------------
*/

#include <Wtv020sd16p.h>

int resetPin = 2;  // The pin number of the reset pin.
int clockPin = 3;  // The pin number of the clock pin.
int dataPin = 4;  // The pin number of the data pin.
int busyPin = 5;  // The pin number of the busy pin.

/*
Create an instance of the Wtv020sd16p class.
 1st parameter: Reset pin number.
 2nd parameter: Clock pin number.
 3rd parameter: Data pin number.
 4th parameter: Busy pin number.
 */
Wtv020sd16p wtv020sd16p(resetPin,clockPin,dataPin,busyPin);

int sensorPin = 1;  //define a pin for Photo resistor
int ledBlue=11;     //define a pin for LED
int ledRed = 13;

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
unsigned int proceed = 0;

void setup()
{
    wtv020sd16p.reset();
    Serial.begin(9600);  //Begin serial communcation
    pinMode( ledBlue, OUTPUT );
    pinMode( ledRed, OUTPUT);
    timeThreshCount = 0;
    //threshFunc = &blinkLed;
    threshFunc = &playSong;
}

void loop()
{
    //Serial.println(analogRead(sensorPin)); //Write the value of the photoresistor to the serial monitor.
    int light = analogRead(sensorPin);
    if (light < 400){
      analogWrite(ledBlue, 255);
      proceed = 1;
    }
    else{
      analogWrite(ledBlue, 0);
      proceed = 0;
    }
   //delay(10); //short delay for faster response to light.
   if (proceed) {
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
}

void blinkLed() {
    /* Intentionally block thread, do not listen during blocking */
    digitalWrite(ledRed, HIGH);
    delay(2000);
    digitalWrite(ledRed, LOW);
}

void playSong()
{
  wtv020sd16p.asyncPlayVoice(0);
  delay(2000);
  wtv020sd16p.pauseVoice();
}
