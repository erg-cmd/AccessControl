/**
 * @file	: firm_app.h
 * @brief	: Defines & declaracion de funciones de firm_app.c
 * @version	: 1.0
 * @date	: 11/11/2013
 * @author	: by_Ra!
===============================================================================
*/

#ifndef __FIRM_APP_H__
#define __FIRM_APP_H__

#include "lpc_types.h"

/*---------------------Prototipos de Funciones---------------------*/
void Ra_DelayHold(UNS_32);
FlagStatus Ra_Delay (UNS_32);
void Ra_Parpadeo(void);
void Ra_Parpadeo2(void);
Bool Ra_Comparar(UNS_8*,UNS_8*,int);

#endif
