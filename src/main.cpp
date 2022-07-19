#include <Arduino.h>
#include <TimerOne.h>
#include <TimerFour.h>

// PWM Frequency
const long frequency = 50000L;
// temp constants
const float c1 = 1.009249522e-03; 
const float c2 = 2.378405444e-04;
const float c3 = 2.019202697e-07;

// Temp inputs
const int temp0_pin = A0;
const int temp1_pin = A1;

// Temp values
float temp0 = 0;
float temp1 = 0;

// Fan outputs
const int fanMin = 0;
const int fanMax = 1023;

// TIMER
const int fan0_pin = TIMER1_A_PIN; // 9
const int fan1_pin = TIMER1_B_PIN; // 10

const int fan2_pin = TIMER4_AC_PIN; // 5
const int fan3_pin = TIMER4_D_PIN;  // 6


int temp0_limits[2] = {30, 50};
int temp1_limits[2] = {30, 50};

int fan0  = 0;
int fan1  = 0;
int fan2  = 0;
int fan3  = 0;

int lastMillis = 0;
bool writeData = false;

void setup() {
  pinMode(fan0_pin, OUTPUT);
  pinMode(fan1_pin, OUTPUT);
  pinMode(fan2_pin, OUTPUT);
  pinMode(fan3_pin, OUTPUT);

  pinMode(temp0_pin, INPUT);
  pinMode(temp1_pin, INPUT);

  // Set Timer1 to highest frequency:
  Timer1.initialize(100);
  Timer1.pwm(fan0_pin, 0);
  Timer1.pwm(fan1_pin, 0);

  Timer4.initialize(100);
  Timer4.pwm(fan2_pin, 0);
  Timer4.pwm(fan3_pin, 0);

  // put your setup code here, to run once:

  Serial.begin(9600);
}

float getTemp(int pin){
  float logR2, R2, T;
  int v = analogRead(pin);
  R2 = 10000 * (1023.0 / (float)v - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  return T;
}


void loop() {
  // put your main code here, to run repeatedly:
  int curMillis = millis();

  if(curMillis - lastMillis > 100){ 
    temp0 = getTemp(temp0_pin);
    temp1 = getTemp(temp1_pin);

    fan0 = map(
      constrain(temp0, temp0_limits[0], temp0_limits[1]), 
      temp0_limits[0], temp0_limits[1], 
      0, 1023
    );
    fan1 = fan0;

    fan2 = map(
      constrain(temp1, temp1_limits[0], temp1_limits[1]), 
      temp1_limits[0], temp1_limits[1], 
      0, 1023
    );
    fan3 = fan2;

    Timer1.setPwmDuty(fan0_pin, fan0);
    Timer1.setPwmDuty(fan1_pin, fan1);

    Timer4.setPwmDuty(fan2_pin, 1023 - fan2);
    Timer4.setPwmDuty(fan3_pin, fan3);

    lastMillis = curMillis;
  }

  if(Serial.available() > 0){
    String command = Serial.readStringUntil('\n');
    int value = command.substring(4).toInt();

    if(command.startsWith("T0L:")){
      if(value > 0){
        temp0_limits[0] = value;
      }
      Serial.print("// temp0 low set to ");
      Serial.println(temp0_limits[0]);
      
    }
    else if(command.startsWith("T0H:")){
      if(value > 0){
        temp0_limits[1] = value;
      }
      Serial.print("// temp0 high set to ");
      Serial.println(temp0_limits[1]);
    }

    else if(command.startsWith("T1L:")){
      if(value > 0){
        temp1_limits[0] = value;
      }
      Serial.print("// temp1 low set to ");
      Serial.println(temp1_limits[0]);
    }

    else if(command.startsWith("T1H:")){
      if(value > 0){
        temp1_limits[1] = value;
      }
      Serial.print("// temp1 high set to ");
      Serial.println(temp1_limits[1]);
    }
    else if(command.startsWith("SRL:")){
      Serial.println("// data toggled");
      writeData = value > 0;
    }
    else{
      Serial.println("// Unknown command");
      return;
    }
    
  }

  if(writeData && Serial.availableForWrite()){
    Serial.print("/*");
    Serial.print(temp0);
    Serial.print(",");
    Serial.print(temp1);
    Serial.print(",");
    Serial.print(fan0);  
    Serial.print(",");
    Serial.print(fan1); 
    Serial.print(",");
    Serial.print(fan2);  
    Serial.print(",");
    Serial.print(fan3);
    Serial.print("*/");
    Serial.println(); 
  }

  delay(10);
}