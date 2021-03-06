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
const float volThresh = 0.5f;

/* Function pointer to a function to activate after silence threshold
 * is achieved. */
void (*threshFunc)(void) = NULL;

/* Threshold function declarations */
void blinkLed();
void printThresh();

/* Do not speak again for COOLDOWN milliseconds after speaking. */
int cooldown;
const int cooldownTime = 3000;

unsigned int sample;
unsigned int proceed = 0;

const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

int DISPLAY_TIME = 10;  // In milliseconds

int color = 0;
int currColorVal;
unsigned int currAudio;
const unsigned int maxAudioFile = 15; 

void setup()
{
	currAudio = 0;
    wtv020sd16p.reset();
    Serial.begin(9600);  //Begin serial communcation
    pinMode( ledRed, OUTPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    timeThreshCount = 0;
    //threshFunc = &blinkLed;
    threshFunc = &playSong;
	currAudio = 0;
}

void loop()
{
    Serial.println(analogRead(sensorPin)); //Write the value of the photoresistor to the serial monitor.
    int light = analogRead(sensorPin);
    if (light < 600) {
      proceed = 1;
	  if (currColorVal < 768){
        showRGB(currColorVal);  
        currColorVal += 2;  
      } else {
        currColorVal = 0;
        showRGB(currColorVal);
      }
    }
    else{
      proceed = 0;
	  lightsOff();
    }
    
    delay(DISPLAY_TIME);
	Serial.print("currColorVal: ");
	Serial.println(currColorVal);

   if (proceed) {
      unsigned long startMillis= millis();  // Start of sample window
      unsigned int peakToPeak = 0;   // peak-to-peak level
  
      unsigned int signalMax = 0;
      unsigned int signalMin = 1024;
  	
      double volts = audioSample(startMillis);
      Serial.print("Volts: ");
      Serial.print(volts);
      Serial.print(". Cooldown: ");
      Serial.println(cooldown);
  
      if (volts <= volThresh) {
          timeThreshCount++;
      } else {
          timeThreshCount = 0;
      }
  
  	if (cooldown > 0) {
  		cooldown -= sampleWindow;
  	}
  
      if (timeThreshCount >= timeThresh && cooldown <= 0) {
          threshFunc();
  	  cooldown = cooldownTime;
          timeThreshCount = 0;
      }
   }
}

double audioSample(unsigned long startMillis) {
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
	return (peakToPeak * 3.3) /1024; // convert to volts
}

void blinkLed() {
    /* Intentionally block thread, do not listen during blocking */
    digitalWrite(ledRed, HIGH);
    delay(2000);
    digitalWrite(ledRed, LOW);
}

void showRGB(int color)
{

if (color <= 255)          // zone 1
{
	digitalWrite(RED_PIN, HIGH);
	digitalWrite(BLUE_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
}
else if (color <= 511)     // zone 2
{
	digitalWrite(RED_PIN, LOW);
	digitalWrite(BLUE_PIN, HIGH);
	digitalWrite(GREEN_PIN, LOW);
}
else // color >= 512       // zone 3
{
	digitalWrite(RED_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);
	digitalWrite(GREEN_PIN, HIGH);
}
}

void lightsOff() {
	digitalWrite(RED_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
}

void playSong()
{
	wtv020sd16p.asyncPlayVoice(currAudio);
	if (++currAudio >= maxAudioFile) {
		currAudio = 0;
	}
}


void playSongClipped()
{
  wtv020sd16p.asyncPlayVoice(0);
  delay(2000);
  wtv020sd16p.pauseVoice();
}
