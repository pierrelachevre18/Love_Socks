/*************************************************************
  File:      Love_Socks_AI.ino
  Contents:  Global code
  
  History:
  when       who      	what/why
  ----       -------  	---------------------------------------------
  2018-2-27  Pierre   	Created from Line_Follow_v2
  2018-2-28	 Louise		  Code Reviewed
  2018-3-1	 Pierre		  Removed the PID, now only has discrete version
                        Added a LED to communicate when needed
                        Corrected the giant motor miswiring	
  2018-3-7   Louise     added servo control
  2018-3-7   Pierre     Merged with functional 90deg turn
                        Increased movement speed
 ***********************************************************/

/*---------------Includes-----------------------------------*/

#include <PWMServo.h>


/*---------------Main constants Defines-----------------------------*/

#define TAPE_THR         300    // For the line following detectors
                                // Needs to classify gray as dark
                                // Good value as of 02-23 : 300
#define POS_TAPE_THR     300    // Different to detect green tape too

#define TALK_TIME_INTERVAL  2000000			//Interval between message display
#define CTRL_INTERVAL       1000			//Interval between control value update
#define POS_INTERVAL        1000			//Interval between position update
#define SOLENOID_INTERVAL   500000    //Interval for solenoid 
#define ROTATE_INTERVAL      650000    //For the 90 deg turn
#define ADJ_SPEED_INTERVAL  500000

//Tape Follow
#define PIN_RIGHT_SWIPER         A0
#define PIN_LEFT_SWIPER        A1
#define PIN_POS_SWIPER          A2
//DC Motors
#define PIN_PWM_RIGHT           A8
#define PIN_PWM_LEFT            A9
#define PIN_IN_R_1              9
#define PIN_IN_R_2              10
#define PIN_IN_L_1              11
#define PIN_IN_L_2              12

//Servo motors
#define PIN_SERVO1            A6
#define PIN_SERVO2            A7

//Info LED
#define PIN_RED_LED             3

//Beacon
#define PIN_BEACON              A3

//Nominal voltage for motors, 0<V<256 (needs room for controller though!)
int V_nom_R=140;
int V_nom_L=140;
int V_nom_RT=80;        //For Turning

//Controller parameters
int Kp=10;                // Gain for discrete controller

// Initialize variables
int left_swiper = 0;
int right_swiper = 0;
int pos_swiper=0;
int V_u=0;
int dir_sign=1;
unsigned long rt_start_time=0;

int pos_servo1 = 0;    // variable to store the servo position
int pos_servo2 = 0;
/*---------------Module Function Prototypes-----------------*/


/*---------------State Definitions--------------------------*/
// State for movement
// Free is uncontrolled for testing
typedef enum {
  STATE_FWD, STATE_BCK, STATE_STOP, STATE_RT, STATE_FREE, STATE_FWD_OL
} States_m;

// State for positions
typedef enum {
  STATE_ONTAPE, STATE_OFFTAPE
} States_t;
int pos_id=1;

//State for global game playing
typedef enum {
  STATE_PHASE1, STATE_PHASE2, STATE_PHASE3
} States_g;
c
/*---------------Module Variables---------------------------*/
States_m state_m;
States_t state_t;
States_g state_g;
IntervalTimer dispTimer;
IntervalTimer tapeTimer;
IntervalTimer posTimer;
IntervalTimer adjTimer;
PWMServo servo1;  // create servo object to control a servo
PWMServo servo2; 
/*---------------Main Functions----------------*/

void setup() {
  //Initiate Timers
  Serial.begin(9600);
  //dispTimer.begin(say_stuff,TALK_TIME_INTERVAL);
  tapeTimer.begin(tapeController,CTRL_INTERVAL);    //Needs to be updated at constant intervals
  posTimer.begin(updatePos,POS_INTERVAL);
  adjTimer.begin(adjSpeed,ADJ_SPEED_INTERVAL);
  
  //Initiate States
  state_m = STATE_FWD;
  state_t = STATE_OFFTAPE;
  state_g = STATE_PHASE2;

  //Initiate Output Pins
  pinMode(PIN_PWM_RIGHT, OUTPUT);
  pinMode(PIN_PWM_LEFT, OUTPUT);
  pinMode(PIN_IN_L_1, OUTPUT);
  pinMode(PIN_IN_L_2, OUTPUT);
  pinMode(PIN_IN_R_1, OUTPUT);
  pinMode(PIN_IN_R_2, OUTPUT);
  pinMode(PIN_RED_LED,OUTPUT);
  pinMode(PIN_SERVO1,OUTPUT);
  pinMode(PIN_SERVO2,OUTPUT);

  // Initiate Servos
  servo1.attach(PIN_SERVO1);  // attaches the servo on pin 9 to the servo object
  servo2.attach(PIN_SERVO2);
}

void loop() {
  left_swiper = analogRead(PIN_LEFT_SWIPER);
  right_swiper = analogRead(PIN_RIGHT_SWIPER);
  pos_swiper=analogRead(PIN_POS_SWIPER);
  handleMove();
  checkGlobalEvents();
   
   pos_servo1 = 90;
   servo1.write(pos_servo1);              // tell servo to go to position in variable 'pos'

}

/*----------------Core Functions--------------------------*/

void say_stuff()
{
  //Useful do display stuff to debug
  Serial.println("Swipers");
  Serial.println(left_swiper);
  Serial.println(right_swiper);
  Serial.println(pos_swiper);
  Serial.println("Pos");
  Serial.println(pos_id);
  Serial.println(state_t);
}
void checkGlobalEvents(void) {
  //check for events
  if((state_g==STATE_PHASE2)&& (state_m==STATE_FWD) && (pos_id>=10)) {
    state_m=STATE_RT;
    rt_start_time=micros();
   Serial.println(rt_start_time);
  //redLEDOn();
  }
  if((state_m==STATE_RT) &&((micros()-rt_start_time)>ROTATE_INTERVAL)){
    Serial.println("All good!");
    state_m=STATE_FWD_OL;
  }
}

void redLEDOn(void) {
  digitalWrite(PIN_RED_LED,HIGH);
}

void redLEDOff(void)
{
  digitalWrite(PIN_RED_LED,LOW);
}

/*----------Movement Functions-----------------*/

void handleMove(void) {
  switch (state_m) {
    case STATE_FWD:
    rightFwd(V_nom_R+V_u);
    leftFwd(V_nom_L-V_u);
    break;
    case STATE_BCK:
    rightBck(V_nom_R);
    leftBck(V_nom_L);
    break;
    case STATE_STOP:
    rightOff();
    leftOff();
    break;
    case STATE_FREE:
    rightFwd(V_nom_R);
    leftFwd(V_nom_L);
    break;
    case STATE_RT:
    rightBck(V_nom_RT);
    leftFwd(V_nom_RT);
    break;
    case STATE_FWD_OL:
    rightFwd(200);
    leftFwd(200);
    break;    
      default:    // Should never get into an unhandled state
      Serial.println("Unplanned Motor State");
  }
}

  void tapeController(void) {
  // Controller for line follow, discrete version
  // Needs to return an int at most 256-V_nom
    V_u=Kp*((right_swiper>TAPE_THR)-(left_swiper>TAPE_THR));
  }

// Individual motor functions

  void leftFwd(int speed){
    digitalWrite(PIN_IN_L_1,LOW);
    digitalWrite(PIN_IN_L_2,HIGH);
    analogWrite(PIN_PWM_LEFT,speed);
  }

  void leftBck(int speed){
    digitalWrite(PIN_IN_L_1,HIGH);
    digitalWrite(PIN_IN_L_2,LOW);
    analogWrite(PIN_PWM_LEFT,speed);
  }

  void leftOff(void){
    digitalWrite(PIN_IN_L_1,LOW);
    digitalWrite(PIN_IN_L_2,LOW);
  }

  void rightFwd(int speed){
    digitalWrite(PIN_IN_R_1,LOW);
    digitalWrite(PIN_IN_R_2,HIGH);
    analogWrite(PIN_PWM_RIGHT,speed);
  }

  void rightBck(int speed){
    digitalWrite(PIN_IN_R_1,HIGH);
    digitalWrite(PIN_IN_R_2,LOW);
    analogWrite(PIN_PWM_RIGHT,speed);
  }

  void rightOff(void){
    digitalWrite(PIN_IN_R_1,LOW);
    digitalWrite(PIN_IN_R_2,LOW);
  }

void adjSpeed(void){
  if (pos_id>=8){
    V_nom_R=80;
    V_nom_L=80;
  }
  if (pos_id<=7){
    V_nom_R=140;
    V_nom_L=140;
  }  
}

/*-------Position Functions--------*/

  void updatePos(void){
  //Increment position when needed
    switch (state_t) {
      case STATE_ONTAPE:
      if (pos_swiper>POS_TAPE_THR*1.05) {
        state_t=STATE_OFFTAPE;
        pos_id=pos_id+(dir_sign+1)/2;
        Serial.println("Line passed");
        Serial.println(pos_swiper);
        Serial.println(pos_id);
        redLEDOff();
      }
      break;
      case STATE_OFFTAPE:
      if (pos_swiper<POS_TAPE_THR*0.95){
        state_t=STATE_ONTAPE;
        pos_id=pos_id+(dir_sign+1)/2;
        Serial.println("Line reached");
        Serial.println(pos_swiper);
        Serial.println(pos_id);
        redLEDOn();
      }
      break;
      default:
      Serial.println("Unplanned Position State");
    }
  }

/*---------Buzzword Functions---------------*/

