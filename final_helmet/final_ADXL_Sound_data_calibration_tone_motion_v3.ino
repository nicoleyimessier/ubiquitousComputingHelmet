/*Parts of this code were used from the "Example Sound Level Sketch for the
 Adafruit Microphone Amplifier" & "Sparkfun ADXL3xx code"*/

#include "SD.h"
#include <Wire.h>
#include "RTClib.h"
#include "pitches.h"

#define redLEDpin 7
#define speaker 6
#define silentTrigger 4 

//Accelerometer limits
int xMin; //Minimum x Value
int xMax; //Maximum x Value
int xVal; //Current x Value

int yMin; //Minimum y Value
int yMax; //Maximum y Value
int yVal; //Current y Value

int zMin; //Minimum z Value
int zMax; //Maximum z Value
int zVal; //Current z Value


RTC_DS1307 RTC;  //Declaring RTC device

const int chipSelect = 10;

//AMP Constants
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
float X,Y,Z;

//calibrate variables
const unsigned int X_AXIS_PIN = A0;
const unsigned int Y_AXIS_PIN = A1;
const unsigned int Z_AXIS_PIN = A2;

//motion detection
boolean moveDetected=false; // When motion is detected - changes to true
int tolerance=5; // Sensitivity of the motion
const int buttonPin=2;
boolean calibrated=false; // When accelerometer is calibrated - changes to true 


void setup()
{
  Serial.begin(9600);
  pinMode(2,OUTPUT);
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  pinMode(0, OUTPUT);   //For sleep on the accelerometer
  digitalWrite(0, HIGH); 

  pinMode(1, OUTPUT);   //For 6g select on the accelerometer
  digitalWrite(1, HIGH); 

  pinMode(redLEDpin, OUTPUT);

  pinMode(buttonPin, INPUT); 
  
  pinMode(speaker, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  float X,Y,Z;
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("Date,Timestamp,millis,X,Y,Z,Volts,peakToPeak");
  dataFile.close();
  Serial.println("Date  Timestamp  millis   X   Y   Z   Volts   peakToPeak");

}

void loop()
{ 

  //  ----------------CALIBRATE----------------------
  const int x = analogRead(X_AXIS_PIN);
  const int y = analogRead(Y_AXIS_PIN);
  const int z = analogRead(Z_AXIS_PIN);

  Serial.print("button: ");
  Serial.println(digitalRead(buttonPin));

  if(digitalRead(buttonPin)==1){
    calibrateAccel();
  }

  Serial.print("x(");
  Serial.print(xMin);
  Serial.print("/");
  Serial.print(xMax);
  Serial.print("), y(");
  Serial.print(yMin);
  Serial.print("/");
  Serial.print(yMax);
  Serial.print("), z(");
  Serial.print(zMin);
  Serial.print("/");
  Serial.print(zMax);
  Serial.println(")");


  //  ---------------------END Calibrate---------------------------------



  // --------------------ADXL------------------------------------
  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    delay(10);
    int sensor = analogRead(analogPin);

    if(analogPin==0)
    {
      X=sensor;
    }

    if(analogPin==1)
    {
      Y=sensor;
    }

    if(analogPin==2)
    {
      Z=sensor;
    }



  }
  //--------------------AMP----------------------------------------
  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(3);
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

  //testing sound with red led pin
  if (peakToPeak > 200) {
    digitalWrite(redLEDpin, HIGH); 
  }
  else {
    digitalWrite(redLEDpin, LOW);
  }

  //trigger sound off with a certain frequency
  if (peakToPeak > 150) {
    digitalWrite(2,LOW);
    Serial.println("volume_sound: low");
  }
  else {
    digitalWrite(2, HIGH);
    Serial.println("volume_sound: high");
  }


  Wire.begin();
  DateTime now;

  // --------------------Data Logging------------------------------------
  // open the file.
  File dataFile = SD.open("test1.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {

    // fetch the time
    now = RTC.now();

    dataFile.print(now.year(), DEC);
    dataFile.print("/");
    dataFile.print(now.month(), DEC);
    dataFile.print("/");
    dataFile.print(now.day(), DEC);
    dataFile.print(",");
    dataFile.print(now.hour(), DEC);
    dataFile.print(":");
    dataFile.print(now.minute(), DEC);
    dataFile.print(":");
    dataFile.print(now.second(), DEC);
    dataFile.print(",");
    dataFile.print(millis());
    dataFile.print(",");

    dataFile.print(X);
    dataFile.print(",");
    dataFile.print(Y);
    dataFile.print(",");
    dataFile.print(Z);
    dataFile.print(",");
    dataFile.print(volts);
    dataFile.print(",");
    dataFile.print(peakToPeak);
    dataFile.print(",");
    dataFile.print(xMin); 
    dataFile.print(",");
    dataFile.print(xMax);
    dataFile.print(",");
    dataFile.print(yMin);
    dataFile.print(",");
    dataFile.print(yMax);
    dataFile.print(",");
    dataFile.print(zMin);
    dataFile.print(",");
    dataFile.println(zMax);

    dataFile.close();

    // print to the serial port too:


    // log time
    // Serial.print(now.unixtime()); // seconds since 1970
    // Serial.print(", ");
    Serial.println(" ");
    Serial.print(now.year(), DEC);
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(", ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print(", ");
    Serial.print(millis(), DEC);
    Serial.print(", ");

    Serial.print(X);
    Serial.print(", ");
    Serial.print(Y);
    Serial.print(", ");
    Serial.print(Z);
    Serial.print(", ");
    Serial.print(volts);
    Serial.print(", ");
    Serial.println(peakToPeak);


  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  //-------------Motion Detection ----------------------

  Serial.print("motion detection: ");
  Serial.println(moveDetected);

  if(calibrated){
    xVal = analogRead(X_AXIS_PIN);
    yVal = analogRead(Y_AXIS_PIN);
    zVal = analogRead(Z_AXIS_PIN);

    if(xVal >(xMax+tolerance)||xVal < (xMin-tolerance)){
      Serial.println("X MOVING");
      digitalWrite(redLEDpin, HIGH);
      digitalWrite(2,LOW);
      Serial.println("volume_moving: low");
      tone(speaker, NOTE_A4);
    }
    else {
      digitalWrite(2, HIGH);
      Serial.println("volume_moving: high");
      noTone(speaker);
    }


    if(yVal >(yMax+tolerance)||yVal < (yMin-tolerance)){
      Serial.println("Y MOVING");
      digitalWrite(redLEDpin, HIGH);
      digitalWrite(2,LOW);
      Serial.println("volume_moving: low");
      tone(speaker, NOTE_A4);
    }
    else {
      digitalWrite(2, HIGH);
      Serial.println("volume_moving: high");
      noTone(speaker);
    }



    if(zVal >(zMax+tolerance)||zVal < (zMin-tolerance)){
      Serial.println("Z MOVING");
      digitalWrite(redLEDpin, HIGH);
      digitalWrite(2,LOW);
      Serial.println("volume_moving: low");
      tone(speaker, NOTE_A4);
    }
    else {
      digitalWrite(2, HIGH);
      Serial.println("volume_moving: high");
      noTone(speaker);
    }


    delay(1000);
  }


}

// ------------Calibrate-----------------------------------------
void calibrateAccel(){

  //initialise x,y,z variables
  xVal = analogRead(X_AXIS_PIN);
  xMin = xVal;
  xMax = xVal;

  yVal = analogRead(Y_AXIS_PIN);
  yMin = yVal;
  yMax = yVal;

  zVal = analogRead(Z_AXIS_PIN);
  zMin = zVal;
  zMax = zVal;

  Serial.println("CALIBRATION STARTED");

  //calibrate the Accelerometer (should take about 0.5 seconds)
  for (int i=0; i<50; i++){
    // Calibrate X Values
    xVal = analogRead(X_AXIS_PIN);
    if(xVal>xMax){
      xMax=xVal;
    }
    else if (xVal < xMin){
      xMin=xVal;
    }

    // Calibrate Y Values
    yVal = analogRead(Y_AXIS_PIN);
    if(yVal>yMax){
      yMax=yVal;
    }
    else if (yVal < yMin){
      yMin=yVal;
    }

    // Calibrate Z Values
    zVal = analogRead(Z_AXIS_PIN);
    if(zVal>zMax){
      zMax=zVal;
    }
    else if (zVal < zMin){
      zMin=zVal;
    }

    //Delay 10msec between readings
    delay(10);
  }

  //End of calibration sequence sound. ARMED.
  Serial.println("CALIBRATION COMPLETE");
  calibrated=true;

}



















