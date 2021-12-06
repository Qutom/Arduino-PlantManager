#include <Wire.h> //LCD 라이브러리
#include <LiquidCrystal_I2C.h> //LCD 라이브러리
#include <EEPROM.h> // EEPROM 라이브러리
#include <Servo.h> //서보모터 라이브러리
#include <Keypad.h>
#include "plant_info.h"
#include "display.h"

//펌프 대기열을 나타냅니다.
typedef struct pump_queue  {
  byte pin_number;
  byte object;

  struct pump_queue* next;
} queue;

//STORED VARIABLE in EEPROM
byte water_temp[8][144];
int water_cooldown[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //펌프가 작동하고 난 후의 대기시간
boolean pump_queued[8] = {false, false, false, false, false, false, false, false}; //대기열에 해당 식물이 포함되어 있는가?
byte water_min[8] = {0, 0, 0, 0, 0, 0 ,0 ,0};  //최소 수분량을 나타냅니다. water_min[n]은 1+n번 식물을 나타냅니다.
int water_current[8] = { 100, 100, 100, 100, 100, 100, 100, 100}; //현재 수분량을 나타냅니다.
boolean light_state[8] = { true , true , true , true , true , true , true , true }; //LED 상태를 나타냅니다.
char plant_name[8][18] = {"PLANT1", "PLANT2", "PLANT3", "PLANT4", "PLANT5", "PLANT6", "PLANT7", "PLANT8"};

//PIN VALUE
byte PIN_SENSOR_W[8] = {A0,A1,A2,A3,A4,A5,A6,A7};
byte PIN_LED[8][2] = {{22,23},{24,25},{26,27},{28,29},{30,31},{32,33},{34,35},{36,37}};
byte PIN_KEYPAD[2][4] = {{39,41,43,45},{47,49,51,53}};
byte PIN_PUMP[3] = {10, 12 ,13}; //IN2 , IN3 , ENB PIN
byte PIN_JOYSTICK[3] = { A9 , A10 , 38 };
byte PIN_SERVO_UP = 4;
byte PIN_SERVO_DOWN = 5;
byte PIN_SENSOR_CDS = A8;
byte PIN_SENSOR_POWER = 2; //토양 수분 감지 센서의 부식문제를 막기위함

boolean display_info = true;
boolean option_modified = false;
boolean pump_state = false;
boolean joystick_state = false;
int servo_angle = 0;
int temp_index = 0;
unsigned long previous_time_1000;
unsigned long previous_time_600000;
long pump_time; //Pump가 연속적으로 동작한 시간
byte plant_num = 0; 
byte option_selected = 0; //현재 선택되어 있는 option , 0 = WATER , 1 = LIGHT
byte lcd_state = 0; //현재 lcd로 출력중인 화면 0 = INFO , 1 = OPTION , 2 = INPUT
short temp_input = 0; //숫자 입력을 저장
byte current_pump = 50; //현재 펌프가 작업중인 식물의 번호를 의미

//CONSTRUCT OBJECT
LiquidCrystal_I2C lcd(0x27,20,4); //LCD를 lcd 객체로 초기화 하였습니다.
Servo servo_down;
Servo servo_up;

//키패드 초기화
const byte ROWS = 4, COLS = 4;
char Keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad( makeKeymap(Keys), PIN_KEYPAD[0], PIN_KEYPAD[1], ROWS, COLS);


//FUNCTION PROTOTYPE
char get_joystick(int x, int y , int d);
void print_info();
void print_option();
void print_option_input();
void update_display(int joystick_input , int keypad_input );
void control_servo(void);
void control_queue();
void read_data(void);
byte up_count = 0;
byte down_count = 0;
//SETUP
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  previous_time_1000 = millis();
  previous_time_600000 = millis();
  
  //LED , KEYPAD
  for (int i =0; i<8; i++) {
    pinMode(PIN_LED[i][0], OUTPUT);
    pinMode(PIN_LED[i][1], OUTPUT);
  }

  //RESET WATER_TEMP
  for (int i =0; i < 8; i++) {
    for(int j = 0; j < 144; j++) {
      water_temp[i][j] = -1;
    }
  }
  
  //PUMP
  for (int i =0; i<3; i++) {
    pinMode(PIN_PUMP[i] , OUTPUT);
  }
  
  //JOYSTICK_BUTTON
  pinMode(PIN_JOYSTICK[2], INPUT);
  pinMode(PIN_SENSOR_POWER, OUTPUT);
  //SERVO
  servo_up.attach(PIN_SERVO_UP);
  servo_down.attach(PIN_SERVO_DOWN);
  
  //LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0,SELECTED_ARROW);
  //READ EEPROM
  for(int i=0; i<8; i++) {

    //WATER_MIN
    water_min[i] = EEPROM.read(i);
    
  }

  byte temp = EEPROM.read(10);
  for ( int i = 0; i < 8; i++ ) {
    if ( temp % 2 == 1 ) {
      light_state[i] = true;
    } else {
      light_state[i] = false;
    }

    temp /= 2;
  }
  pump_time = 0;

  for ( int i = 0; i < 8; i++ ) {
      if ( light_state[i] == true ) {
        digitalWrite(PIN_LED[i][0] , HIGH );
        digitalWrite(PIN_LED[i][1] , HIGH );
      } else {
        digitalWrite(PIN_LED[i][0] , LOW );
        digitalWrite(PIN_LED[i][1] , LOW );
      }
      
  }

  
  print_info();
}

void loop() {

  option_modified = false;
  long current_time = millis();
  long diff_1000 = current_time - previous_time_1000;
  long diff_600000 = current_time - previous_time_600000;
  
  if ( diff_1000 >= 1000 ) {
    previous_time_1000 = current_time;
  }
  if ( diff_600000 >= 600000 ) {
    previous_time_600000 = current_time;
  }
    
   
  /* 
   *  ==========================================================================
   *                             USER INPUT SECTION
   *  ==========================================================================                                         
   */ 
   
  //JOYSTICK
  char joystick = get_joystick(analogRead(PIN_JOYSTICK[0]) , analogRead(PIN_JOYSTICK[1]) , digitalRead(PIN_JOYSTICK[2]));
  
  //KEYPAD
  char key = keypad.getKey();
  boolean is_key = false;
  if (key) {
    is_key = true;
  }

  //FOR PRESENTATION, SET water_temp value to random
  if ( analogRead(PIN_JOYSTICK[1]) > 930 ) {
    up_count += 1;
    Serial.println(up_count);
    if (up_count >= 80) {
      srand(millis());
      for (int i =0; i < 8; i++) {
        for(int j = 0; j < 144; j++) {
          water_temp[i][j] = rand() % 100;
        }
      }
      Serial.println("WOW");
      up_count = 0;
    }
  } else {
    up_count = 0;
  }

  if ( analogRead(PIN_JOYSTICK[1]) <  90 ) {
    down_count += 1;
    
    if (down_count >= 80) {
      current_pump = 50;
      Serial.println("MOM");
      down_count = 0;
    }
  } else {
    down_count = 0;
  }
  /* 
   *  ==========================================================================
   *                              GRAPHIC SECTION
   *  ==========================================================================                            
   */  
  //Graphic update
  
  if ( key || (joystick != 'n')) {
    update_display(joystick,key);
  } else if ( display_info && diff_1000 >= 1000) {
    print_info();
  }

  /* 
   *  ==========================================================================
   *                      BLUETOOTH,SET OPTION SECTION
   *  ==========================================================================                                         
   */ 
   
  //read bluetooth serial(changed option value)
  read_data();

  //set modified option value with update EEPROM
  if ( option_modified ) {
    byte light_state_temp = 0;
    for (int i = 0; i < 8; i++) {
      EEPROM.update(i , water_min[i]);    //Update Water Min
      if ( light_state[i] ) {
        light_state_temp += pow(2, i);
      }
    }
    EEPROM.update(10 , light_state_temp); //Update Light State
    Serial.println("[EEPROM] Updated");
  }
  
  /* 
   *  ==========================================================================
   *                            READ SENSOR SECTION
   *  ==========================================================================                                         
   */ 
  
  //read water sensor
  if( diff_1000 >= 1000 ) {
    digitalWrite(PIN_SENSOR_POWER , HIGH);
    for ( int i = 0; i < 8; i++ ) {
    //read and calculate

    water_current[i] = map(analogRead(PIN_SENSOR_W[i]), 0 , 1023 , 100 ,0);
    //set Pump queue
    if ( water_min[i] > water_current[i] ) {
      pump_queued[i] = true;
    } else {
      pump_queued[i] = false;
    }
    }
    Serial.println("");
    //record sensor value ( water_temp )
    if ( diff_600000 >= 600000 ) {

      for ( int i = 0; i < 8; i++ ) {
        water_temp[i][temp_index] = water_current[i];   
      }

      if ( temp_index >= 143 ) {
        temp_index = 0;
      } else {
        temp_index++;
      }
      
  }
  
  digitalWrite(PIN_SENSOR_POWER , LOW); //PREVENT DECAY
  
  /* 
   *  ==========================================================================
   *                              CONTROL SECTION
   *  ==========================================================================                                         
   */ 
   
  //set queue
  if ( current_pump == 50 || pump_queued[current_pump] == false ) {
    current_pump = set_queue();
    //control pump , servo
    control_servo();
    control_pump(diff_1000);
    Serial.println(current_pump);
  }
  
  
  

  
  //control LED

    for ( int i = 0; i < 8; i++ ) {
      if ( light_state[i] == true ) {
        digitalWrite(PIN_LED[i][0] , HIGH );
        digitalWrite(PIN_LED[i][1] , HIGH );
      } else {
        digitalWrite(PIN_LED[i][0] , LOW );
        digitalWrite(PIN_LED[i][1] , LOW );
      }
      
    }


    
  }


  delay(100);
}

//Loop End






//입력 우선 순위, (r,l) > (u,d) > p
char get_joystick(int x, int y , int d) {

 
    if ( (abs(x-512) < 100) && (abs(y-512) < 100) )
      joystick_state = false;
    //(r,l) 입력   
    if ( joystick_state == false ) {
      
      if ( x < 90 ) {
        joystick_state = true;
        return 'l';
      } else if( x > 930 ){
        joystick_state = true;
        return 'r';
      }
      
      if ( y < 90 ) {
        joystick_state = true;
        return 'd';
      } else if ( y > 930 ) {
        joystick_state = true;
        return 'u';
      }   
    }



  return 'n';
}

byte set_queue() {
  for (byte i = 0; i < 8; i++) {
    if ( pump_queued[i] && current_pump == 50) {
      return i;
    }
  }

  return 50;
}

void control_pump(long diff_1000) {
  if ( current_pump == 50 && pump_time <= 300000 ) {
    digitalWrite(PIN_PUMP[0] , HIGH);
    digitalWrite(PIN_PUMP[1] , HIGH);
    analogWrite(PIN_PUMP[2], 0);
    Serial.println("Stop Pump");
    pump_time = 0;
  } else {
    digitalWrite(PIN_PUMP[0] , HIGH);
    digitalWrite(PIN_PUMP[1] , LOW);
    analogWrite(PIN_PUMP[2], 150);
    Serial.println(pump_time);
    Serial.println("Active Pump");
    pump_time += diff_1000;
  }
}

//Servo Control
void control_servo(void) {
  for ( int i = 0; i < 8; i++ ) {
    //set servo angle
    if ( pump_queued[i] ) {
       int angle = i*45;
       pump_state = false;
       
       if( angle <= 180 ) {
          servo_down.write(angle);
          servo_up.write(0);
       } else if ( angle >= 180 && angle <= 360 ) {
          servo_down.write(180);
          servo_up.write(angle-180);
       }
 
       delay(100);
       pump_state = true;
       break;
    }
  }
}
