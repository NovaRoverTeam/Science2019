/* As of 31/05
*/

#include <Servo.h>

const int pinDirA = 12; //Drill direction pin
const int pinDirB = 13; //Actuator direction pin
const int pinSpdA = 10; //Drill speed pin
const int pinSpdB = 11; //Actuator speed pin
int pinEnc = 2; //Encoder input pin
const int pinSieve = 22; //Barrel sieve control pin
const int pinSievePumps = 24; //Pumps on either side of wet sieve control pin
const int pinCuvettePumps = 26; //Pumps to feed liquid into cuvettes pin
const int pinMixers = 28; //Mixing motors to stir vats
const int pinSpectro = 4;
const int pinStepEn = 5;
const int pinStepDir = 6;
const int pinStepPulse = 7;
const int stepsPerRev = 400; //half-stepping

//Assuming that ground level is at 0 and there are 660 pulses per cm
const long int intMin = -6000; //Lowest allowable point past zero when safety is on
const long int int5cm = 3300; //5cm above ground level
const long int int10cm = 6600; //10cm above ground level
const long int int30cm = 20000; //Point at which auger should clear chute
const long int intMax = 24500; //Highest allowable point past zero when safety is on
const int intRatio = 76; //Speed at which drill can spin at ratio

const int timeFlush = 3250; //Time to flush cuvette holder pipes in ms
const int timeFill = 2250; //Time to fill cuvette in ms to 3 mL
//const int timeFill = 750;

String strInput; //Initialise variable for serial input
long int intPos = 0; //Encoder pulses from 0; can be negative or positive
bool safety = false; //Safety off by default
bool autoRetract = false; //True when executing automatic retraction
bool autoDrill = false; //True executing automatic drilling
bool autoExtract = false; //True when executing automatic extraction
//long int intTarget; //Relevant when on auto mode; custom target position
bool goingUp = false; //Tracks what direction the actuator travels in to update encoder count
bool encoderFlag = false; //True when encoder count should be updated
bool serialFlag = false; //True when command is queued

int intStepCount = 0;
int intSteps;

void setup()
{
  Serial.begin(9600);
  pinMode(pinStepEn, OUTPUT);
  digitalWrite(pinStepEn, HIGH); //Disable stepper
  pinMode(pinDirA, OUTPUT);
  pinMode(pinDirB, OUTPUT);
  pinMode(pinSpdA, OUTPUT);
  pinMode(pinSpdB, OUTPUT);
  pinMode(pinEnc, INPUT_PULLUP);
  pinMode(pinSieve, OUTPUT);
  pinMode(pinSievePumps, OUTPUT);
  pinMode(pinCuvettePumps, OUTPUT);
  pinMode(pinMixers, OUTPUT);
  pinMode(pinSpectro, OUTPUT);
  pinMode(pinStepDir, OUTPUT);
  pinMode(pinStepPulse, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinEnc), flag, RISING);
  digitalWrite(pinSieve, LOW); //Sieve off
  digitalWrite(pinDirA, LOW); //Reverse drill
  digitalWrite(pinDirB, LOW); //Actuator going down
  analogWrite(pinSpdA, 0); //Cut power to drill
  analogWrite(pinSpdB, 0); //Cut power to actuator
  digitalWrite(pinSpectro, HIGH);
  digitalWrite(pinStepDir, LOW); //Stepper CCW
}

void flag()
{
  encoderFlag = true; //Trip flag when encoder pulses
}

void loop()
{
  if (encoderFlag) //If encoder flagged
  {
    if (goingUp) //Increment or decrement based on direction
    {
      intPos++; //Increment by one
    }
    else
    {
      intPos--; //Otherwise decrement by one
    }
  }

  if (Serial.available() > 0)
  {
    strInput = Serial.readString(); //Read command from serial line
    serialFlag = true; //Command should be executed
  }

  if (serialFlag)
  {
    if (strInput == "2") //Automated sequence to drill down and then stop 10cm deep
    {
      goingUp = false;
      autoDrill = true;
      autoRetract = false;
      autoExtract = false;
      digitalWrite(pinDirA, HIGH); //Drill forward
      digitalWrite(pinDirB, LOW); //Actuator down
      analogWrite(pinSpdA, intRatio); //Drill at ratio
      analogWrite(pinSpdB, 255); //Actuator full power
      Serial.println("2");
    }
    else if (strInput == "3") //Automated sequence to extract soil
    {
      goingUp = true;
      autoExtract = true;
      autoRetract = false;
      autoDrill = false;
      digitalWrite(pinDirB, HIGH); //Actuator up
      analogWrite(pinSpdA, 0); //Cut drill power
      analogWrite(pinSpdB, 255); //Actuator full power
      Serial.println("3");
    }
    else if (strInput == "4") //Kill switch; stops autopilots, drill and actuator
    {
      analogWrite(pinSpdA, 0);
      analogWrite(pinSpdB, 0);
      autoRetract = false; //Stop retraction autopilot
      autoDrill = false; //Stop drilling autopilot
      autoExtract = false; //Stop extraction autopilot
      Serial.println("4");
    }
    else if (strInput == "5") //Safety off
    {
      safety = false;
      Serial.println("5");
    }
    else if (strInput == "6") //Safety on
    {
      safety = true;
      Serial.println("6");
    }
    else if (!safety)
    {
      //Unsafe commands enabled
      if (strInput == "0") //Zero encoder count
      {
        intPos = 0;
        Serial.println("0");
      }
      else if (strInput == "1") //Automatic retraction
      {
        intPos = 0; //Zero encoder
        safety = true; //Turn on safety
        autoRetract = true; //Autopilot
        autoDrill = false;
        autoExtract = false;
        goingUp = true;

        digitalWrite(pinDirB, HIGH); //Actuator goes up
        analogWrite(pinSpdA, 0); //Cut drill power
        analogWrite(pinSpdB, 255); //Full actuator power

        Serial.println("1");
      }
      else if (strInput == "7") //Drill down at ratio, full power
      {
        goingUp = false;
        digitalWrite(pinDirA, HIGH); //Normal drill
        digitalWrite(pinDirB, LOW); //Actuator going down
        analogWrite(pinSpdA, 76); //Drill at ratio
        analogWrite(pinSpdB, 255); //Full actuator power
        Serial.println("7");
      }
      else if (strInput == "8") //Retract drill at ratio, full power
      {
        goingUp = true;
        digitalWrite(pinDirA, LOW); //Reverse drill
        digitalWrite(pinDirB, HIGH); //Actuator going up
        analogWrite(pinSpdA, 76); //Full drill power
        analogWrite(pinSpdB, 255); //Full actuator power
        Serial.println("8");
      }
      else if (strInput == "9") //Direct actuator up
      {
        goingUp = true;
        digitalWrite(pinDirB, HIGH);
        Serial.println("9");
      }
      else if (strInput == "10") //Direct actuator down
      {
        goingUp = false;
        digitalWrite(pinDirB, LOW);
        Serial.println("10");
      }
      else if (strInput == "11") //Direct drill forwards
      {
        digitalWrite(pinDirA, HIGH);
      }
      else if (strInput == "12") //Direct drill reverse
      {
        digitalWrite(pinDirA, LOW);
      }
      else if (strInput == "13") //Cut actuator power
      {
        analogWrite(pinSpdB, 0);
      }
      else if (strInput == "14") //Actuator power at one third
      {
        analogWrite(pinSpdB, 85);
      }
      else if (strInput == "15") //Actuator power at half
      {
        analogWrite(pinSpdB, 128);
      }
      else if (strInput == "16") //Actuator power at two thirds
      {
        analogWrite(pinSpdB, 170);
      }
      else if (strInput == "17") //Actuator power full
      {
        analogWrite(pinSpdB, 255);
      }
      else if (strInput == "18") //Cut drill power
      {
        analogWrite(pinSpdA, 0);
      }
      else if (strInput == "19") //Drill power at one third
      {
        analogWrite(pinSpdA, 85);
      }
      else if (strInput == "20") //Drill power at half
      {
        analogWrite(pinSpdA, 128);
      }
      else if (strInput == "21") //Drill power at two thirds
      {
        analogWrite(pinSpdA, 170);
      }
      else if (strInput == "22") //Drill power full
      {
        analogWrite(pinSpdA, 255);
      }
      else if (strInput == "23")
      {
        goingUp = false;
        digitalWrite(pinDirA, HIGH);
        digitalWrite(pinDirB, LOW);
        analogWrite(pinSpdA, 255);
        analogWrite(pinSpdB, 128);
        Serial.println("23");
      }
      else if (strInput == "24") //Turn on barrel sieve
      {
        digitalWrite(pinSieve, HIGH);
      }
      else if (strInput == "25") //Turn off barrel sieve
      {
        digitalWrite(pinSieve, LOW);
      }
      else if (strInput == "26") //Turn on wet sieve pumps
      {
        digitalWrite(pinSievePumps, HIGH);
      }
      else if (strInput == "27") //Turn off wet sieve pumps
      {
        digitalWrite(pinSievePumps, LOW);
      }
      else if (strInput == "28") //Turn on cuvette pumps
      {
        digitalWrite(pinCuvettePumps, HIGH);
      }
      else if (strInput == "29") //Turn off cuvette pumps
      {
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (strInput == "30") //Turn on mixers
      {
        digitalWrite(pinMixers, HIGH);
      }
      else if (strInput == "31") //Turn off mixers
      {
        digitalWrite(pinMixers, LOW);
      }
      else if (strInput == "32")
      {
        digitalWrite(pinCuvettePumps, HIGH);
        delay(timeFlush + timeFill);
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (strInput == "33")
      {
        digitalWrite(pinCuvettePumps, HIGH);
        delay(timeFill);
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (strInput == "34") //Fill three pairs of cuvettes
      {
        //Assume that cuvettes are already lined up
        digitalWrite(pinCuvettePumps, HIGH);
        delay(timeFlush);

        for (int i = 0; i < 3; i++)
        {
          digitalWrite(pinCuvettePumps, HIGH);
          delay(timeFill);
          digitalWrite(pinCuvettePumps, LOW);
          delay(300);

          digitalWrite(pinStepEn, LOW); //Enable stepper
          for (int j = 0; j < getSteps(i); j++)
          {
            digitalWrite(pinStepPulse, HIGH); //These four lines result in 1 step
            delayMicroseconds(2500);
            digitalWrite(pinStepPulse, LOW);
            delayMicroseconds(2500);
            intStepCount++;
          }
          digitalWrite(pinStepEn, HIGH); //Disable stepper
          delay(100);
        }
        delay(300);
        fixSteps(intStepCount,stepsPerRev/4); //Fix drift
        intStepCount=0; //Reset count
        
        digitalWrite(pinStepEn, HIGH); //Disable stepper
      }
      else if (strInput == "35") //Scan twelve cuvettes
      {
        digitalWrite(pinStepEn, LOW); //Enable stepper
        for (int i = 0; i < 12; i++)
        {
          //Scan once
          digitalWrite(pinSpectro, LOW);
          delay(50);
          digitalWrite(pinSpectro, HIGH);
          delay(2450);

          //Rotate to next cuvette
          for (int j = 0; j < getSteps(i); j++)
          {
            digitalWrite(pinStepPulse, HIGH); //These four lines result in 1 step
            delayMicroseconds(2500);
            digitalWrite(pinStepPulse, LOW);
            delayMicroseconds(2500);
            intStepCount++;
          }

          delay(300);
        }

        fixSteps(intStepCount,stepsPerRev); //Fix drift
        Serial.println(intStepCount);
        intStepCount=0; //Reset count
        
        digitalWrite(pinStepEn, HIGH); //Disable stepper
      }
    }
    serialFlag = false; //Unflag now that command has been executed
  }

  /*if (autoRetract) //Retraction autopilot
    {
    if (intPos > int30cm) //Stop once past 30cm
    {
      analogWrite(pinSpdB, 0); //Cut actuator power
      autoRetract = false;
    }
    }
    else if (autoDrill) //Drilling autopilot
    {
    if (intPos < int10cm) //Start drilling into ground when within 10cm
    {
      digitalWrite(pinDirA, HIGH); //Drill forward
      analogWrite(pinSpdA, 255); //Full drill power
      analogWrite(pinSpdB, 170); //Actuator power two thirds
    }
    else if (intPos < intMin) //Stop drilling when 10cm deep
    {
      analogWrite(pinSpdA, 0); //Cut drill power
      analogWrite(pinSpdB, 0); //Cut actuator power
      autoDrill = false; //Stop autopilot
    }
    }
    else if (autoExtract)
    {
    if (intPos > int10cm)
    {
      digitalWrite(pinDirA, LOW); //Reverse drill
      analogWrite(pinSpdA, 76); //Drill at ratio
    }
    else if (intPos > int30cm)
    {
      analogWrite(pinSpdA, 0); //Cut drill power
      analogWrite(pinSpdB, 0); //Cut actuator power
      autoExtract = false; //Autopilot off
    }
    }

    if (safety) //Check if carriage is going too far when safety is on
    {
    if (intPos < intMin || intPos > intMax)
    {
      analogWrite(pinSpdA, 0); //Kill drill
      analogWrite(pinSpdB, 0); //Kill actuator
      safety = false; //Safety off immediately
      autoRetract = false; //Stop retraction autopilot
      autoDrill = false; //Stop drilling autopilot
      autoExtract = false; //Stop extraction autopilot
    }
    }*/
}

void fixSteps(int intStepCount, int intTarget)
{
  int error = intStepCount - intTarget;

  if (error < 0)
  {
    digitalWrite(pinStepDir, LOW);
  }
  else if (error > 0)
  {
    digitalWrite(pinStepDir, HIGH);
  }

  digitalWrite(pinStepEn, LOW); //Enable stepper
  for (int j = 0; j < error; j++)
  {
    digitalWrite(pinStepPulse, HIGH); //These four lines result in 1 step
    delayMicroseconds(2500);
    digitalWrite(pinStepPulse, LOW);
    delayMicroseconds(2500);
  }
  digitalWrite(pinStepEn, HIGH); //Disable stepper

  digitalWrite(pinStepDir,LOW);
}

int getSteps(int revs)
{
  if (revs==2||revs==5||revs==8||revs==11)
  {
    return floor(stepsPerRev / 12)+1;
  }
  else
  {
    return floor(stepsPerRev / 12);
  }
}

