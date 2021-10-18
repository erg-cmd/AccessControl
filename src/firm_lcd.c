/**
 * @file	: firm_lcd.c
 * @brief	: Inicializacion en 4 bits y funciones de manejo del LCD WH1602B
 * @version	: 2.0
 * @date	: 16/10/2013
 * @author	: by_Ra!
  * NOTE: faltaria la implementacion de una funcion que interprete un dato y me 
	lo imprima en su representacion decimal, hexa, etc.
===============================================================================
*/
#include "lpc_types.h"
#include "lpc17xx_gpio.h"
#include "firm_lcd.h"

//Se define un tipo de datos, el cual es un puntero a una matriz que contiene las direcciones
//de cada puerto, de esta forma se puede hacer configurable el puerto a usar
static LPC_GPIO_TypeDef(* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

//debe definirse en el main
extern volatile uint32_t msTicks;

/*********************************************************************
 * @fn			: void LCD_Delay ( UNS_32 msec )
 * @brief		: Genera un retraso de 0.x milisegundos
 * @param[in]	: UNS_32 msec
 * @return		: void
 * Note:  ----
 **********************************************************************/

void LCD_Delay(UNS_32 msec) 
{
	volatile uint32_t done = msTicks + msec;
	while (msTicks < done);
}

/*********************************************************************
 * @fn			: void LCD_Delay_40ns ( UNS_32 nsec )
 * @brief		: Genera un retraso multiplo de 40ns, se pretende generar un delay
 * de 280ns o mayor
 * @param[in]	: UNS_32 nsec
 * @return		: void
 * Note:  necesario debido a la rapidez del puerto para cambiar de estados
 * el LCD necesita un ancho minimo de 250ns del ENABLE para validar
 **********************************************************************/

void LCD_Delay_40ns(UNS_32 nsec) 
{
	volatile uint32_t done = LPC_RIT->RICOUNTER + nsec;
	while (LPC_RIT->RICOUNTER < done);
}

/*********************************************************************
 * @fn			: void LCD_WrtNibble ( UNS_8 val )
 * @brief		: coloca el nibble bajo en LCD_PIN0, previo de un retardo 3-4ms
 * @param[in]	: UNS_8 val
 * @return		: void
 * Note:  ----
 **********************************************************************/
 
void LCD_WrtNibble(UNS_32 nibble) 
{
	uint32_t mask=0;
	LCD_Delay(4);//3

	mask=(LPC_GPIO[LCD_PORT]->FIOPIN & ~(0x0F << LCD_PIN0));
	LPC_GPIO[LCD_PORT]->FIOPIN = mask | ((nibble & 0x0F) << LCD_PIN0);
}
	
/*********************************************************************
 * @fn			: void LCD_WrtCmd ( UNS_8 val )
 * @brief		: Manda comandos al LCD
 * @param[in]	: UNS_8 val
 * @return		: void
 * Note:  ----
 **********************************************************************/

void LCD_WrtCmd(UNS_8 val)
{
	//Para comandos RS = 0
	LCD_WrtNibble(val>>4);
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINRS); //RS = 0
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE);  // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE);  // E = 0
	LCD_WrtNibble(val);
	
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINRS); //RS = 0
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE);  // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE);  // E = 0
	
}


/*********************************************************************
 * @fn			: void LCD_WrtData ( UNS_8 caracter )
 * @brief		: Escribe un caracter en el display
 * @param[in]	: UNS_8 caracter
 * @return		: void
 * Note:  ----
 **********************************************************************/

void LCD_WrtData(UNS_8 caracter)
{
	//Para escribir caracteres en display RS = 1
	LCD_WrtNibble(caracter>>4);
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINRS); //RS = 1
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE);  // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE);  // E = 0
	LCD_WrtNibble(caracter);
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINRS); //RS = 1
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE);  // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE);  // E = 0
}

/*********************************************************************
 * @fn			: void LCD_WrtString ( char* string )
 * @brief		: Imprime un string de uint8_t, fin de cadena NULL
 * @param[in]	: UNS_8* string
 * @return		: void
 * Note:  ----
 **********************************************************************/

void LCD_WrtString(char* string)
{
	LCD_Delay(5);
	while(*string)
	{
		LCD_WrtData(*string++);
	}
}

/*********************************************************************
 * @fn			: void LDC_GoToXY ( UNS_8 x, UNS_8 y )
 * @brief		: setea el cursor en la posicion x,y del LCD; X(0,1); Y(0,15)
 * @param[in]	: x posicion de fila(0,1) ; y posicion de columna (0,15)	 		
 * @return		: void
 * Note: 
 **********************************************************************/

void LCD_GoToXY ( UNS_8 x, UNS_8 y ) 
{
    if ( y==0 )
    	//LCD_WrtCmd((1<<LCD_DDRAM)+LCD_START_LINE1+x);
		LCD_WrtCmd(0x80 + x);
    else /* y==1 */
    	//LCD_WrtCmd((1<<LCD_DDRAM)+LCD_START_LINE2+x);
		LCD_WrtCmd(0xC0 + x);
}

/*********************************************************************
 * @fn			: void LCD_Init ( void )
 * @brief		: Configuracion de puertos, inicializacion y config-
 * -guracion particular
 * @param[in]	: void	 		
 * @return		: void
 * Note: Se configura el puerto, se pasa a 4 bits
 * se realizan las configuraciones segun el usuario
 **********************************************************************/

void LCD_Init(void) 
{
	// set all pins to output
	LPC_GPIO[LCD_PORT]->FIODIR |= (0x0F << LCD_PIN0)| (1 << LCD_PINE) | (1 << LCD_PINRS); 
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE) |(1<<LCD_PINRS); // E = 0 y RS = 0
	LCD_Delay(2);
	LCD_WrtCmd(LCD_FUNCTION_SET_4BIT);//0x38 para 8bits y 28 para 4bits
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE); // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE); // E = 0
	LCD_Delay(2);
	LCD_WrtCmd(LCD_DISP_ON_CUR_OFF);
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE); // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE); // E = 0
	LCD_Delay(2);
	LCD_WrtCmd(LCD_DISPLAY_CLEAR);
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE); // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE); // E = 0
	LCD_Delay(2);
	LCD_WrtCmd(LCD_INC_CURSOR);
	LPC_GPIO[LCD_PORT]->FIOSET = (1 << LCD_PINE); // E = 1
	LCD_Delay_40ns(5);//retardo 280ns
	LPC_GPIO[LCD_PORT]->FIOCLR = (1 << LCD_PINE); // E = 0
	LCD_Delay(2);
	//LCD_WrtCmd(LCD_DISPLAY_ON);	// Hace visible el cursor y parpadea	
}

