/*************************************************************
 * @file	: firm_teclado.c
 * @purpose	: Funciones para el manejo del teclado 
 * @version	: 1.0
 * @date	  : 13/10/2013
 * @author	: Elias Gracia
 *****************************************************************/

#include "lpc_types.h"
#include "lpc17xx_gpio.h"
#include "firm_teclado.h"


extern UNS_8 buffer_teclado;	//!< definido en el Main del programa

/*********************************************************************
 * @fn void TECLADO_Config()
 * @brief	seteo del GPIO para la entrada y salida del teclado
 * @param[in]	----
 * @return	----
 * Note: ----
 **********************************************************************/

void TECLADO_Config( void)
{
	//Definimos como salida las columnas
	GPIO_SetDir(TECLADO_PORT,(TECLADO_COLUMNA_1|TECLADO_COLUMNA_2|TECLADO_COLUMNA_3),1);
	//Definimos como entrada las filas
	GPIO_SetDir(TECLADO_PORT,(TECLADO_FILA_1|TECLADO_FILA_2|TECLADO_FILA_3|TECLADO_FILA_4),0);
	// Seteamos un 1 en las columnas
	GPIO_SetValue(TECLADO_PORT,(TECLADO_COLUMNA_1|TECLADO_COLUMNA_2|TECLADO_COLUMNA_3));
}


/*********************************************************************
 * @fn void TECLADO_AntiRebote()
 * @brief	elimina por software el rebote de las teclas, validando
 * 				por reiterar una cantidad de veces la tecla 
 * @param[in]	----
 * @return	----
 * Note: 
 **********************************************************************/

void TECLADO_AntiRebote( void ) {
	
	static UNS_8 CodigoAnterior = TECLADO_NO_KEY ;
	static UNS_8 EstadosEstables = 0 ;
	static UNS_8 CodigoActual ;
      
	CodigoActual = TECLADO_Lectura();				
	

	if( CodigoActual == TECLADO_NO_KEY ) {  //!< No se pulso tecla o rebote
		CodigoAnterior = TECLADO_NO_KEY ;
		EstadosEstables = 0 ;
		return ;
	}

	if( EstadosEstables == 0 )	{  //!< Primera vez
		CodigoAnterior = CodigoActual ;
		EstadosEstables = 1 ;
		return ;
	}

	if( EstadosEstables == TECLADO_REBOTES )	{  	//!< Acepto la tecla
		EstadosEstables = TECLADO_REBOTES + 1;	//!< Hará que no vuelva a entrar hasta que suelten la tecla
		buffer_teclado = CodigoAnterior ;	
		return ;
	}
	
	if( EstadosEstables > TECLADO_REBOTES )		//!< La tecla sigue pulsada
		return ;

	if( CodigoActual == CodigoAnterior )
		EstadosEstables ++ ;
	else
		EstadosEstables = 0 ;	
	return ;
}



/*********************************************************************
 * @fn void TECLADO_Lectura()
 * @brief	elimina por software el rebote de las teclas, validando
 * 			por reiterar una cantidad de veces la tecla 
 * @param[in]	----
 * @return	----
 * Note: considerar que estamos con pull-up en las entradas! Baje el nivel 
 * optimizacion del compilador porque no respectaba el codigo.
 **********************************************************************/

#pragma O0

UNS_8  TECLADO_Lectura( void ) 
{
	UNS_32 aux;
	
	//Barro Columna 1
	GPIO_SetValue(TECLADO_PORT,(TECLADO_COLUMNA_2|TECLADO_COLUMNA_3));
	GPIO_ClearValue(TECLADO_PORT,TECLADO_COLUMNA_1);
	aux = GPIO_ReadValue(TECLADO_PORT);
	if(!(aux & TECLADO_FILA_1)) return 1;
	if(!(aux & TECLADO_FILA_2)) return 4;
	if(!(aux & TECLADO_FILA_3)) return 7;
	if(!(aux & TECLADO_FILA_4)) return 10;//el 10 es "X", Cancelar o borrar
	
	//Barro Columna 2
	GPIO_SetValue(TECLADO_PORT,(TECLADO_COLUMNA_1|TECLADO_COLUMNA_3));
	GPIO_ClearValue(TECLADO_PORT,TECLADO_COLUMNA_2);
	aux = GPIO_ReadValue(TECLADO_PORT);
	if(!(aux & TECLADO_FILA_1)) return 2;
	if(!(aux & TECLADO_FILA_2)) return 5;
	if(!(aux & TECLADO_FILA_3)) return 8;
	if(!(aux & TECLADO_FILA_4)) return 0;
	
	//Barro Columna 3
	GPIO_SetValue(TECLADO_PORT,(TECLADO_COLUMNA_1|TECLADO_COLUMNA_2));
	GPIO_ClearValue(TECLADO_PORT,TECLADO_COLUMNA_3);
	aux = GPIO_ReadValue(TECLADO_PORT);
	if(!(aux & TECLADO_FILA_1)) return 3;
	if(!(aux & TECLADO_FILA_2)) return 6;
	if(!(aux & TECLADO_FILA_3)) return 9;
	if(!(aux & TECLADO_FILA_4)) {return 11;}//el 11 es el "Visto", Ok!
	else {return TECLADO_NO_KEY;}//Si no se presiono ninguna tecla.	
}

#pragma O2

/*********************************************************************
 * @fn FlagStatus TECLADO_HayTecla()
 * @brief	Si el Buffer del teclado es distinto a NO_KEY retorna 1 sino 
 un 0
 * @param[in]	----
 * @return FlagStatus
 * Note: 
 **********************************************************************/

FlagStatus TECLADO_HayTecla ()
{
   if(buffer_teclado != TECLADO_NO_KEY) 
	 {
		 return SET;
	 }else
	 {
		return RESET;
	 }
}
/*********************************************************************
 * @fn void TECLADO_Hay_Tecla()
 * @brief	Acceder a la variable buffer_teclado indirectamente informando 
 * 			si hay o no tecla y borrar el buffer siempre que se lea para no leer
 * 			2 veces el mismo dato
 * @param[in]	----
 * @return	----
 * Note: 
 **********************************************************************/

UNS_8 TECLADO_LeerTecla ()
{
    UNS_8 aux;
   	aux = buffer_teclado;
		buffer_teclado = TECLADO_NO_KEY;
		return aux;
}
