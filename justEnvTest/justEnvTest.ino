#include <SparkFun_VEML6075_Arduino_Library.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_TSL2561_U.h"
#include "Adafruit_VEML6075.h"
#include "SparkFunBME280.h"
#include "Adafruit_CCS811.h"
#include "SoftwareSerial.h"
#include "Adafruit_AS726x.h"

const int LEDpin = 13;
int analoguePin = A1;
int val = 0;
int MethaneRaw;
int Methaneppb;
BME280 mySensor;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_VEML6075 uv = Adafruit_VEML6075();
Adafruit_CCS811 ccs;
const int chipSelect = 8;
File test;

long int previousTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("starting");
  while (!Serial) {
    Serial.println("in loop");
  }
  pinMode(analoguePin, INPUT);  //pin for methane sensor readings

  //Initialise SD card and test
  Serial.println("Initializing SD card...");
  if (!SD.begin(8)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  test = SD.open("TEST.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  while (!test)
  {
    Serial.println("Opening test.txt...");
  }
  Serial.print("Writing to .txt...");
  test.println("New session opened - test");
  // close the file:
  test.close();
  Serial.println("done.");


  // TSL2561 start up code  //use tsl.begin() to default to Wire, //tsl.begin(&Wire2) directs api to use Wire2, etc.
  if (!tsl.begin())
  {
    Serial.print("No TSL2561 detected, check wiring or I2C ADDR!");
    //while (1);
  }

  //BMS280 start up code:
  Wire.begin();
  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("No BME280 detected, check wiring or I2C ADDR!");
    //while (1);
  }
  // VEML6075 start up code
  configureSensor();
  if (! uv.begin()) {
    Serial.println("No VEML6075 detected, check wiring or I2C ADDR!");
  }
  // uv.setHighDynamic(false);
  uv.setForcedMode(false);
  // Set the calibration coefficients
  uv.setCoefficients(2.22, 1.33,  // UVA_A and UVA_B coefficients
                     2.95, 1.74,  // UVB_C and UVB_D coefficients
                     0.001461, 0.002591); // UVA and UVB responses
}

void loop() {
  if (millis() - previousTime >= 1000)
  {
    previousTime = millis();
    test = SD.open("TEST.txt", FILE_WRITE); //Opening file on SD card
    sensors_event_t event;  // Starting new sensor  events
    tsl.getEvent(&event);
    MethaneRaw = analogRead(A1);
    Methaneppb = Methane_ppb(MethaneRaw);
    test.print("Time: " + String(millis()) + ", ");
    if (test) {    // write data to SD card
      if (event.light)
      { test.print("light: "); test.print(event.light); test.print(" lux, ");
      }
      else
      {
        test.print("Lux Sensor overload");
      }

      test.print("Hum: ");  test.print(mySensor.readFloatHumidity(), 0); test.print("%, ");
      test.print("Pres: ");  test.print(mySensor.readFloatPressure(), 0); test.print(" Pa, ");
      //test.print(" Alt: ");  test.print(mySensor.readFloatAltitudeMeters(), 1); test.print(" m, ");
      test.print("Temp: ");  test.print(mySensor.readTempC(), 2); test.print("°C, ");
      test.print("UVA: "); test.print(uv.readUVA()); test.print(", ");
      test.print("UVB: "); test.print(uv.readUVB()); test.print(", ");
      // test.print("UVI: "); test.print(uv.readUVI()); test.print(", ");
      // test.print("Meth_Raw: "); test.println(MethaneRaw); test.print(" ppb, ");
      test.print("Meth_ppb: "); test.println(Methaneppb); test.print(" ppb, ");
      test.print("CO2: ");  test.print(ccs.geteCO2()); test.print(" ppm, ");
      //test.print("TVOC: ");  test.print(ccs.getTVOC()); test.print(" ppb, ");
      test.println();
    }
    else
    {
      Serial.println("Error opening file in loop.");
    }
    // close the file
      test.close();
  }

}

void configureSensor()
{
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
}

int Methane_ppb(double Rawval) {
  double ppb = (10.938 * exp(1.7742 * (Rawval * 5 / 4095))) * 1000;
  return ppb;
}
