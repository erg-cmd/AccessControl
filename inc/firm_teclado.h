/**
 * @file		:firm_teclado.h
 * @brief		:
 * @version	: 1.0
 * @date		: 13/10/2013
 * @author	: by_Ra!
===============================================================================
*/

#ifndef __FIRM_TECLADO_H__
#define __FIRM_TECLADO_H__

#define		TECLADO_NO_KEY			0xFF

//en caso de no ser efectivo aumentar la cantidad
#define		TECLADO_REBOTES			2

/*---Definiciones de los Pines del Teclado---
* Nota:
* es preferible tomar de un port los pines en
* forma seguida o contigua sino replantear
*/
#define FILA_1		4	
#define FILA_2		6
#define FILA_3		3
#define FILA_4		5
#define COLUM_1 	7
#define COLUM_2 	9
#define COLUM_3 	8


#define TECLADO_FILA_1			(1 << FILA_1)
#define TECLADO_FILA_2			(1 << FILA_2)
#define TECLADO_FILA_3			(1 << FILA_3)
#define TECLADO_FILA_4			(1 << FILA_4)
#define TECLADO_COLUMNA_1		(1 << COLUM_1)
#define TECLADO_COLUMNA_2		(1 << COLUM_2)
#define TECLADO_COLUMNA_3		(1 << COLUM_3)
#define TECLADO_PORT 				2

/*------------------------------------------*/

// Prototipos de firm_teclado

void  TECLADO_Config		(void);
void  TECLADO_AntiRebote(void);
UNS_8 TECLADO_Lectura 	(void);
UNS_8 TECLADO_LeerTecla (void);
FlagStatus TECLADO_HayTecla (void);

/*------------------------------------------*/

#endif
