/***************************************************************************
  Adafruit AS7262 6-Channel Visible Light Sensor for photometer
  I2C address 0x49
  Sensor will read all wavelengths at 1 second intervals, as well as sensor temperature
  WIll then write data to file 'read.txt'
  Please start serial plotter to view graphed data
  Adapted from "AS7262_test" Written by Dean Miller for Adafruit Industries.
  Must start with LED switched off!
 ***************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_AS726x.h"
#include "Adafruit_TSL2561_U.h" //Lux
#include "Adafruit_CCS811.h" //CO2
#include "Adafruit_VEML6075.h" //UV
//Methane through A1
const int pinInput = 2;
const int LEDpin = 3;
const int analoguePin = A1;
long int previous=0;
int val = 0;
int MethaneRaw;
int Methaneppb;
File myFile;
Adafruit_CCS811 CCS;
Adafruit_AS726x ams;
Adafruit_VEML6075 uv = Adafruit_VEML6075();
//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];
//buffer to hold calibrated values
float calibratedValues[AS726x_NUM_CHANNELS];
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LEDpin, OUTPUT);
  pinMode(pinInput, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  //begin and make sure we can talk to the sensor
  if (!ams.begin()) {
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  initSD();
  initTSL();
  initVEML();
  digitalWrite(LEDpin, HIGH);
}
void loop() {
  if (millis()-previous>=1000)
  {
    previous=millis();
    myFile=SD.open("ENV2.txt",FILE_WRITE);
    CCS.readData();
    sensors_event_t event;  // Starting new sensor  events
    tsl.getEvent(&event);
    MethaneRaw = analogRead(A1);
    Methaneppb = Methane_ppb(MethaneRaw);

    if (myFile)
    {
      if (event.light)
      {
        myFile.print(event.light);
      }
      else
      {
        myFile.print("LUXOVL");
      }

      myFile.print(CCS.geteCO2()); myFile.print(", ");
      myFile.print(Methaneppb); myFile.print(", ");
      myFile.print(uv.readUVA()); myFile.print(", ");
      myFile.println(uv.readUVB());
    }
    myFile.close();
  }
  
  if (digitalRead(pinInput) == LOW)
  {
    ams.startMeasurement(); //begin a measurement
    bool rdy = false;  //wait till data is available
    while (!rdy) {
      rdy = ams.dataReady();
    }
    delay(1900);
    myFile = SD.open("FDA.txt", FILE_WRITE);
    if (myFile)
    {
      for (int i = 0; i < 3; i++)
      {
        //read the values!
        ams.readRawValues(sensorValues);
        ams.readCalibratedValues(calibratedValues);

        myFile.print(String(i)+": ");
        myFile.print(sensorValues[AS726x_VIOLET],2); myFile.print(", ");
        myFile.print(sensorValues[AS726x_BLUE],2); myFile.print(", ");
        myFile.print(sensorValues[AS726x_GREEN],2); myFile.print(", ");
        myFile.print(sensorValues[AS726x_YELLOW],2); myFile.print(", ");
        myFile.print(sensorValues[AS726x_ORANGE],2); myFile.print(", ");
        myFile.print(sensorValues[AS726x_RED],2); myFile.print(", ");
        myFile.print(calibratedValues[AS726x_VIOLET],2); myFile.print(", ");
        myFile.print(calibratedValues[AS726x_BLUE],2); myFile.print(", ");
        myFile.print(calibratedValues[AS726x_GREEN],2); myFile.print(", ");
        myFile.print(calibratedValues[AS726x_YELLOW],2); myFile.print(", ");
        myFile.print(calibratedValues[AS726x_ORANGE],2); myFile.print(", ");
        myFile.println(calibratedValues[AS726x_RED],2);
      }
        myFile.close();
    }
    else
    {
      // if the file didn't open, print an error:
      while (1)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      }
    }
  }
}

void initTSL()
{
  if (!tsl.begin())
  {
    Serial.print("No TSL2561 detected, check wiring or I2C ADDR!");
    //while (1);
  }
}

void initSD()
{
  if (!SD.begin(8)) {
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("FDA.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println("New session opened - FDA");
    myFile.println("Violet, Blue, Green, Yellow, Orange, Red, CViolet, CBlue, CGreen, CYellow, COrange, CRed");
    // close the file:
    myFile.close();
  } else {
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  myFile = SD.open("ENV2.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println("New session opened - ENV2");
    myFile.println("Light [Lx], CO2 [ppm], Methane [ppb], UVA, UVB");
    // close the file:
    myFile.close();
  } else {
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }
}

int Methane_ppb(double Rawval)
{
  double ppb = (10.938 * exp(1.7742 * (Rawval * 5 / 4095))) * 1000;
  return ppb;
}

void initVEML()
  {
  // VEML6075 start up code
  if (!uv.begin()) {
    Serial.println("No VEML6075 detected, check wiring or I2C ADDR!");
  }
  // uv.setHighDynamic(false);
  uv.setForcedMode(false);
  // Set the calibration coefficients
  uv.setCoefficients(2.22, 1.33,  // UVA_A and UVA_B coefficients
                     2.95, 1.74,  // UVB_C and UVB_D coefficients
                     0.001461, 0.002591); // UVA and UVB responses
  }
