#include "display.h"
#include "plant_info.h"

//LCD
void print_info() {
  
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print(String("") + (plant_num+1));
  lcd.print(PLANT_INFO[0]);
  lcd.print(plant_name[plant_num]);
  
  lcd.setCursor(0,1);
  lcd.print(PLANT_INFO[1]);
  lcd.setCursor(8,1);
  lcd.print(water_current[plant_num]);
  lcd.setCursor(14,1);
  lcd.print(water_min[plant_num]);

  lcd.setCursor(0,2);
  lcd.print(PLANT_INFO[2]);
  if ( light_state[plant_num] ) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }

  lcd.setCursor(0,3);
  lcd.print(PLANT_INFO[3]);

  lcd_state = 0;
}

void print_option() {
  option_selected = 0;
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("OPTION [NUM : ");
  lcd.print(plant_num + 1);
  lcd.setCursor(15,0);
  lcd.print("]");

  lcd.setCursor(0,1);
  lcd.print(PLANT_SETTING[1]);
  lcd.setCursor(13,1);
  lcd.print(water_min[plant_num]);
  lcd.setCursor(17,1);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.write((uint8_t)0);
  
  lcd.setCursor(0,2);
  lcd.print(PLANT_SETTING[2]);
  lcd.setCursor(13,2);
  if( light_state[plant_num] ) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }
  lcd.setCursor(0,3);
  lcd.print(PLANT_SETTING[3]);
  lcd_state = 1;
}

void print_option_input() {
  temp_input = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Water min%");

  lcd.setCursor(0,2);
  lcd.print("*-REMOVE/NUM-INPUT");
  
  lcd.setCursor(0,3);
  lcd.print("D-APPLY/C-EXIT");

  lcd.setCursor(2,1);
  lcd.print(String(temp_input));
  lcd_state = 2;
}

void update_display(int joystick_input , int keypad_input ) {

  switch(lcd_state) {
    case 0 : { //info 
      if ( keypad_input == 'A' ) { //option
        print_option();
        display_info = false;
      } else if ( joystick_input == 'r' || joystick_input == 'l') {
        if ( joystick_input == 'r' ) {   //plant_num 번호 증가
          plant_num = (plant_num >= 7) ? 0 : (plant_num + 1);
        } else if ( joystick_input == 'l' ) {  //plant_num 번호 감소
          plant_num = (plant_num <= 0 ) ? 7 : (plant_num - 1);
        }

        print_info();
        display_info = true;
      }
      break;
    }

    case 1 : {  //option
      if ( keypad_input == 'C' ) { //exit
        print_info();
        display_info = true;
      } else if ( keypad_input == 'D' ) { //select
        if ( option_selected == 1 ) { //light
          light_state[plant_num] = !light_state[plant_num];
          option_modified = true;
          print_option();
        } else {
          print_option_input();
          display_info = false;
        }
      } else {
        if ( joystick_input == 'u' || joystick_input == 'd' ) { //move selected option

          if( option_selected == 1 ) {
            lcd.setCursor(0,1);
            lcd.write((uint8_t)0);
            lcd.setCursor(0,2);
            lcd.print(" ");
          } else {
            lcd.setCursor(0,2);
            lcd.write((uint8_t)0);
            lcd.setCursor(0,1);
            lcd.print(" ");
          }
          
          option_selected = (option_selected == 0) ? 1 : 0;
          
         }
      }
      break;
    }

    case 2 : { //input
      if ( keypad_input >= '0' && keypad_input <= '9' && temp_input < 100 ) { //input
         
         temp_input = (temp_input * 10) + (keypad_input - '0');

         if ( temp_input > 100 ) {
          temp_input = 100;
         }
         lcd.setCursor(0,1);
         lcd.print("                    ");
         lcd.setCursor(2,1);
         lcd.print(String(temp_input));
         
      } else if ( keypad_input == '*' ) { //remove(backspace)
         if ( temp_input > 0 ) {
          temp_input /= 10;
         }

         lcd.setCursor(0,1);
         lcd.print("                    ");
         lcd.setCursor(2,1);
         lcd.print(String(temp_input));
      } else if ( keypad_input == 'C' ) { //exit
         print_option();
         display_info = false;
      } else if ( keypad_input == 'D' ) { //apply
         water_min[plant_num] = temp_input;
         option_modified = true;
         //EEPROM.update(plant_num , temp_input);
         print_option();
      }
      break;
    }
    
 }
}
