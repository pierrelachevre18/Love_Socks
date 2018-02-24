/*************************************************************
  File:      Line_Follow.ino
  Contents:  Testing sandbox for line following

  History:
  when       who      what/why
  ----       -------  ---------------------------------------------
  2018-02-22 Pierre   First Functional Version
  2018-02-23 Pierre   Case switching validated
 ************************************************************/

/*---------------Includes-----------------------------------*/

#include <Metro.h>

/*---------------Module Defines-----------------------------*/

#define TAPE_THR         300    // For the line following detectors
                                // Needs to classify gray as dark
                                // Good value as of 02-23 : 300
#define TALK_TIME_INTERVAL 1000000

#define PIN_LEFT_SWIPER         A0
#define PIN_RIGHT_SWIPER        A1

int left_swiper = 0;
int right_swiper = 0;
int state_id = 0;

/*---------------Module Function Prototypes-----------------*/


/*---------------State Definitions--------------------------*/
typedef enum {
  STATE_FWD, STATE_BCK, STATE_BTL_R, STATE_BTL_L
} States_t;
// BTL = Back to line

/*---------------Module Variables---------------------------*/
States_t state;
IntervalTimer dispTimer;


/*---------------Raptor Main Functions----------------*/

void setup() {
  Serial.begin(9600);
  dispTimer.begin(say_stuff,TALK_TIME_INTERVAL);
  
  state = STATE_FWD;
}

void loop() {
  left_swiper = analogRead(PIN_LEFT_SWIPER);
  right_swiper = analogRead(PIN_RIGHT_SWIPER);
  checkGlobalEvents();
  handleMove();
}

/*----------------Module Functions--------------------------*/

void say_stuff()
{
  // Useful do display stuff to debug
  Serial.println(state_id); //printing a state id because printing a state won't work
  Serial.println(left_swiper);
  Serial.println(right_swiper);
}
void checkGlobalEvents(void) {
  if (TestOnTrack()) RespOnTrack();
  if (TestOutLeft()) RespOutLeft();
  if (TestOutRight()) RespOutRight();
}

void handleMove(void) {
      switch (state) {
      case STATE_FWD:
          //both motors forward  
      break;
      case STATE_BCK:
          //both backwards
        break;
      case STATE_BTL_R:
          //rotate to the left
        break;
      case STATE_BTL_L:
           //rotate to the right
           break;
      default:    // Should never get into an unhandled state
        Serial.println("What is this I do not even...");
  }
}

unsigned char TestOnTrack(void) {
  return ((left_swiper<TAPE_THR) & (right_swiper<TAPE_THR));
}

void RespOnTrack(void) {
    state=STATE_FWD;
    state_id=1;
}

unsigned char TestOutLeft(void) {
  return (left_swiper>TAPE_THR);
}

void RespOutLeft(void) {
    state=STATE_BTL_L;
    state_id=2;
}

unsigned char TestOutRight(void) {
  return (right_swiper>TAPE_THR);
}

void RespOutRight(void) {
    state=STATE_BTL_R;
    state_id=3;
}
