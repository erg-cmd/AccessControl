/**
 * @file	: firm_app.c
 * @brief	: funciones de mi autoria
 * @version	: 1.0
 * @date	: 11/11/2013
 * @author	: by_Ra!
===============================================================================
*/

#include "firm_app.h"
#include "lpc_types.h"
#include <LPC17xx.H>

/*************************Variables Globales**************************/
extern UNS_32 msTicks;
static UNS_32 i;
FlagStatus LED_OO1 = RESET;
FlagStatus LED_OO2 = RESET;

/*********************************************************************
 * @fn			: void Ra_Delay ( UNS_32 msec )
 * @brief		: Genera un retraso de 0.x milisegundos 
 * @param[in]	: UNS_32 msec
 * @return		: void
 * Note:  Necesario crear una variable global como base de tiempo, msTicks
 * que se incremente cada 1ms, 
 **********************************************************************/
void Ra_DelayHold (UNS_32 msec)
{
	while(!Ra_Delay(msec));
}

/*********************************************************************
 * @fn			: bool Ra_Delay? ( UNS_32 msec )
 * @brief		: se setea el tiempo deseado en msec, cuando se cumple el
 * tiempo impuesto retorna 1, en caso contrario 0.
 * @param[in]	: UNS_32 msec
 * @return		: 
 * Note:  Necesario crear una variable global como base de tiempo, msTicks
 * que se incremente cada 1ms, 
 **********************************************************************/
FlagStatus Ra_Delay (UNS_32 msec)
{
	static volatile UNS_32 tiempo = 0;
	//Si tiempo es 0, no esta activo. Cargando el valor
	//de msTicks lo seteamos
	if (tiempo == 0) tiempo = msTicks;
	
		if (msTicks - tiempo > msec)
		{
			tiempo = 0;return SET;
		}else
		{
			return RESET;
		}	
}

/*********************************************************************
 * @fn		void Ra_Parpadeo (void)
 * @brief		cambia el estado del led P3.25 si paso 300 ms
 * @param[in]	void 		 		
 * @return	void
 * Note: Dependiente de RIT para generar la base de tiempo
 **********************************************************************/
void Ra_Parpadeo(void)
{
// Parpadeo del LED
			if((msTicks - i) > 300)
			{
				if(LED_OO1){LPC_GPIO3->FIOCLR = _BIT(25);}
				else{LPC_GPIO3->FIOSET = _BIT(25);}
				LED_OO1 = !LED_OO1;
				i = msTicks;
			}
}

/*********************************************************************
 * @fn		void Ra_Parpadeo (void)
 * @brief		cambia el estado del led P3.25 si paso 300 ms
 * @param[in]	void 		 		
 * @return	void
 * Note: Dependiente de RIT para generar la base de tiempo
 **********************************************************************/
void Ra_Parpadeo2(void)
{
// Parpadeo del LED
			if((msTicks - i) > 300)
			{
				if(LED_OO2){LPC_GPIO3->FIOCLR = _BIT(26);}
				else{LPC_GPIO3->FIOSET = _BIT(26);}
				LED_OO2 = !LED_OO2;
				i = msTicks;
			}
}

/*********************************************************************
 * @fn		Bool Ra_Comparar (char *)
 * @brief		cambia el estado del led P3.25 si paso 300 ms
 * @param[in]	void 		 		
 * @return	void
 * Note: Dependiente de RIT para generar la base de tiempo
 **********************************************************************/
Bool Ra_Comparar(UNS_8* cadena1,UNS_8* cadena2,int N)
{
	volatile UNS_8 i = 0;	
	while((cadena1[i] == cadena2[i+1]) && (i < 10))i++;
	if(i == N){return TRUE;}
	else{return FALSE;}		
}

/*****************************Fin del Texto*******************************/
