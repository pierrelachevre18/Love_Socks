/*************************************************************
  File:      Love_Sock.ino
  Contents:  Main Code which includes the various functionalities

  History:
  when       who      what/why
  ----       -------  ---------------------------------------------
  2018-02-25 Pierre   File Created from Line_Follow
 ************************************************************/

/*---------------Includes-----------------------------------*/

#include <Metro.h>

/*---------------Module Defines-----------------------------*/

#define TAPE_THR         300    // For the line following detectors
                                // Needs to classify gray as dark
                                // Good value as of 02-23 : 300
                                
#define TALK_TIME_INTERVAL 1000000

#define PIN_LEFT_SWIPER         A0          //Left Tape Detector
#define PIN_RIGHT_SWIPER        A1          //Right Tape Detactor
#define PIN_POS_READER          A2          //Position Tape Detector

int left_swiper = 0;
int right_swiper = 0;
int state_id = 0;
int pos_reader=0;

/*---------------Module Function Prototypes-----------------*/


/*---------------State Definitions--------------------------*/
// Movement state
// BTL = Back to line
typedef enum {
  STATE_FWD, STATE_BCK, STATE_BTL_R, STATE_BTL_L, STATE_STOP, STATE_LOST
} States_m;

// Localisation state (over line or not, the exact position is just a number
// 1 for right before incubator to all the way to 13 before the garage
// Even numbers are always over tape
// 12 is the turning point, behavior TBD
int pos_id=1;
typedef enum {
  STATE_OVERTAPE, STATE_WHITE
} States_t;

/*---------------Module Variables---------------------------*/
States_m state_m;
States_t state_t;
IntervalTimer dispTimer;


/*---------------Raptor Main Functions----------------*/

void setup() {
  Serial.begin(9600);
  dispTimer.begin(say_stuff,TALK_TIME_INTERVAL);
  
  state_m = STATE_FWD;
  state_t = STATE_WHITE;
}

void loop() {
  left_swiper = analogRead(PIN_LEFT_SWIPER);
  right_swiper = analogRead(PIN_RIGHT_SWIPER);
  pos_reader = analogRead(PIN_POS_READER);
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

/*--------------Movement Functions-------------------*/
void checkGlobalEvents(void) {
  // Keeping on track
  if (TestOnTrack()) RespOnTrack();
  if (TestOutLeft()) RespOutLeft();
  if (TestOutRight()) RespOutRight();
  if (TestLost()) RespLost();
  // Where am I?
}

void handleMove(void) {
      switch (state_m) {
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
      case STATE_LOST:
          //TBD, probably rotate left and right until finding the track
          break;
      case STATE_STOP:
          //motors off
          break;
      default:    // Should never get into an unhandled state
        Serial.println("Unknown state");
  }
}

unsigned char TestOnTrack(void) {
  return ((left_swiper<TAPE_THR) & (right_swiper<TAPE_THR));
}

void RespOnTrack(void) {
    state_m=STATE_FWD;
    state_id=1;
}

unsigned char TestOutLeft(void) {
  return ((left_swiper>TAPE_THR)& (state_t==STATE_WHITE));
}

void RespOutLeft(void) {
    state_m=STATE_BTL_L;
    state_id=2;
}

unsigned char TestOutRight(void) {
  return ((right_swiper>TAPE_THR)& (state_t==STATE_WHITE));
}

void RespOutRight(void) {
    state_m=STATE_BTL_R;
    state_id=3;
}

unsigned char TestLost(void) {
  return ((left_swiper>TAPE_THR)&(right_swiper>TAPE_THR));
}

void RespLost(void) {
  state_m=STATE_LOST;
  state_id=4;
}


/*----------------Localisation functions------------------*/

