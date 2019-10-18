#include <SparkFun_VEML6075_Arduino_Library.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
//#include "Adafruit_TSL2561_U.h"
//#include "Adafruit_VEML6075.h"
#include "SparkFunBME280.h" //Pressure, Humidity, Temp
//#include "Adafruit_CCS811.h"
#include "SoftwareSerial.h"
#include "AS726X.h"

const int pinInput = 2;

long int previous = 0;

const int chipSelect = 8;
File card;
BME280 BME;
//Adafruit_CCS811 CCS;
//Adafruit_VEML6075 uv = Adafruit_VEML6075();
AS726X sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();
  pinMode(pinInput, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  //Serial.println("Starting environmental sensors");
  initSD();
  initBME();
  //initVEML();
}

void loop()
{
  if (millis() - previous >= 1000)
  {
    previous = millis();

    card = SD.open("ENV.txt", FILE_WRITE);
    if (card)
    {
      //Serial.println("Printing to file");
      //CCS.readData();
      //MethaneRaw = analogRead(A1);
      //Methaneppb = Methane_ppb(MethaneRaw);
      card.print(BME.readFloatHumidity(), 0); card.print(", ");
      card.print(BME.readFloatPressure(), 0); card.print(", ");
      card.print(BME.readTempC(), 2); card.print(", ");
      //card.print(CCS.geteCO2()); card.print(", ");
      //card.print(Methaneppb);
      card.println();
      //card.print("UVA: "); card.print(uv.readUVA()); card.print(", ");
      //card.print("UVB: "); card.println(uv.readUVB());
    }
    else
    {
      //Serial.println("Can't open in loop");
    }
    card.close();
  }

  if (digitalRead(pinInput) == LOW)
  {
    //Serial.println("BCA logging");
    delay(1900);
    card = SD.open("BCA.txt", FILE_WRITE);

    if (card)
    {
      for (int i = 0; i < 3; i++)
      {
        sensor.takeMeasurements();
        card.print(String(i) + ": ");
        card.print(sensor.getViolet(), 2); card.print(", ");
        card.print(sensor.getCalibratedViolet(), 2); card.print(", ");
        card.print(sensor.getBlue(), 2); card.print(", ");
        card.println(sensor.getCalibratedBlue(), 2);
      }
      card.close();
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

void initSD()
{
  if (!SD.begin(8)) {
    while (1)
    {
      //Serial.println("Can't initialise SD");
    }
  }

  card = SD.open("ENV.txt", FILE_WRITE);
  if (card) {
    card.println("New session opened - ENV");
    card.println("Humidity [%], Pressure [Pa], Temp. [°C], CO2 [ppm], Methane [ppb]");
    // close the file:
    card.close();
    //Serial.println("ENV.txt initialised");
  }
  else
  {
    while (1)
    {
      //Serial.println("Can't open ENV.txt");
    }
  }

  card = SD.open("BCA.txt", FILE_WRITE);
  if (card) {
    card.println("New session opened - BCA");
    card.println("Violet, CViolet, Blue, CBlue");
    // close the file:
    card.close();
    //Serial.println("BCA.txt initialised");
  }
  else
  {
    while (1)
    {
      //Serial.println("Can't open BCA.txt");
    }
  }
}

void initBME()
{
  Wire.begin();
  if (BME.beginI2C() == false) //Begin communication over I2C
  {
    //Serial.println("No BME280 detected, check wiring or I2C ADDR!");
  }
}

/*int Methane_ppb(double Rawval)
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
  }*/

