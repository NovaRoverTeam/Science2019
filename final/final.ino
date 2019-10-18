/* As of 31/05
*/

const int pinDirA = 12; //Drill direction pin
const int pinDirB = 13; //Actuator direction pin
const int pinSpdA = 10; //Drill speed pin
const int pinSpdB = 11; //Actuator speed pin
const int pinSieve = 22; //Barrel sieve control pin
const int pinSievePumps = 24; //Pumps on either side of wet sieve control pin
const int pinCuvettePumps = 26; //Pumps to feed liquid into cuvettes pin
const int pinMixers = 28; //Mixing motors to stir vats
const int pinSpectro = 2;
const int pinStepEn = 3;
const int pinStepDir = 4;
const int pinStepPulse = 5;
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

int intInput; //Initialise variable for serial input
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
  pinMode(pinDirA, OUTPUT);
  pinMode(pinDirB, OUTPUT);
  pinMode(pinSpdA, OUTPUT);
  pinMode(pinSpdB, OUTPUT);
  pinMode(pinSieve, OUTPUT);
  pinMode(pinSievePumps, OUTPUT);
  pinMode(pinCuvettePumps, OUTPUT);
  pinMode(pinMixers, OUTPUT);
  pinMode(pinSpectro, OUTPUT);
  pinMode(pinStepDir, OUTPUT);
  pinMode(pinStepPulse, OUTPUT);
  pinMode(pinStepEn, OUTPUT);
  digitalWrite(pinDirA, LOW); //Reverse drill
  digitalWrite(pinDirB, LOW); //Actuator going down
  digitalWrite(pinSieve, LOW); //Sieve off
  digitalWrite(pinStepEn, HIGH);
  analogWrite(pinSpdA, 0); //Cut power to drill
  analogWrite(pinSpdB, 0); //Cut power to actuator
  digitalWrite(pinSpectro, HIGH);
  digitalWrite(pinStepDir, LOW); //Stepper CCW
  digitalWrite(pinStepPulse,LOW);
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
    intInput = Serial.read(); //Read command from serial line
    serialFlag = true; //Command should be executed
  }

  if (serialFlag)
  {
    if (intInput == 2) //Automated sequence to drill down and then stop 10cm deep
    {
      goingUp = false;
      autoDrill = true;
      autoRetract = false;
      autoExtract = false;
      digitalWrite(pinDirA, HIGH); //Drill forward
      digitalWrite(pinDirB, LOW); //Actuator down
      analogWrite(pinSpdA, intRatio); //Drill at ratio
      analogWrite(pinSpdB, 255); //Actuator full power
    }
    else if (intInput == 3) //Automated sequence to extract soil
    {
      goingUp = true;
      autoExtract = true;
      autoRetract = false;
      autoDrill = false;
      digitalWrite(pinDirB, HIGH); //Actuator up
      analogWrite(pinSpdA, 0); //Cut drill power
      analogWrite(pinSpdB, 255); //Actuator full power
    }
    else if (intInput == 4) //Kill switch; stops autopilots, drill and actuator
    {
      analogWrite(pinSpdA, 0);
      analogWrite(pinSpdB, 0);
      autoRetract = false; //Stop retraction autopilot
      autoDrill = false; //Stop drilling autopilot
      autoExtract = false; //Stop extraction autopilot
    }
    else if (intInput == 5) //Safety off
    {
      safety = false;
    }
    else if (intInput == 6) //Safety on
    {
      safety = true;
    }
    else if (!safety)
    {
      //Unsafe commands enabled
      if (intInput == 0) //Zero encoder count
      {
        intPos = 0;
      }
      else if (intInput == 1) //Automatic retraction
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

      }
      else if (intInput == 7) //Drill down at ratio, full power
      {
        goingUp = false;
        digitalWrite(pinDirA, HIGH); //Normal drill
        digitalWrite(pinDirB, LOW); //Actuator going down
        analogWrite(pinSpdA, 76); //Drill at ratio
        analogWrite(pinSpdB, 255); //Full actuator power
      }
      else if (intInput == 8) //Retract drill at ratio, full power
      {
        goingUp = true;
        digitalWrite(pinDirA, LOW); //Reverse drill
        digitalWrite(pinDirB, HIGH); //Actuator going up
        analogWrite(pinSpdA, 76); //Full drill power
        analogWrite(pinSpdB, 255); //Full actuator power
      }
      else if (intInput == 9) //Direct actuator up
      {
        goingUp = true;
        digitalWrite(pinDirB, HIGH);
      }
      else if (intInput == 10) //Direct actuator down
      {
        goingUp = false;
        digitalWrite(pinDirB, LOW);
      }
      else if (intInput == 11) //Direct drill forwards
      {
        digitalWrite(pinDirA, HIGH);
      }
      else if (intInput == 12) //Direct drill reverse
      {
        digitalWrite(pinDirA, LOW);
      }
      else if (intInput == 13) //Cut actuator power
      {
        analogWrite(pinSpdB, 0);
      }
      else if (intInput == 14) //Actuator power at one third
      {
        analogWrite(pinSpdB, 85);
      }
      else if (intInput == 15) //Actuator power at half
      {
        analogWrite(pinSpdB, 128);
      }
      else if (intInput == 16) //Actuator power at two thirds
      {
        analogWrite(pinSpdB, 170);
      }
      else if (intInput == 17) //Actuator power full
      {
        analogWrite(pinSpdB, 255);
      }
      else if (intInput == 18) //Cut drill power
      {
        analogWrite(pinSpdA, 0);
      }
      else if (intInput == 19) //Drill power at one third
      {
        analogWrite(pinSpdA, 85);
      }
      else if (intInput == 20) //Drill power at half
      {
        analogWrite(pinSpdA, 128);
      }
      else if (intInput == 21) //Drill power at two thirds
      {
        analogWrite(pinSpdA, 170);
      }
      else if (intInput == 22) //Drill power full
      {
        analogWrite(pinSpdA, 255);
      }
      else if (intInput == 23)
      {
        goingUp = false;
        digitalWrite(pinDirA, HIGH);
        digitalWrite(pinDirB, LOW);
        analogWrite(pinSpdA, 255);
        analogWrite(pinSpdB, 128);
      }
      else if (intInput == 24) //Turn on barrel sieve
      {
        digitalWrite(pinSieve, HIGH);
      }
      else if (intInput == 25) //Turn off barrel sieve
      {
        digitalWrite(pinSieve, LOW);
      }
      else if (intInput == 26) //Turn on wet sieve pumps
      {
        digitalWrite(pinSievePumps, HIGH);
      }
      else if (intInput == 27) //Turn off wet sieve pumps
      {
        digitalWrite(pinSievePumps, LOW);
      }
      else if (intInput == 28) //Turn on cuvette pumps
      {
        digitalWrite(pinCuvettePumps, HIGH);
      }
      else if (intInput == 29) //Turn off cuvette pumps
      {
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (intInput == 30) //Turn on mixers
      {
        digitalWrite(pinMixers, HIGH);
      }
      else if (intInput == 31) //Turn off mixers
      {
        digitalWrite(pinMixers, LOW);
      }
      else if (intInput == 32) //Flush and fill one cuvette
      {
        digitalWrite(pinCuvettePumps, HIGH);
        delay(timeFlush + timeFill);
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (intInput == 33) //Fill one cuvette
      {
        digitalWrite(pinCuvettePumps, HIGH);
        delay(timeFill);
        digitalWrite(pinCuvettePumps, LOW);
      }
      else if (intInput == 34) //Fill three pairs of cuvettes
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
        digitalWrite(pinStepEn, HIGH); //Disable stepper
        delay(300);
        fixSteps(intStepCount,stepsPerRev/4); //Fix drift
        intStepCount=0; //Reset count
      }
      else if (intInput == 35) //Scan twelve cuvettes
      {
        
        for (int i = 0; i < 12; i++)
        {
          //Scan once
          digitalWrite(pinSpectro, LOW);
          delay(50);
          digitalWrite(pinSpectro, HIGH);
          delay(2450);
          digitalWrite(pinStepEn, LOW); //Enable stepper

          //Rotate to next cuvette
          for (int j = 0; j < getSteps(i); j++)
          {
            digitalWrite(pinStepPulse, HIGH); //These four lines result in 1 step
            delayMicroseconds(2500);
            digitalWrite(pinStepPulse, LOW);
            delayMicroseconds(2500);
            intStepCount++;
          }
          digitalWrite(pinStepEn, HIGH); //Disable stepper

          delay(300);
        }

        fixSteps(intStepCount,stepsPerRev); //Fix drift
        intStepCount=0; //Reset count
        
        
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
    delayMicroseconds(00);
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

  /*if (revs==5||revs==11)
  {
    return floor(stepsPerRev / 12)+1;
  }
  else
  {
    return floor(stepsPerRev / 12);
  }*/
}
