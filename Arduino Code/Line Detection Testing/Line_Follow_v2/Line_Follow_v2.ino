/*************************************************************
  File:      Line_Follow.ino
  Contents:  Moving from a multiple state approach to a CL control one.
  
  History:
  when       who      what/why
  ----       -------  ---------------------------------------------
  2018-02-25 Pierre   File created to use CL control
 ************************************************************/

/*---------------Includes-----------------------------------*/

#include <Metro.h>

/*---------------Main constants Defines-----------------------------*/

#define TAPE_THR         300    // For the line following detectors
                                // Needs to classify gray as dark
                                // Good value as of 02-23 : 300
#define TALK_TIME_INTERVAL 3000000
#define CTRL_INTERVAL      1000

#define PIN_LEFT_SWIPER         A0
#define PIN_RIGHT_SWIPER        A1
#define PIN_POS_SWIPER          A2
#define PIN_DIR_LEFT            A3
#define PIN_DIR_RIGHT           A4
#define PIN_EN_LEFT             A5
#define PIN_EN_RIGHT            A6

int V_nom=127;                //Nominal voltage for motors, 0<V<256 (needs room for controller!)

// Initialize variables

int left_swiper = 0;
int right_swiper = 0;
int pos_swiper=0;
int tape_error=0;
int tape_error_sum=0;
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


/*---------------Main Functions----------------*/

void setup() {
  Serial.begin(9600);
  dispTimer.begin(say_stuff,TALK_TIME_INTERVAL);
  tapeTimer.begin(tapeController,CTRL_INTERVAL);
  
  state_m = STATE_FREE;
  state_t = STATE_OFFTAPE;
}

void loop() {
  left_swiper = analogRead(PIN_LEFT_SWIPER);
  right_swiper = analogRead(PIN_RIGHT_SWIPER);
  pos_swiper=analogRead(PIN_POS_SWIPER);
  tape_error=right_swiper-left_swiper;
  handleMove();
  checkGlobalEvents();
}

/*----------------Core Functions--------------------------*/

void say_stuff()
{
  // Useful do display stuff to debug
  Serial.println("Swipers");
  Serial.println(left_swiper);
  Serial.println(right_swiper);
  Serial.println(pos_swiper);
  Serial.println("Pos");
  Serial.println(pos_id);
}
void checkGlobalEvents(void) {
  //check for events
}

/*----------Movement Functions-----------------*/

void handleMove(void) {
      switch (state_m) {
      case STATE_FWD:
            digitalWrite(PIN_DIR_LEFT,1);
            digitalWrite(PIN_DIR_RIGHT,1);
            analogWrite(PIN_EN_RIGHT,V_nom-V_u);
            analogWrite(PIN_EN_LEFT,V_nom+V_u);
            updatePos();
      break;
      case STATE_BCK:
            digitalWrite(PIN_DIR_LEFT,0);
            digitalWrite(PIN_DIR_RIGHT,0);
            analogWrite(PIN_EN_RIGHT,V_nom+V_u);
            analogWrite(PIN_EN_LEFT,V_nom-V_u);
            updatePos();
        break;
      case STATE_STOP:
            digitalWrite(PIN_DIR_LEFT,1);
            digitalWrite(PIN_DIR_RIGHT,1);
            analogWrite(PIN_EN_RIGHT,0);
            analogWrite(PIN_EN_LEFT,0);
          break;
      case STATE_FREE:
            digitalWrite(PIN_DIR_LEFT,1);
            digitalWrite(PIN_DIR_RIGHT,1);
            analogWrite(PIN_EN_RIGHT,V_nom);
            analogWrite(PIN_EN_LEFT,V_nom);
            updatePos();
            break;
      default:    // Should never get into an unhandled state
        Serial.println("Unplanned Motor State");
  }
}

void tapeController(void) {
  // Needs to return an int at most 127
  tape_error_sum=tape_error_sum+tape_error; //For PI controller, not implemented yet
  int u_p;
  u_p=map(tape_error,0,1023,0,127);
  V_u= u_p;
}

/*-------Position Functions--------*/
void updatePos(void){
  switch (state_t) {
    case STATE_ONTAPE:
        if (pos_swiper>TAPE_THR) {
          state_t=STATE_OFFTAPE;
          pos_id=pos_id+(dir_sign+1)/2;
        }
      break;
    case STATE_OFFTAPE:
        if (pos_swiper<TAPE_THR){
          state_t=STATE_ONTAPE;
          pos_id=pos_id+(dir_sign+1)/2;
        }
      break;
    default:
      Serial.println("Unplanned Position State");
  }
}

