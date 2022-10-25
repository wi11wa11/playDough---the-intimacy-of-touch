//----- william wallis s3773723 --------
//----- electronic prototyping ---------
//----- RMIT University, Oct 2022 ------

//libraries
#include <AccelStepper.h> //include AccelStepper code library

//millis
unsigned long currentMillis;
unsigned long noTouch_previousMillis = 0;
unsigned long touchConnect_previousMillis = 0;
int noTouchInterval = 8000;
int x = 0;

//touch pins
#define touchPin 4

//touch variables
int touchVal = 0; //touch value read variable
const int smallTouchSensed = 24; //constant value for if small touch is sensed //ADJUST for touch
const int noTouchSensed = 25; //constant value for no touch being sensed //ADJUST for touch
const int touchConnection = 7; //constant value for a touch connection sensed //ADJUST for touch

//motor pins
#define motor1Pin1 12
#define motor1Pin2 14
#define enable1Pin 13

//PWM properties
const int freq = 30000; //PWM frequency for dc motor
const int pwmChannel = 0; //PWM channel
const int resolution = 8; //PWM 8-bit resolution (0-255)
int dutyCycle = 200;
int dutyCycleAccel = 10;

//stepper variables
AccelStepper stepper(AccelStepper::DRIVER, 21, 22); //Define a stepper and the pins it will use -- this uses AccelStepper code
int pos = 200; //stepper position variable
int state;

//-----------------------------
void setup() {
  //pin modes
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT); //dc motor pins as outputs

  //dc motor PWM setup
  ledcSetup(pwmChannel, freq, resolution); //configures the PWM functionalities for the dc motor
  ledcAttachPin(enable1Pin, pwmChannel); //connects the channel to the motor pin
  ledcWrite(pwmChannel, dutyCycle);

  //stepper setup
  stepper.setMaxSpeed(1000); //max speed motor reaches is 2000 (not sure if this is due to an issue with the motor)
  stepper.setAcceleration(4000);//acceleration is limited to 0 - 4000 for the esp32 I think

  //serialport
  Serial.begin(115200); //starts serial port
}

//----------------------------
void loop() {
  currentMillis = millis(); //initiates millis

  touchVal = touchRead(touchPin); //digitally reads touch pin
  //  Serial.print("Touch value is = ");
  //  Serial.println(touchVal); //prints touch value to serial (NOTE: printing breaks stepper functionality)
  //  delay(50); //small delay to not clog up serial port (NOTE: remove delay and printlns for final code)

  if (touchVal >= noTouchSensed) { //if no touch is sensed
    noTouch_stepper_and_dcMotor_control();
    touchConnect_previousMillis = currentMillis;
  }

  if (touchVal <= smallTouchSensed && touchVal > touchConnection) { //if a small touch is sensed (a nearby/hovering touch)
    DC_motor_smallTouch_control();
    stepper_smallTouch_control();
    noTouch_previousMillis = currentMillis; //resets the prevMillis to current millis each time touch is sensed
    touchConnect_previousMillis = currentMillis;
  }

  if (touchVal <= touchConnection) { //if a touch connection is made
    stepper_touchConnection_control();
    noTouch_previousMillis = currentMillis; //resets the prevMillis to current millis each time touch is sensed
  }
}

//-------------------------------------
void stepper_smallTouch_control() {
  if (stepper.distanceToGo() == 0) { //distance from target position to current position, if 0 execute statement
    delay(50);
    pos = -pos;
    stepper.moveTo(pos);
  }
  stepper.run();
  pos = random(100, 600);
}

//-----------------------------------------
void DC_motor_smallTouch_control() {
  //  Serial.println("Touched - motor moving (～￣▽￣)～"); //print line
  //  delay(50); //small delay (NOTE: can remove delay and println for final code)

  // Move DC motor forward with increasing speed
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); //sets the direction of the motor to counter clockwise

  ledcWrite(pwmChannel, dutyCycle); //sets the speed of the motor via PWM
  dutyCycle = dutyCycle + dutyCycleAccel; //adds to cycle value to speed up over time

  //if the dutyCycle reached 255 (max speed) set the dutyCycle to 255
  if (dutyCycle >= 255) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    dutyCycle = 255;
  }
}

//----------------------------------------------
void noTouch_stepper_and_dcMotor_control() {
  noTouchInterval = random(8000, 15000); //random time interval for no touch function //ADJUST
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW); //sets motor to off as soon as no touch is sensed
  dutyCycle = 200; //sets dutyCycle back to 200

  if (currentMillis - noTouch_previousMillis >= noTouchInterval) {
    noTouch_previousMillis = currentMillis; //sets the noTouch millis to the current millis when statement executes
    for (x = 0; x < random(100, 600); x++) { //Loop for the stepper movement and turning motor on
      digitalWrite(21, HIGH);
      delay(1);
      digitalWrite(21, LOW); //moves stepper motor clockwise a small amount
      delay(1);
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW); //sets dc motor to counterclockwise
      ledcWrite(pwmChannel, dutyCycle); //turns on motor
    }
    delay(random(500, 2000)); //delay for motor movement to be noticeable
  }
}

void stepper_touchConnection_control() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW); //sets motor to off as soon as connection is made
  dutyCycle = 200; //sets dutyCycle back to 200

  if (currentMillis - touchConnect_previousMillis >= 4000) {
    state = digitalRead(22); //reads the easydriver direction pin
    if (state == HIGH) {
      digitalWrite(22, LOW); //changes the easydriver direction
    }
    else if (state == LOW) {
      digitalWrite(22, HIGH); //changes the easydriver direction
    }
    for (x = 0; x < 15; x++) {
      digitalWrite(21, HIGH);
      delay(2);
      digitalWrite(21, LOW); //moves stepper motor a small amount
      delay(2);
    }
  }


}
