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
int clockPin = 26;
int dataPin = 25;

int potentiometer = 39;

int buzzer = 32;

void control(int n){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, n);
  digitalWrite(latchPin, HIGH);
}

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
const bool letter_A[] =   {1, 1, 1, 0, 1, 1, 1};
const bool letter_P[] =   {1, 1, 0, 0, 1, 1, 1};
const bool letter_H[] =   {0, 1, 1, 0, 1, 1, 1};
const bool letter_E[] =   {1, 0, 0, 1, 1, 1, 1};
const bool* numbers[] = {zero, one, two, three, four, five, six, seven, eight, nine, letter_P, letter_A, letter_H , letter_E};

const int digitPins[] = {D1, D2, D3, D4, DS};

uint8_t digitON = HIGH;
uint8_t digitOFF = !digitON;

uint8_t segmentON = LOW;
uint8_t segmentOFF = !segmentON;

int buttons[] = {34, 35};

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], digitOFF);
  }
  for(int i = 0; i < 2; i++){
    pinMode(buttons[i], INPUT);
  }

  pinMode(potentiometer, INPUT);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(buzzer, OUTPUT);
  
  Serial.begin(9600);
}

void show_digit(int n, bool point, bool high){
  int mask = 0;
  for(int i = 0; i < 7; i++){
    int smth = (!numbers[n][i]^high)<<(7-segmentPins[i]);
    mask |= smth;
    mask |= (!high^point)<<(7-DP);
    control(mask);
  }
}

void show_digits(int* digits, bool *points){
  for(int i = 0; i < 5; i++){
    if(digits[i] != -1){
      show_digit(digits[i], points[i], segmentON^i==4);      
      digitalWrite(digitPins[i], digitON);
    } else
      digitalWrite(digitPins[i], digitOFF);
      
    digitalWrite(digitPins[(i+1)%5], digitOFF);
    digitalWrite(digitPins[(i+2)%5], digitOFF);
    digitalWrite(digitPins[(i+3)%5], digitOFF);
    digitalWrite(digitPins[(i+4)%5], digitOFF);
    delay(4);
    digitalWrite(digitPins[(i)%5], digitOFF);
  }
}

enum Mode {
  WORK,
  BREAK,
  PAUSE,
  EDIT
};

enum Mode prev_mode = WORK;
enum Mode mod = PAUSE;

int button_state[2];
unsigned long button_times[2] = {0, 0};

//for dubble and triple push
//int button_streak[2] = {0, 0};
//unsigned long button_streak_times[2] = {0, 0};

void update_stuff(){
  for(int i = 0; i < 2; i++){
    if(analogRead(buttons[i]) > 4000){
      if(button_times[i] == 0){
        button_state[i] = 1;
        button_times[i] = millis();
      }
    }else {
      if(button_times[i] && millis()-button_times[i] > 1000){
        button_state[i] = 3;
      } else if(button_times[i]){
        button_state[i] = 2;
      } else{
        button_state[i] = 0;  
      }
      button_times[i] = 0;
    }
  }
}

unsigned int default_work_time = 10;
unsigned int default_break_time = 5;

unsigned long ramaining_work_time = 10;
unsigned long ramaining_break_time = 5;

unsigned long this_work_time = 10;
unsigned long this_break_time = 5;


bool demo = true;

void show_time(int a, int b, int mode_letter){
  int digits[] = {a/10, a%10, b/10, b%10, mode_letter};
  bool points[] = {false, true, false, false, false};
  show_digits(digits, points);
}

unsigned long start_time = 0;


void pause_mode(){
  if(button_state[0] == 2){
    mod = prev_mode;
    start_time = millis();
    return;
  }
  else if(button_state[1] == 2){
    mod = EDIT;
  }
  else if(button_state[1] == 3){
    ramaining_work_time = default_work_time;
    ramaining_break_time = default_break_time;
    this_work_time = default_work_time;
    this_break_time = default_break_time;
  }

  

  show_time(ramaining_work_time, ramaining_break_time, millis()%1500>750?12:(prev_mode==WORK?11:10));
}

void edit_mode(){
  if(button_state[0] == 3){
    if(prev_mode == WORK) prev_mode = BREAK;
    else if(prev_mode == BREAK) prev_mode = WORK;
  }
  else if(button_state[1] == 2){
    if(prev_mode == WORK){
      default_work_time = analogRead(potentiometer)*60/4095;
      prev_mode = BREAK;
    }
    else if(prev_mode == BREAK){
      default_break_time = analogRead(potentiometer)*60/4095;
      prev_mode = WORK;
    }
  }
  else if(button_state[1] == 3){
    mod = PAUSE;
  }
  if(prev_mode == WORK){
    
    show_time(analogRead(potentiometer)*60/4095, default_break_time, millis()%1500>750?13:11);
  }
  else if(prev_mode == BREAK){
    show_time(default_work_time, analogRead(potentiometer)*60/4095, millis()%1500<750?13:10);
  }
}

void break_mode(){
  if(ramaining_break_time == 0){
    ramaining_break_time = default_break_time;
    this_break_time = default_break_time;
    start_time = millis();
    mod = WORK;
    prev_mode = BREAK;
    return;
  }
  
  if(button_state[0] == 2){
    mod = PAUSE;
    prev_mode = BREAK;
    this_break_time = ramaining_break_time;
    return;
  }
  else if(button_state[1] == 2){
    mod = EDIT;
    prev_mode = BREAK;
  }
  else if(button_state[1] == 3){
    ramaining_break_time = default_break_time;
    this_break_time = default_break_time;
    start_time = millis();
  }
  else if(button_state[0] == 3){
    ramaining_break_time = default_break_time;
    this_break_time = default_break_time;
    start_time = millis();
    mod = WORK;
    prev_mode = WORK;
    return;
  }
  
  if((millis()-start_time)<200 && prev_mode == WORK){
    tone(buzzer, 2000);
  }
  else
    tone(buzzer, 0);
  
  ramaining_break_time = this_break_time-(millis()-start_time)/1000;
  
  show_time(ramaining_work_time, ramaining_break_time, 10);
}


void work_mode(){
  if(ramaining_work_time == 0){
    ramaining_work_time = default_work_time;
    this_work_time = default_work_time;
    start_time = millis();
    mod = BREAK;
    prev_mode = WORK;
    return;
  }
  
  if(button_state[0] == 2){
    mod = PAUSE;
    prev_mode = WORK;
    this_work_time = ramaining_work_time;
    return;
  }
  else if(button_state[0] == 3){
    ramaining_work_time = default_work_time;
    this_work_time = default_work_time;
    start_time = millis();
    mod = BREAK;
    prev_mode = BREAK;
    return;
  }
  else if(button_state[1] == 2){
    mod = EDIT;
    prev_mode = WORK;
  }
  else if(button_state[1] == 3){
    ramaining_work_time = default_work_time;
    this_work_time = default_work_time;
    start_time = millis();
  }

  if((millis()-start_time)<100 && prev_mode == BREAK){
    tone(buzzer, 2000);
  }
  else
    tone(buzzer, 0);
  
  ramaining_work_time = this_work_time-(millis()-start_time)/1000;
  
  show_time(ramaining_work_time, ramaining_break_time, 11);
}

void loop() {
  update_stuff();
  printf("%i\n", analogRead(potentiometer));

  switch(mod){
    case PAUSE:
      pause_mode();
      break;
    case WORK:
      work_mode();
      break;
    case BREAK:
      break_mode();
      break;
    case EDIT:
      edit_mode();
      break;
  }
}
