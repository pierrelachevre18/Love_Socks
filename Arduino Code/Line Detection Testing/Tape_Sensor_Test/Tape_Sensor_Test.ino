/*************************************************************
  File:      Tape_Sensor_Test.ino
  Contents:  For building circuits for the tape sensors
  
  History:
  when       who      what/why
  ----       -------  ---------------------------------------------
  2018-02-22 Louise   Program Created
 ************************************************************/
 
IntervalTimer printTimer;
int line_swiper =0;

void setup() {
  Serial.begin(9600);
  pinMode(A9, OUTPUT);
  printTimer.begin(LinePrint, 1000000);
}

void loop() {
  line_swiper = analogRead(A0);
}

void LinePrint(void){
  Serial.println(line_swiper);
  return;
}
