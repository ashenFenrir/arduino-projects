byte D1 = 2;
byte D2 = 4;
byte D3 = 16;
byte D4 = 17;

byte A = 12;
byte B = 27;
byte C = 33;
byte D = 25;
byte E = 26;
byte F = 14;
byte G = 32;
byte DP = 5; // decimal point

byte pot = 15;

const byte button1 = 18;

// Define the input pins for the 7-segment display segments.
const int segmentPins[] = {A, B, C, D, E, F, G, DP};

const bool zero[] =       {1, 1, 1, 1, 1, 1, 0};
const bool one[] =        {0, 1, 1, 0, 0, 0, 0};
const bool two[] =        {1, 1, 0, 1, 1, 0, 1};
const bool three[] =      {1, 1, 1, 1, 0, 0, 1};
const bool four[] =       {0, 1, 1, 0, 0, 1, 1};
const bool five[] =       {1, 0, 1, 1, 0, 1, 1};
const bool six[] =        {1, 0, 1, 1, 1, 1, 1};
const bool seven[] =      {1, 1, 1, 0, 0, 0, 0};
const bool eight[] =      {1, 1, 1, 1, 1, 1, 1};
const bool nine[] =       {1, 1, 1, 1, 0, 1, 1};
const bool* numbers[] = {zero, one, two, three, four, five, six, seven, eight, nine};

// Define the input pins for the 7-segment display digits.
const int digitPins[] = {D1, D2, D3, D4};

// For common cathode
uint8_t digitON = HIGH;
uint8_t digitOFF = LOW;

uint8_t segmentON = LOW;
uint8_t segmentOFF = HIGH;

enum Mode {
    RUNNING_WORK,
    RUNNING_BREAK,
    PAUSE,
    EDIT1,
    EDIT2
};

int workTime = 10;
int breakTime = 5;

int currentWorkTime = workTime;
int currentBreakTime = breakTime;

unsigned long modeStartTime;
unsigned long button1Time;

enum Mode mod;
enum Mode pauseFromMode;

float dot_running_speed = 2;



void setup() {
  // Pin initialization.
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], segmentOFF);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], digitON);
  }

  //potentiometer
  pinMode(pot, INPUT);

  pinMode(button1, INPUT);

  Serial.begin(9600);
  
  mod = PAUSE;
  pauseFromMode = RUNNING_WORK;
  modeStartTime = millis();
  button1Time = -1;
}



void show_digit(int n){
  for(int i = 0; i < 7; i++){
    digitalWrite(segmentPins[i], numbers[n][i]?segmentON:segmentOFF);
  }
}
void show_digits(int* digits, int point){
//  int a = n/1000;
//  int b = n/100%10;
//  int c = n/10%10;
//  int d = n%10;
//
//  int digits[] = {a, b, c, d};

  for(int i = 0; i < 4; i++){
    if(digits[i] != -1){
    show_digit(digits[i]);
    if(point-1 == i) digitalWrite(DP, segmentON);
    else digitalWrite(DP, segmentOFF);
    digitalWrite(digitPins[i], digitON);
    digitalWrite(digitPins[(i+1)%4], digitOFF);
    digitalWrite(digitPins[(i+2)%4], digitOFF);
    digitalWrite(digitPins[(i+3)%4], digitOFF);
    delay(5);
    }
  }
}



void show_pause_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;
//  int n = floor((float)analogRead(pot)/4095*60);
  //Serial.println((float)analogRead(pot)/4095*60);

  int a = currentWorkTime/10;
  int b = currentWorkTime%10;
  int c = currentBreakTime/10;
  int d = currentBreakTime%10;

  int digits[] = {a, b, c, d};
  
  show_digits(digits, int(elapsedTime*dot_running_speed/1000)%3+1);

   if(digitalRead(button1)){
    if(button1Time == -1)
    {
      mod = pauseFromMode;

      button1Time = millis();
      modeStartTime = millis();
      return;
    }
  }else
    button1Time=-1;
}

bool demo = true;

void show_work_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;

  int displayTime = currentWorkTime-elapsedTime/(1000*(1+59*(!demo)));

  if(displayTime<0){
    mod = RUNNING_BREAK;
    currentWorkTime = workTime;
    modeStartTime = millis();
    return;
  }

  if(digitalRead(button1)){
    if(button1Time == -1)
    {
      pauseFromMode = RUNNING_WORK;
      mod = PAUSE;
      currentWorkTime = displayTime;
      
      button1Time = millis();
      modeStartTime = millis();
      return;
    }
  }else
    button1Time=-1;
  
  int a = displayTime/10;
  int b = displayTime%10;
  int c = currentBreakTime/10;
  int d = currentBreakTime%10;

  int digits[] = {a, b, c, d};
  
  show_digits(digits, 2);

  
}
void show_break_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;

  int displayTime = currentBreakTime-elapsedTime/(1000*(1+59*(!demo)));

  if(displayTime<0){
    mod = RUNNING_WORK;
    currentBreakTime = breakTime;
    modeStartTime = millis();
    return;
  }

  if(digitalRead(button1)){
    if(button1Time == -1)
    {
      pauseFromMode = RUNNING_BREAK;
      mod = PAUSE;
      currentBreakTime = displayTime;

      button1Time = millis();
      modeStartTime = millis();
      return;
    }
  }else
    button1Time=-1;
  
  int a = currentWorkTime/10;
  int b = currentWorkTime%10;
  int c = displayTime/10;
  int d = displayTime%10;

  int digits[] = {a, b, c, d};
  
  show_digits(digits, 2);
}
void show_edit1_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;
}
void show_edit2_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;
}

void loop() {
  switch(mod){
    case PAUSE: show_pause_mode(); break;
    case RUNNING_WORK: show_work_mode(); break;
    case RUNNING_BREAK: show_break_mode(); break;
  }
}
