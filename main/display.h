#ifndef _DISPLAY_H_
#define _DISPLAY_H_

//ȭ��ǥ ����� ���� ���� 
byte SELECTED_ARROW[] = {
  							 B00000,
  							 B00100,
  							 B00010,
  							 B11111,
  						 	 B11111,
  							 B00010,
  							 B00100,
  							 B00000
							}; 
							
							
//�Ĺ� ���� 
const char PLANT_INFO[4][21] = { 
								 " , ", 
								 "WATER :     /     %",
								 "LIGHT : ",
								 "A - HELP / B - OPTION"
							   };

//�ɼ�							   
const char PLANT_SETTING[4][21] = { 
								 "OPTION [ ]", 
								 " MIN_WATER : ",
								 " LIGHT     : ",
								 "BUTTON-SELECT/C-EXIT "
							   };
							   

#endif 
