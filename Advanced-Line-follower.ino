#include <EnableInterrupt.h>

#include "DCMotors.h" 
#include "Sensor.h"

#define START 's'
#define STOP 't'
#define isrPinRight 8
#define isrPinLeft 3

bool debug = true;
bool startDrive = false;
volatile bool isrInProcess  = false;


DCMotors<10,18,19,11,14,15> motors; //enL, L1, L2, enR, R1, R2
Sensor<2,3,4,5,6,7,8,9> sensors;

float Kp=8, Ki=5 ,Kd=0.03;

float P=0, D=0;
volatile float PID_value=0;
volatile float I=0;
float error=0;
float previous_error=0, previous_I=0;
char c;
//float speedmlp = 0.0;

void setup() {
  
  motors.configure();
  sensors.configure();
  Serial.begin(9600);

  debug = false;
  
}

void loop() {

    while(isrInProcess);
    
    if(Serial.available()){
        c = Serial.read();
     
        if(c == START){ 
          invalidate();
          startDrive = true; }
        else {
          
          startDrive = false;}
        
        if(debug){ btDebug(); }
      }
      
    if(startDrive){ 
      Serial.println("startdrive true");
      sensors.updateError();
      calculate_pid();
      if(!sensors.is90 && !sensors.is135 && !sensors.isCross){
        motors.drive((int)PID_value);  
        }
       else if(sensors.is90 && !sensors.is135 && !sensors.isCross){

           if(error > 0 ){ //turn right

                  enableInterrupt(isrPinLeft, isrRightTurnComplete, FALLING );
            
            } else if (error < 0){

                  enableInterrupt(isrPinRight, isrLeftTurnComplete, FALLING );
                
              } 
           isrInProcess = true;   
           motors.turn90((int)PID_value);
           
        }
        else if (sensors.is135 && !sensors.is90 && !sensors.isCross){
            motors.turn135((int) PID_value);
            
            sensors.is135 = false;
            invalidate();          
        }
        else if ( sensors.isCross && !sensors.is135 && !sensors.is90){
            motors.driveCross();
            sensors.isCross = false;
            invalidate();
          }   
    }
    else if(!startDrive){
      invalidate();
      motors.stopMoving();
    }
    delay(1);

}


void invalidate(){
  sensors.error = 0;
   PID_value = 0;
   I=0;
}

void calculate_pid()
{

    error = (sensors.error);
    //Serial.print(error);
 
    P = error;
    I = I + error*0.001;
    D = (error - previous_error)/.001;
    
    PID_value = (Kp*P) + (Ki*I) + (Kd*D);
        
    previous_error=error;
}

void isrRightTurnComplete(){
  
    motors.stopMoving();
    disableInterrupt(isrPinLeft);
    sensors.is90 = false;
    invalidate();
    isrInProcess = false;
  }

void isrLeftTurnComplete(){
  
    motors.stopMoving();
    disableInterrupt(isrPinRight);
    sensors.is90 = false;
    invalidate();
    isrInProcess = false;
  }

void btDebug()
{
      if(c=='1'){
          Ki+=0.1;
          Serial.println(Ki);
      }
      else if(c=='0'){  
          Kd+=0.1;
          Serial.println(Kd);
      }
      else if(c=='2'){
          Kp+=1;
          Serial.println(Kp);
      }
      else if(c=='3'){
          Ki-=0.1;
          Serial.println(Ki);
      }
      else if(c=='4'){  
          Kd-=0.1;
          Serial.println(Kd);
      }
      else if(c=='5'){
          Kp-=1;
          Serial.println(Kp);
      } else if(c=='6'){
        motors.spConst += 1;
        Serial.println(motors.spConst);
          
      } else if(c=='7'){
        motors.spConst -= 1;  
      }
}

