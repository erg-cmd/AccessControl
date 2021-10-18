/**
 * @file	: firm_lcd.h
 * @brief	: Defines & declaracion de funciones de firm_lcd.c
 * @version	: 2.0
 * @date	: 16/10/2013
 * @author	: by_Ra!
===============================================================================
*/

#ifndef __FIRM_LCD_H__
#define __FIRM_LCD_H__

#include "lpc_types.h"

/* Defines	----------------------------------------- */

#define LCD_PORT  (uint32_t)0		//Puerto a usar por el LCD
#define LCD_PIN0  (uint32_t)18		//Primero de los 4 bits de datos
#define LCD_PINRS (uint32_t)10	  //Pin RS
#define LCD_PINE  (uint32_t)1		//Pin E


//Codigo hexa de las diferentes funciones del LCD

#define LCD_DISPLAY_CLEAR 			0x01
#define LCD_RETURN_HOME 				0x02
#define LCD_DEC_CURSOR 					0x04
#define LCD_INC_CURSOR 					0x06
#define LCD_DISP_OFF 						0x08
#define LCD_DISP_OFF_CUR_ON 		0x0A
#define LCD_DISP_ON_CUR_OFF 		0x0C
#define LCD_DISP_ON							0x0E
#define LCD_DISP_ON_CUR_BLINK 	0x0F
#define LCD_SHIFT_CUR_LEFT 			0x10
#define LCD_SHIFT_CUR_RIGHT 		0x14
#define LCD_SHIFT_DISP_LEFT 		0x18
#define LCD_SHIFT_DISP_RIGHT 		0x1C
#define LCD_FUNCTION_SET_4BIT		0x28
#define LCD_START_LINE1 				0x0
#define LCD_START_LINE2 				0x40
#define LCD_DDRAM								7

/*-----Prototipos de funciones-----*/

void LCD_Delay(UNS_32);

void LCD_Delay_40ns(UNS_32);

void LCD_WrtNibble(UNS_32);

void LCD_WrtCmd(UNS_8);

void LCD_WrtString(char*);		

void LCD_WrtData(UNS_8);

void LCD_GoToXY(UNS_8,UNS_8);	

void LCD_Init(void);

#endif

/*-----Fin de texto-----*/

