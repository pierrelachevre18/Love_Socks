/*************************************************************
  File:      Love_Socks_AI.ino
  Contents:  Global code
  
  History:
  when       who      what/why
  ----       -------  ---------------------------------------------
  2018-2-27  Pierre   Created from Line_Follow_v2
 ***********************************************************/

/*---------------Includes-----------------------------------*/

#include <Metro.h>

/*---------------Main constants Defines-----------------------------*/

#define TAPE_THR         300    // For the line following detectors
                                // Needs to classify gray as dark
                                // Good value as of 02-23 : 300
#define POS_TAPE_THR     550    // Different to detect green tape too

#define TALK_TIME_INTERVAL  2000000
#define CTRL_INTERVAL       1000
#define POS_INTERVAL        1000

//Tape Follow
#define PIN_LEFT_SWIPER         A0
#define PIN_RIGHT_SWIPER        A1
#define PIN_POS_SWIPER          A2
//DC Motors
#define PIN_PWM_LEFT            A8
#define PIN_PWM_RIGHT           A9
#define PIN_IN_L_1              9
#define PIN_IN_L_2              10
#define PIN_IN_R_1              11
#define PIN_IN_R_2              12

//Buzzword Dispenser
#define PIN_SOLENOID            7

//Nominal voltage for motors, 0<V<256 (needs room for controller though!)
int V_nom_R=90;
int V_nom_L=90;

//Controller parameters
int Kpi=30;               // 1/proportional gain
int Kii=0*600;              // 1/integral gain
int Kdi=0*30;               // 1/differential gain
int Kdis=15;                // Gain for discrete controller

// Initialize variables
int left_swiper = 0;
int right_swiper = 0;
int pos_swiper=0;
int tape_error=0;
int tape_error_sum=0;
int tape_error_prev=0;
int V_u=0;
int dir_sign=1;

/*---------------Module Function Prototypes-----------------*/


/*---------------State Definitions--------------------------*/
// State for movement
// Free is uncontrolled for testing
typedef enum {
  STATE_FWD, STATE_BCK, STATE_STOP, STATE_RT, STATE_FREE
} States_m;

// State for positions
typedef enum {
  STATE_ONTAPE, STATE_OFFTAPE
} States_t;
int pos_id=1;

/*---------------Module Variables---------------------------*/
States_m state_m;
States_t state_t;
IntervalTimer dispTimer;
IntervalTimer tapeTimer;
IntervalTimer posTimer;


/*---------------Main Functions----------------*/

void setup() {
  //Initiate Timers
  Serial.begin(9600);
  //dispTimer.begin(say_stuff,TALK_TIME_INTERVAL);
  tapeTimer.begin(tapeControllerDis,CTRL_INTERVAL);    //Needs to be updated at constant intervals
  posTimer.begin(updatePos,POS_INTERVAL);

  //Initiate States
  state_m = STATE_FREE;
  state_t = STATE_OFFTAPE;

  //Initiate Output Pins
  pinMode(PIN_PWM_RIGHT, OUTPUT);
  pinMode(PIN_PWM_LEFT, OUTPUT);
  pinMode(PIN_IN_L_1, OUTPUT);
  pinMode(PIN_IN_L_2, OUTPUT);
  pinMode(PIN_IN_R_1, OUTPUT);
  pinMode(PIN_IN_R_2, OUTPUT);
  pinMode(PIN_SOLENOID,OUTPUT);
}

void loop() {
  left_swiper = analogRead(PIN_LEFT_SWIPER);
  right_swiper = analogRead(PIN_RIGHT_SWIPER);
  pos_swiper=analogRead(PIN_POS_SWIPER);
  tape_error_prev=tape_error;
  tape_error=-right_swiper+left_swiper;
  if (abs(tape_error_sum)>12000) tape_error_sum=tape_error_sum/10;
  tape_error_sum=tape_error_sum+tape_error;
  handleMove();
  checkGlobalEvents();
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
}

/*----------Movement Functions-----------------*/

void handleMove(void) {
  switch (state_m) {
    case STATE_FWD:
    rightFwd(V_nom_R-V_u);
    leftFwd(V_nom_L+V_u);
    break;
    case STATE_BCK:
    rightBck(V_nom_R+V_u);
    leftBck(V_nom_L-V_u);
    break;
    case STATE_STOP:
    rightOff();
    leftOff();
    break;
    case STATE_FREE:
    rightFwd(V_nom_R);
    leftFwd(V_nom_L);
    break;
      default:    // Should never get into an unhandled state
      Serial.println("Unplanned Motor State");
    }
  }

  void tapeController(void) {
  // Controller for line follow
  // Needs to return an int at most 256-V_nom
    V_u=tape_error/Kpi+tape_error_sum/Kii+(tape_error-tape_error_prev)/Kdi;
  }

  void tapeControllerDis(void) {
  // Controller for line follow, discrete version
  // Needs to return an int at most 256-V_nom
    V_u=Kdis*((right_swiper>TAPE_THR)-(left_swiper>TAPE_THR));
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
      }
      break;
      case STATE_OFFTAPE:
      if (pos_swiper<POS_TAPE_THR*0.95){
        state_t=STATE_ONTAPE;
        pos_id=pos_id+(dir_sign+1)/2;
        Serial.println("Line reached");
        Serial.println(pos_swiper);
        Serial.println(pos_id);
      }
      break;
      default:
      Serial.println("Unplanned Position State");
    }
  }

/*---------Buzzword Functions---------------*/
  void openSolenoid(void){
    digitalWrite(PIN_SOLENOID, LOW);
  }

  void closeSolenoid(void){
    digitalWrite(PIN_SOLENOID, HIGH);
  }
