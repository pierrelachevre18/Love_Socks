
IntervalTimer printTimer;
int line_swiper =0;

void setup() {
  Serial.begin(9600);
  printTimer.begin(LinePrint, 500000);
}

void loop() {
  line_swiper = analogRead(A0);
}

void LinePrint(void){
  Serial.println(line_swiper);
  return;
}
