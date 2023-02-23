/**
 * George Fox University
 * ENGR 380 - Servant Engineering: Team ETA
 * Name: buzzer_ball
 * Purpose: Captures motion of the ball and communicates through a buzzer
 * 
 * @author Jamie F. O'Connell (joconnell19@georgefox.edu)
 * @version 1.2.1 04/13/2022
 */

#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Wire.h>
#include <SPI.h>
#include "SparkFunLSM9DS1.h"
#include "pitches.h"

LSM9DS1 imu;
#define PRINT_CALCULATED
#define PRINT_SPEED 15 // ms between prints
#define DECLINATION -1.7 // Declination (degrees) in Portland, OR.

// Magnitude Limits for Accelerometer Readings
#define MAX_ACCEL_ON_GROUND 1.5
#define MIN_ACCEL_ON_GROUND 0.8
#define MIN_ACCEL_FOR_SPIKE 2.5
#define MIN_SPIN_FOR_SPIKE 1.5
#define MAX_MAG_ACCEL_IN_AIR 0.5

// TIME CONTANTS
#define TIME_STOPPED_TO_BUZZ 2000 //ms
#define SHORT 50
#define MID 100
#define LONG 150



static unsigned long lastPrint = 0; // Keep track of print time

// DEFINE BUZZER PINS
const int buzzerPin = 9;
const int buzzerPin2 = 10;

// INITIALIZE STOP COUNTER
int stopCounter = 0;

boolean off = false;
int counter = 0;

//////////////////////////
// FUNCTION DEFINITIONS //
//////////////////////////

/**
 * Reads the gyroscope, acceleration, and magnetometer info of the
 * SparkFunLSM9DS1 chip.
 */
void readValues();

/**
 * Checks if the ball has been stopped for 2 seconds.
 * If so, buzz constantly until the ball is moved.
 */
void alertStop();

/**
 * Sends signal to buzzer if there is a spike in the magnitude
 * of acceleration. Sends a different signal if the ball is detected
 * to be rolling on a surface.
 */
void alertSpikes();

/**
 * Sends signal to buzzer if the ball is airborne,
 * i.e. the magnitude of acceleration is around 0.
 */
void alertInAir();

/**
 * Prints the magnitude of acceleration of the device.
 */
void printSumOfAccel();



void setup()
{
  Serial.begin(115200);

  Wire.begin();

  // THIS CHUNK OF CODE IS FROM LSM9DS1 EXAMPLE CODE
  if (imu.begin() == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
  }
  
  // setup buzzers to output
  pinMode(buzzerPin, OUTPUT);
  pinMode(buzzerPin2, OUTPUT);
}


void loop()
{
  readValues();

  if ((lastPrint + PRINT_SPEED) < millis())
  {
    alertStop();
    alertSpikes();
    alertInAir();
    printSumOfAccel();
   

    lastPrint = millis(); // Update lastPrint time
  }
}



void readValues() {
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    imu.readGyro();
  }
  
  if ( imu.accelAvailable() )
  {
    imu.readAccel();
  }
  
  if ( imu.magAvailable() )
  {
    imu.readMag();
  }
}


void printSumOfAccel()
{
  imu.readAccel();
  Serial.println(sqrt(sq(imu.calcAccel(imu.ax)) + sq(imu.calcAccel(imu.ay)) + sq(imu.calcAccel(imu.az))));
}


void alertStop()
{
    double magAccel = sqrt(sq(imu.calcAccel(imu.ax)) + sq(imu.calcAccel(imu.ay)) + sq(imu.calcAccel(imu.az)));
    double magGyro = sqrt(sq(imu.calcAccel(imu.gx)) + sq(imu.calcAccel(imu.gy)) + sq(imu.calcAccel(imu.gz)));

    // TURN OFF STOP NOISE
    
    if (magAccel > 2)
    {
      counter++;
    }
    if (counter == 4)
    {
      stopCounter = 0;
    }
    if (counter > 4)
    {
      counter = 0;
    }

    // Count how long the ball is stopped for
    if (magAccel < MAX_ACCEL_ON_GROUND && magAccel > MIN_ACCEL_ON_GROUND) { // if ball not moving, incr counter
      stopCounter++;
    }
    else {
      stopCounter = 0; // if ball moving, reset counter
    }

    if (stopCounter > TIME_STOPPED_TO_BUZZ / PRINT_SPEED) {
      tone(buzzerPin, NOTE_CS8, MID);
      delay(SHORT);
      noTone(buzzerPin);
      tone(buzzerPin2, NOTE_F6, MID);
      delay(SHORT);
      noTone(buzzerPin2);
    }

}


void alertSpikes()
{
  // alert spike in acceleration
  double magAccel = sqrt(sq(imu.calcAccel(imu.ax)) + sq(imu.calcAccel(imu.ay)) + sq(imu.calcAccel(imu.az)));
  
  if (magAccel > MIN_ACCEL_FOR_SPIKE) {
    tone(buzzerPin, NOTE_DS8, MID);
    delay(SHORT);
    noTone(buzzerPin);
    tone(buzzerPin2, NOTE_DS8, MID);
    delay(MID);
    noTone(buzzerPin2);
  }

  // alert spike in roll while ball is not in air
  double magGyro = sqrt(sq(imu.calcAccel(imu.gx)) + sq(imu.calcAccel(imu.gy)) + sq(imu.calcAccel(imu.gz)));
  Serial.print(magGyro);
  Serial.println(magAccel);
  if (magGyro > MIN_SPIN_FOR_SPIKE && magAccel > MIN_ACCEL_ON_GROUND && magAccel < MAX_ACCEL_ON_GROUND) { 
    tone(buzzerPin, NOTE_B7, MID);
    delay(SHORT);
    noTone(buzzerPin);
    tone(buzzerPin2, NOTE_B7, MID);
    delay(SHORT);
    noTone(buzzerPin2);
  }
}


void alertInAir()
{
    double magAccel = sqrt(sq(imu.calcAccel(imu.ax)) + sq(imu.calcAccel(imu.ay)) + sq(imu.calcAccel(imu.az)));

    if (magAccel < MAX_MAG_ACCEL_IN_AIR) {
      tone(buzzerPin, NOTE_E7, MID);
    }
}
