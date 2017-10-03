// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AccelStepper.h>
#include <AFMotor.h>
#include "HX711.h"
#define CALIBRACION 2280.f
#define STOP_LATE 30000

// HX711.DOUT  - pin #A1
// HX711.PD_SCK - pin #A0

HX711 scale(A9, A8);    // parameter "gain" is ommited; the default value 128 is used by the library

// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(48, 2);
// These constants won't change.  They're used to give names
// to the pins used:
AF_DCMotor motordc(1);
const int analogInPin = A11;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = A11; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 1023;   

// you can change these to DOUBLE or INTERLEAVE or MICROSTEP!
void forwardstep() {  
  motor.onestep(BACKWARD, SINGLE);
}
void backwardstep() {  
  motor.onestep(FORWARD, SINGLE);
}

int red = 44; //this sets the red led pin
int green = 45; //this sets the green led pin
int blue = 46; //this sets the blue led pin

AccelStepper stepper(forwardstep, backwardstep); 

void setup() {
  Serial.begin(9600);
  
  Serial.print("{");
  Serial.print("\"state\":\"staring\",");
  Serial.print("\"value\":0,");
  
  Serial.print("\"read_average\":");
  Serial.print(scale.read_average(10));// print the average of 20 readings from the ADC
  Serial.print(",");
  
  Serial.print("\"get_value\":");
  Serial.print(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)
  Serial.print(",");

  Serial.print("\"get_units\":\"");
  Serial.print(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided 
  Serial.print("\",");                           // by the SCALE parameter (not set yet) 

  
  Serial.print("}");
  Serial.println("");
  
  Serial.println();   
 
             

  scale.set_scale(CALIBRACION); // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                 // reset the scale to 0
  
  // turn on motor
  motordc.setSpeed(250);
 
  motordc.run(RELEASE);
  
  stepper.setSpeed(250);
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  analogWrite(red, 1024);
  analogWrite(green, 1024);
  analogWrite(blue, 1024);
}
int to_load = 0;
int cur_load = 0;

int times = 0;
int old = -1;
int stop_late = 0;
int readed;
int last_load = 0;
int promedio_en_banda = 5;


void loop() {
    analogWrite(red, 0);
    analogWrite(green, 1024);
    analogWrite(blue, 0);
  if (Serial.available() > 0) {
    readed = Serial.read();
    if (readed > 0){
      to_load = readed;
      scale.tare();
    }
  }
  if (to_load > 0 && to_load - promedio_en_banda > cur_load){ 
    stepper.runSpeed();//mueve sepper
    motordc.run(FORWARD);//mueve banda
  }else
  if (to_load > 0 && to_load - promedio_en_banda <= cur_load){
    stop_late ++;
    if (stop_late >= STOP_LATE){
      last_load = to_load;
      to_load = 0;
      stop_late = 0;
    }
  }else{
    motordc.run(RELEASE);//detiene banda
    stop_late = 0;

    analogWrite(red, 0);
    analogWrite(green, 1024);
    analogWrite(blue, 0);
  }
  if (times > 500){
    cur_load = scale.get_units(2)*10*50/77;
    if (stop_late > 0){
      Serial.print("{");
      Serial.print("\"state\":\"stoping\",");
      Serial.print("\"value\":");
      Serial.print(stop_late*100/STOP_LATE);
      Serial.print(",");
    }else
    if (to_load > cur_load){
      Serial.print("{");
      Serial.print("\"state\":\"loading\",");
      Serial.print("\"value\":");
      Serial.print(to_load);
      Serial.print(",");
    }else{
      if (last_load > 0){
        Serial.print("{");
        Serial.print("\"state\":\"loaded\",");
        Serial.print("\"value\":");
        Serial.print(last_load);
        Serial.print(",");   
      }else{
        Serial.print("{");
        Serial.print("\"state\":\"waiting\",");
        Serial.print("\"value\": 0,");
      }
    }
    Serial.print("\"load\":");
    Serial.print(cur_load);
    Serial.print("}");
    Serial.println("");
    times = 0;
  }
  times ++;
}
