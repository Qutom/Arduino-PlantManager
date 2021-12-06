eld#include <time.h>


byte water_temp[8][144];
byte water_min[8] = {25, 33, 27, 40, 50, 76 ,80 ,20};  //최소 수분량을 나타냅니다. water_min[n]은 1+n번 식물을 나타냅니다.
int water_current[8] = { 100, 100, 100, 100, 100, 100, 100, 100}; //현재 수분량을 나타냅니다.
boolean light_state[8] = { true , true , true , true , false , true , true , true }; //LED 상태를 나타냅니다.
char plant_name[8][18] = {"PLANT1", "PLANT2", "PLANT3", "PLANT4", "PLANT5", "PLANT6", "PLANT7", "PLANT8"};

int j = 1;

void write_to(byte value) {
  
  char h;
  char t;
  char o;

  o = (value % 10) + '0';

  if( o == 0 ) 
    o = '0';
  t = ((value % 100) / 10) +'0';
  h = (value / 100) + '0';
  if ( h != '0' ) {
    Serial.print(h);
    Serial1.write(h);
  }  
  if ( t != '0' ) {
    Serial.print(t);
    Serial1.write(t);
  }

  Serial.print(o);
  Serial1.write(o);
  Serial.print('0');
  if (value == 100) {
    Serial.print('0');
    Serial1.print('0');
  }


}
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  for (int i = 0; i< 8; i++) {
     for (int k = 0; k < 144; k++) {
      water_temp[i][k] = rand() % 100;
     }
  }

    for (int i = 0; i< 8; i++) {
     for (int k = 0; k < 144; k++) {
      Serial.print(water_temp[i][k]);
      Serial.print(',');
     }
     Serial.println(' ');
  }
  srand(time(0));
}

void loop() {
  char input = 'a';

  if ( Serial1.available() ) {
    //Serial.println( Serial1.available());
    input = Serial1.read();
    Serial.println(input);
  }
  
  if ( input >= '0' && input <= '7'  ) {
    Serial.println(String("Input : ") + input);
    Serial1.write("!");
    Serial.write("!");
    int x = 0;
    for(int i = 0; i < 144; i++) {
      write_to(water_temp[input-'0'][i]);
      if(i < 143) {
        Serial1.write(',');
        Serial.write(',');
      }
    }
    Serial1.write("/");
    Serial.write("/");

    write_to(water_min[input-'0']);
    
    Serial1.write("/");
    Serial.write("/");
    
    write_to(water_current[input-'0']);

    Serial1.write("/");
    Serial.write("/");
    write_to(100);
    Serial1.write("?");
    Serial.write("?");
  }
  
  //delay(200);
}
