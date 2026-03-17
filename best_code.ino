#include <QTRSensors.h>
#define KP 0.060         // Proportional gain
#define KI 0                 // Integral gain
#define KD 0.0023            // Differential gain
#define MAX_SPEED 255
#define DEFAULT_SPEED 220  
#define PWMA 3
#define AIN1 5
#define AIN2 4
#define PWMB 11
#define BIN1 6
#define BIN2 7
// This example is designed for use with eight RC QTR sensors. These
// reflectance sensors should be connected to digital pins 3 to 10. The
// sensors' emitter control pin (CTRL or LEDON) can optionally be connected to
// digital pin 2, or you can leave it disconnected and remove the call to
// setEmitterPin().
//
// The setup phase of this example calibrates the sensors for ten seconds and
// turns on the Arduino's LED (usually on pin 13) while calibration is going
// on. During this phase, you should expose each reflectance sensor to the
// lightest and darkest readings they will encounter. For example, if you are
// making a line follower, you should slide the sensors across the line during
// the calibration phase so that each sensor can get a reading of how dark the
// line is and how light the ground is.  Improper calibration will result in
// poor readings.
//
// The main loop of the example reads the calibrated sensor values and uses
// them to estimate the position of a line. You can test this by taping a piece
// of 3/4" black electrical tape to a piece of white paper and sliding the
// sensor across it. It prints the sensor values to the serial monitor as
// numbers from 0 (maximum reflectance) to 1000 (minimum reflectance) followed
// by the estimated location of the line as a number from 0 to 5000. 1000 means
// the line is directly under sensor 1, 2000 means directly under sensor 2,
// etc. 0 means the line is directly under sensor 0 or was last seen by sensor
// 0 before being lost. 5000 means the line is directly under sensor 5 or was
// last seen by sensor 5 before being lost.

QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

void setup()
{
  
  //TCCR2B = TCCR2B & B11111000 | B00000011;   
  //set timer 2 divisor to    32 for PWM frequency of   980.39 Hz

  //TCCR2B = TCCR2B & B11111000 | B00000100;   
  //set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  
  
  // configure the sensors
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){A7,A6,A5,A4,A3,A2,A1,9}, SensorCount);
  qtr.setEmitterPin(2);

  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
  for (uint16_t i = 0; i < 200; i++)
  {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

  // print the calibration minimum values measured when emitters were on
  Serial.begin(9600);
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(1000);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);    
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);




}
int lastError = 0;  
int totalError = 0;

void loop()
{
  // read calibrated sensor values and obtain a measure of the line position
  // from 0 to 5000 (for a white line, use readLineWhite() instead)
  uint16_t position = qtr.readLineBlack(sensorValues);

  // print the sensor values as numbers from 0 to 1000, where 0 means maximum
  // reflectance and 1000 means minimum reflectance, followed by the line
  // position
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }
  Serial.println(position);
  int error = 3400 - position;
  

  int deltaError = error - lastError;
  
  int Pvalue = KP * error;
  //int Ivalue = KI * totalError;
  int Dvalue = KD * deltaError;
  
  int motorSpeed = Pvalue + Dvalue;
  lastError = error;   // Operation for error prediction - derivative term
  totalError += error;
  
  int rightMotorSpeed = DEFAULT_SPEED + motorSpeed;
  int leftMotorSpeed = DEFAULT_SPEED - motorSpeed;
  
  if (rightMotorSpeed > MAX_SPEED) rightMotorSpeed = MAX_SPEED;
  if (leftMotorSpeed > MAX_SPEED) leftMotorSpeed = MAX_SPEED;
  
  if (rightMotorSpeed < 0)

  {

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, abs(rightMotorSpeed));

    //Serial.println("Right motor going backwards");

  }
  else
  {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, abs(rightMotorSpeed));
  }

   

  if (leftMotorSpeed < 0)

  {

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, abs(leftMotorSpeed));
    //Serial.println("Left motor going backwards");

  }
  else
  {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, abs(leftMotorSpeed));
  }
  
  
  
  
  //delay(100);
}
