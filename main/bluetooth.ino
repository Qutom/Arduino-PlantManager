#include "plant_info.h"
//For Bluetooth App
//Send WaterTemp , WaterMin , Light state data
//get Modified Option Value

char buff[128];

//Get Text to Serial1 And Execute Command

//Check command
boolean command_check(char* input) {
  boolean command = false;
  byte count_start_mark = 0;
  byte count_end_mark = 0;
  
  for(int i =0; i < strlen(input); i++) {
    char u = input[i];
    
    if ( u == '!' ) {
      if ( command ) {
        //Serial.print(String(u) + " : ");
        //Serial.println("Start Error");
        return false;
      } else {
        //Serial.print(String(u) + " : ");
        //Serial.println("Start");
        count_start_mark++;
        command = true;
      }

    } 

    if ( u == '?' ) {
      if ( command ) {
        //Serial.print(String(u) + " : ");
        //Serial.println("End");
        count_end_mark++;
        command = false;
      } else {
        //Serial.print(String(u) + " : ");
        //Serial.println("End Error");
        return false;
      }
      
    }


    
  }

  if ( command == false && count_end_mark == count_start_mark ) {
    return true;
  } else {
    return false;
  }
  
}

void read_data(void) {
  int index = 0;
  while( Serial1.available() ) {
    buff[index++] = Serial1.read();
  }

  buff[index] = '\0';
  

  if(buff[0] != '\0') {
    Serial.println(buff);
    if ( command_check(buff) ) {
      execute_command(buff);
    }
  }

  else
    return;
    
}

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
    
    Serial1.write(h);
  }  
  
  if ( t != '0' ) {
    
    Serial1.write(t);
  }

  
  Serial1.write(o);
  
  if (value == 100) {
    
    Serial1.write('0');
  }


}

void send_data(short index) {
    /*
     * Data Form : "!/(water_temp with ,)/(index)/(water_min)/(water_current)/(light_state)/(plant_name)/?"
     */
    Serial.print("Send Data ");
    Serial.write(index+'0');
    Serial.println("");
    
    //start command mark
    Serial1.write("!/");

    //index+1
    Serial1.write(index +'0');
    Serial1.write('/');
    
    //water_temp
    for(int i = 0; i < 144; i++) {
      write_to(water_temp[index][i]);
      if(i < 143) {
        Serial1.write(',');
        
      }
    }

    //water_min
    Serial1.write("/");
    
    write_to(water_min[index]);

    //water_current
    Serial1.write("/");
    
    write_to(water_current[index]);

    //light_state
    Serial1.write("/");
    
    if(light_state[index] == true) {
      Serial1.write('1');
      
    } else {
      Serial1.write('0');
      
    }

    //plant_name
    Serial1.write("/");

    
    int len = strlen(plant_name[index]);
    for (int i = 0; i < len; i++) {
      Serial1.write(plant_name[index][i]);
 
    }

    //end command mark
    Serial1.write("/?");

}


void set_option_bluetooth(int index, short water , short light , char* _name) {
  water_min[index] = water;
  if ( light == 1 ) {
    light_state[index] = true;
  } else {
    light_state[index] = false;
  }

  for (int i = 0; i < 8; i++) {
    Serial.println(light_state[i]);
  }
  strcpy(plant_name[index] , _name);
  option_modified = true;
  Serial.println(String("SET OPTION[") + index + "] : " +  water_min[index] + ", " + light_state[index]);
  
}


short get_digit(short start , short end, char *input) {
  short result= 0;
  short index = start + 1;
  char temp;
  for(index; index < end; index++) {
    temp = input[index];
    
    if ( temp >= '0' && temp <= '9' ) {
      result = (result * 10) + (temp - '0');
    }
  }
  
  return result;
  
}



void execute_command(char *input) {
  
  char temp;
  int index = 0;
  short command = -1;
  byte send_temp = -1;
  do {
    
    temp = input[index++];
    
    if( temp == '!' ) {
      command = 1;
    } else if ( temp == '?' ) {
      command = -1;
    } else if ( temp == '\0') {
      strcpy(input , ""); //buffer 초기화 
      return;
    } else {
      
      if( command >= 1 ) {
        if ( temp == 'i' ) {
          temp = input[index++] - '0'; // 숫자받기
          
          send_data(temp);
          
        } else if (temp == 'o') {
          temp = input[index++] - '0'; // 숫자받기
          
          short index_temp = temp;
        
          short seperator[3] = {index ,-10 , -10};
          index++;
          while( seperator[2] == -10 ) {
            temp = input[index++];
            if( temp == '/') {
              if ( seperator[1] == -10 ) {
                seperator[1] = index -1;
                index++; 
              }
            }
            
            if (temp == '/' && seperator[1] != -10) {
              seperator[2] = index-1;
              index++;
            }

          }
          short water_temp = get_digit(seperator[0] , seperator[1] , input);
          short light_temp = get_digit(seperator[1] , seperator[2]+1 , input);
          Serial.println(light_temp);
          char name_temp[18];
          
          short name_index = 0;
          while ( temp != '?' ) {
            temp = input[index++];
            if ( temp != '?') {
              name_temp[name_index++] = temp;
            }
              
          }
          
          name_temp[name_index] = '\0';
          
          set_option_bluetooth(index_temp, water_temp , light_temp , name_temp);
        }
      } else {
        continue;
      }
      
    } 
  
  }while(temp != '\0');
}
