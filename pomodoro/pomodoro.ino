#include <LibPrintf.h>

byte D1 = 4;
byte D2 = 16;
byte D3 = 17;
byte D4 = 5;
byte DS = 2;


byte A = 7;
byte B = 6;
byte C = 2;
byte D = 1;
byte E = 0;
byte F = 4;
byte G = 3;
byte DP = 5; // decimal point

int latchPin = 27;
//Pin connected to SH_CP(11) blue of 74HC595
int clockPin = 26;
////Pin connected to DS of 74HC595
int dataPin = 25;

void control(int n){
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, n);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);
}

byte pot = 15;

const byte button1 = 35;

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
const int digitPins[] = {D1, D2, D3, D4, DS};

// For common cathode
uint8_t sdigitON = LOW;
uint8_t sdigitOFF = HIGH;

uint8_t digitON = HIGH;
uint8_t digitOFF = LOW;

uint8_t segmentON = HIGH;
uint8_t segmentOFF = LOW;

uint8_t ssegmentON = HIGH;
uint8_t ssegmentOFF = LOW;

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
//  for (int i = 0; i < 8; i++) {
//    pinMode(segmentPins[i], OUTPUT);
//    //digitalWrite(segmentPins[i], segmentOFF);
//  }

  for (int i = 0; i < 5; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], i!=4?digitON:sdigitON);
  }
  
//
//  //potentiometer
//  pinMode(pot, INPUT);
//
//  pinMode(button1, INPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Serial.begin(9600);
  
  mod = PAUSE;
  pauseFromMode = RUNNING_WORK;
  modeStartTime = millis();
  button1Time = -1;
}


unsigned long buttonImpulseTime;
unsigned long buttonCompleteTime;
bool buttonMomentState;
bool buttonGlobalState;

enum BUTTON_EVENT{
  NO_EVENT,
  PRESS_SINGLE,
  PRESS_DOUBLE,
  PRESS_HOLD
};
//0 0
BUTTON_EVENT get_button_event();//damn compiler, it placed the prototype at the top of them code... which why the declaration of the BUTTON_EVENT enum was out of the scope further down the code
BUTTON_EVENT get_button_event(){
  if((analogRead(button1)>4000) != buttonMomentState){    //if state changed...
    if(buttonGlobalState){                          //  if it's start or end of the bounce...
      if(!(analogRead(button1)>4000)){                    //    if it's the start of the bounce...
        buttonImpulseTime = millis();               //      start count down
        buttonMomentState = false;                  //      and set the state to 0
      }else{                                        //    if it's the start of the bounce...
        if(millis()-buttonImpulseTime > 50){        //      if it was up for longer than 50ms...
          buttonGlobalState = false;
          buttonMomentState = false;
          return PRESS_SINGLE;                      //        then it's a single press!
        }
      }
    }
    else{                                           //first press down
      buttonGlobalState = true;
      buttonMomentState = true;
    }
  }
  return NO_EVENT;
}


void show_digit(int n, bool point, bool high){
  int mask = 0;
  for(int i = 0; i < 7; i++){
    int smth = (numbers[n][i]?high:!high)<<(7-segmentPins[i]);
    mask |= smth;
    //printf("m %08b\n", smth);
    //digitalWrite(segmentPins[i], numbers[n][i]?segmentON:segmentOFF);
  }
  //printf("%i ", n);
  //printf("%08b\n", mask);
  mask |= (high^point)<<(7-DP);
  control(mask);
}

void show_digits(int* digits, bool *points){
//  int a = n/1000;
//  int b = n/100%10;
//  int c = n/10%10;
//  int d = n%10;
//
//  int digits[] = {a, b, c, d};

  for(int i = 0; i < 5; i++){
    if(digits[i] != -1){
      show_digit(digits[i], !points[i], i==4);      
      digitalWrite(digitPins[i], i!=4?segmentON:ssegmentON);
    }
    else
      digitalWrite(digitPins[i], i!=4?segmentOFF:ssegmentOFF);
      
    digitalWrite(digitPins[(i+1)%5], ((i+1)%5)!=4?segmentOFF:ssegmentOFF);
    digitalWrite(digitPins[(i+2)%5], ((i+2)%5)!=4?segmentOFF:ssegmentOFF);
    digitalWrite(digitPins[(i+3)%5], ((i+3)%5)!=4?segmentOFF:ssegmentOFF);
    digitalWrite(digitPins[(i+4)%5], ((i+4)%5)!=4?segmentOFF:ssegmentOFF);
    delay(2);
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
  
  //show_digits(digits, int(elapsedTime*dot_running_speed/1000)%3+1);

   if(get_button_event() == PRESS_SINGLE){
//    if(button1Time == -1)
//    {
      mod = pauseFromMode;

//      button1Time = millis();
      modeStartTime = millis();
      return;
    }
//  }else
//    button1Time=-1;
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

  if(get_button_event() == PRESS_SINGLE){
//    if(button1Time == -1)
//    {
      pauseFromMode = RUNNING_WORK;
      mod = PAUSE;
      currentWorkTime = displayTime;
      
//      button1Time = millis();
      modeStartTime = millis();
      return;
    }
//  }else
//    button1Time=-1;
  
  int a = displayTime/10;
  int b = displayTime%10;
  int c = currentBreakTime/10;
  int d = currentBreakTime%10;

  int digits[] = {a, b, c, d};
  
  //show_digits(digits, 2);

  
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

  if(get_button_event() == PRESS_SINGLE){
//    if(button1Time == -1)
//    {
      pauseFromMode = RUNNING_BREAK;
      mod = PAUSE;
      currentBreakTime = displayTime;

//      button1Time = millis();
      modeStartTime = millis();
      return;
    }
//  }else
//    button1Time=-1;
  
  int a = currentWorkTime/10;
  int b = currentWorkTime%10;
  int c = displayTime/10;
  int d = displayTime%10;

  int digits[] = {a, b, c, d};
  
//  show_digits(digits, 2);
}
void show_edit1_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;
}
void show_edit2_mode(){
  unsigned long elapsedTime = millis() - modeStartTime;
}

void loop() {
  for(int i = 0; i < 8; i++){
    int digits[] = {1, 2, 3, 4, 5};
    bool points[] = {true, false, true, false, true};
//    control(1<<(7-i));
//    delay(1000);
//    
    show_digits(digits, points);
  }
//  Serial.print(get_button_event());
//  Serial.print(" ");
//  Serial.print((analogRead(button1)>4000));
//  Serial.print(" ");
//  Serial.println(analogRead(button1));
//  switch(mod){
//    case PAUSE: show_pause_mode(); break;
//    case RUNNING_WORK: show_work_mode(); break;
//    case RUNNING_BREAK: show_break_mode(); break;
//  }
}
