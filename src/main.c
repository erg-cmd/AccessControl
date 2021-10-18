/**
 * @file		:main.c
 * @brief		:Control de acceso, corriendo 7 tareas en rtos
 * 	
 * @version	: 1.1 Final
 * @date		: 09/02/2014
 * @author	: by_Ra!
 * Note: Version Final, sin debugs
===============================================================================
*/

/*****************Includes**************************/

// de RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
// Firm Mias
#include "firm_uart.h"
#include "firm_lcd.h"
#include "firm_teclado.h"
#include "firm_app.h"
// de Drivers
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_libcfg_default.h"
#include "lpc17xx_rit.h"
#include "lpc17xx_rtc.h"
#include "lpc_types.h"
//de FatFS
#include "ff.h"
#include "SPI_MSD_Driver.h"

/*******************Defines**************************/
#define LED1					(1<<25)
#define LED2					(1<<26)
#define PORT_LED			3
#define _BYTESdeDATO 	10
#define _BYTESdeTRAMA 14

#define SETEO_RTC 		0
#define SETEO_SD 			1
#define SETEO_OCIOSO 	2

#define ID_INACTIVO 		0
#define ID_ESPERANDO 		1
#define ID_VERIFICANDO 	2
#define ID_BLOQUEADO 		3

#define SD_INACTIVO 		0
#define SD_LEER_ACCESO 	1
#define SD_LEER_REGISTRO 			2
#define SD_ESCRIBIR_REGISTRO 	3

#define PUERTA_ABRIR 	0 
#define PUERTA_CERRAR 1

#define TECLADO_ACTIVO		1
#define TECLADO_INACTIVO 	0

/*******************Variables Externas**************************/

// declaradas en firm_uart.c
extern UART_BUFFER bc;
extern FlagStatus TxIntStat;

/*******************Variables Globales**************************/
RTC_TIME_Type RTC_Tiempo;//Estructura para RTC
UNS_8 mi_tecla;
xSemaphoreHandle Semaforo_Puerta, Semaforo_SD, Semaforo_Teclado,Semaforo_ID,Semaforo_SETEO;
xTaskHandle Priori_TECLADO,Priori_SD,Priori_PUERTA,Priori_ID,Priori_SETEO;
UNS_8 buffer[_BYTESdeTRAMA],ID_0[_BYTESdeDATO],ID_1[_BYTESdeDATO],ID_2[_BYTESdeDATO],ID_3[_BYTESdeDATO]; //Para descargo los datos de la UART
UNS_8 estado_SETEO, estado_TECLADO, estado_SD, estado_PUERTA, estado_ID;
volatile UNS_8 buffer_teclado = TECLADO_NO_KEY;
volatile UNS_32 msTicks;
FATFS fs;
FIL 	Acceso,Registro;
FRESULT res;
UINT br,btr = 10,bw,btw = 10;

char texto0[] = "...Bienvenido!";
char texto1[] = "Seteo Fecha/Hora";
char texto2[] = " dd/mm/aa hh:mm";
char texto3[] = " La Fecha es:";

char menu_SD_1[] = "SD detectada!";
char menu_SD_2[] = "SD NO detectada";
char menu_SD_3[] = "Inserte SD...";
char menu_SD_4[] = "Encontrado";
char menu_SD_5[] = "No encontrado";
char menu_SD_6[] = "Acceso.txt";
char menu_SD_7[] = "Registro.txt";
char menu_SD_8[] = "Tarj No Valida";
char menu_SD_9[] =  "        O       ";
char menu_SD_10[] = "      (   )     ";
char menu_SD_11[] = "     (     )    ";
char menu_SD_12[] = "    (       )   ";
char menu_SD_13[] = "   (         )  ";

char menu_PUERTA_ABRIR[] = "Tarj Valida";

/**********************************************************************
 * @fn 				: void RIT_IRQHandler ( void )
 * @brief			: incrementa una variable cada 1ms, onda systick
 * @param[in]	: void
 * @return		: void
 * Note				:-
 **********************************************************************/
void RIT_IRQHandler()
{
	RIT_GetIntStatus(LPC_RIT);	//Borramos el Flag
	msTicks++;
}

/*******************Tareas*******************************/
void vOOLED1(void *);
void vTECLADO(void*);
void vSETEO(void* );
void vSD(void* );
void vPUERTA(void* );
void vID(void* );
/*********************************************************************
 * @fn int main (void)
 * @brief		
 * @param[in]	 		 		
 * @return	
 * Note: En caso de no tenre suficiente memoria o creacion fallida de las 
 * tareas imprime Crasheo...
 **********************************************************************/
int main ( void )
{
	portBASE_TYPE Tarea1,Tarea2,Tarea3,Tarea4,Tarea5,Tarea6; //para verificar la creacion de la tarea
	// Variables para la configuracion de la UART(MODO, BUFFER, PINES)
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	PINSEL_CFG_Type PinCfg;
	RIT_CMP_VAL  RIT_Tiempo;//Estructura para RIT
	vSemaphoreCreateBinary(Semaforo_Puerta);
	vSemaphoreCreateBinary(Semaforo_SD);
	vSemaphoreCreateBinary(Semaforo_Teclado);
	vSemaphoreCreateBinary(Semaforo_ID);
	vSemaphoreCreateBinary(Semaforo_SETEO);
	//Seteo del PORT para la UART
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg); 
	//Configura el struct de configuracion UART con datos por defecto (9600,8,1,n) 
	UART_ConfigStructInit(&UARTConfigStruct);
	// Carga el tipo de dato en la UART0
	UART_Init(LPC_UART0, &UARTConfigStruct);
	//Configura el struct de configuracion FIFO con datos por defecto
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	// Carga el tipo de dato para la UART0
	UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);
	//Seteo de funciones call backs
	//(si el Handler no las encuentra deshabilita la respectiva interrupcion)
	UART_SetupCbs(LPC_UART0, 0, (void *)UART_IntReceive);
	UART_SetupCbs(LPC_UART0, 1, (void *)UART_IntTransmit);
	UART_SetupCbs(LPC_UART0, 3, (void *)UART_IntErr);
	// Habilito la transmision
	UART_TxCmd(LPC_UART0, ENABLE);
  //Habilito la interrupcion por recepcion  
	UART_IntConfig(LPC_UART0, UART_INTCFG_RBR, ENABLE);
	TxIntStat = RESET; // = 0
	//Limpio los indices de ambos buffer
	__BUF_RESET(bc.rx_head);
	__BUF_RESET(bc.rx_tail);
	__BUF_RESET(bc.tx_head);
	__BUF_RESET(bc.tx_tail);
	//Inicializacion TECLADO
	TECLADO_Config();
	//Inicializo el RTC
	RTC_Init(LPC_RTC);
	RTC_ResetClockTickCounter(LPC_RTC);
	RTC_CalibCounterCmd(LPC_RTC, DISABLE);
	//configuro los valores del RIT
	RIT_Tiempo.CMPVAL = 25000;	//genero una interrupcion cada 1ms
	RIT_Tiempo.COUNTVAL = 0x00000000;
	RIT_Tiempo.MASKVAL = 0x00000000;
	//seteo de los valores a RIT
	RIT_Init(LPC_RIT);
	RIT_TimerConfig(LPC_RIT,&RIT_Tiempo);
	RIT_TimerClearCmd(LPC_RIT,ENABLE);
	//habilito la interrupcion para el RIT
	NVIC_EnableIRQ(RIT_IRQn);
	// Enable Interrupt for UART0 channel 
  NVIC_EnableIRQ(UART0_IRQn);
	//seteo de los puertos
	LPC_GPIO3->FIODIR |= (LED1 | LED2);
	LPC_GPIO3->FIOCLR |= (LED1 | LED2);
	//Inicializacion LCD
	LCD_Init();
	LCD_WrtCmd(LCD_DISPLAY_CLEAR);
	
	//Creacion de las tareas
	Tarea1 = xTaskCreate(vOOLED1,(signed char*)"LED1",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY,NULL );
	Tarea2 = xTaskCreate(vSETEO,(signed char*)"SETEO",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,Priori_SETEO );
	Tarea3 = xTaskCreate(vTECLADO,(signed char*)"TECLADO",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY,Priori_TECLADO );
	Tarea4 = xTaskCreate(vSD,(signed char*)"SD",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY,Priori_SD);
	Tarea5 = xTaskCreate(vPUERTA,(signed char*)"PUERTA",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,Priori_PUERTA );
	Tarea6 = xTaskCreate(vID,(signed char*)"ID",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY,Priori_ID);
	if (Tarea1 & Tarea2 & Tarea3 & Tarea4 & Tarea5 & Tarea6)
	{
		vTaskStartScheduler();
	}
	LCD_WrtString("Crasheo..."); //falló al crear la Tarea
	UARTSend(LPC_UART0,(UNS_8*)"Crasheo...\n\r",12);
	//loop
	while(1);
}

/*------------------------Implementacion-------------------------------------*/
/*********************************************************************
 * @fn void vOOLED1 (void * pvParameters)
 * @brief		invierte el estado de un led determinado un tiempo
 * @param[in]	void* 		 		
 * @return	void	
 * Note: 
 **********************************************************************/
void vOOLED1(void * pvParameters)
{
	static volatile FlagStatus OO_LED1 = RESET;
	while(1)
	{
		if(OO_LED1){LPC_GPIO3->FIOCLR |= LED1;}
		else{LPC_GPIO3->FIOSET |= LED1;}
		OO_LED1 = !OO_LED1;
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}

/*********************************************************************
 * @fn void vTECLADO (void *pvParameters)
 * @brief		En estado Inactivo toma el semaforo para bloquearse y en
 * estado activo barre el teclado.
 * @param[in]	void 		 		
 * @return	void
 * Note:  AntiRebote tiene un periodo de 250ns
 **********************************************************************/
void vTECLADO (void * pvParameters)
{
	estado_TECLADO = TECLADO_INACTIVO;
	xSemaphoreTake( Semaforo_Teclado, portMAX_DELAY );
	while(1)
	{
		//UARTSend(LPC_UART0,Texto_TECLA,sizeof(Texto_TECLA));//para debug
		switch(estado_TECLADO)
		{
			case TECLADO_INACTIVO:
					xSemaphoreTake( Semaforo_Teclado, portMAX_DELAY );
			break;
			case TECLADO_ACTIVO:
					TECLADO_AntiRebote();				
			break;
		}
		vTaskDelay( 50 / portTICK_RATE_MS);//demoramos la tarea por 50 ms			
	}
}

/*********************************************************************
 * @fn void vSETEO (void *Parameters)
 * @brief	Seteo_RTC Configura el RTC; Seteo_SD Detecta presencia;
 SETEO_OCIOSO cambia las prioridades,activa otras tareas y bloquea esta tarea
 * @param[in]	void 		 		
 * @return	void
 * Note: 
 **********************************************************************/
 void vSETEO (void * pvParameters)
{
	volatile Bool BUCLE;
	unsigned portBASE_TYPE mi_priori;
	static UNS_8 indice = 1;
//	portTickType xDespierto;
//	xDespierto = xTaskGetTickCount();
	estado_SETEO = SETEO_RTC;	
	while(1)
	{
		switch(estado_SETEO)
		{
			case SETEO_RTC:	//Config del RTC
					//UARTSend(LPC_UART0, Texto_SETEO_RTC,sizeof(Texto_SETEO_RTC)); //Para debug
					LCD_WrtString(texto0);	//Mensaje de Bienvenida
					mi_priori = uxTaskPriorityGet(Priori_SETEO); //Prioridad de la actual tarea
					vTaskPrioritySet(Priori_TECLADO, mi_priori); //Seteamos otra tarea con la misma prioridad
					vTaskDelay(2000/portTICK_RATE_MS);//Delay de 2s
					LCD_GoToXY(0,0); 
					LCD_WrtString(texto1);//Imprime "Seteo Fecha/Hora"
					LCD_GoToXY(0,1);
					LCD_WrtString(texto2);//Imprime " dd/mm/aa hh:mm"
					LCD_WrtCmd(LCD_DISP_ON_CUR_BLINK);//Parpadeo de cursor (probar con 0x0D)
					LCD_GoToXY(1,1);
					xSemaphoreGive(Semaforo_Teclado); //Ponemos en ready la Tarea Teclado
					estado_TECLADO = TECLADO_ACTIVO;	//cambiamos el estado de teclado
					while(BUCLE)
					{	
						if(TECLADO_HayTecla())//tecla presionada? osea distinto de NO_KEY
						{
							mi_tecla = TECLADO_LeerTecla() + 48;// +48 para que sean imprimibles por el LCD
							switch(mi_tecla)
							{								
								case ';'://Confirmacion
										LCD_WrtCmd(LCD_DISPLAY_CLEAR);
										LCD_WrtString(texto3);
										LCD_GoToXY(1,1);
										//Verficamos la veracidad de los datos
											if(RTC_Tiempo.DOM > 31) RTC_Tiempo.DOM = 31;
											if(RTC_Tiempo.MONTH > 12) RTC_Tiempo.MONTH = 12;
											if(RTC_Tiempo.HOUR > 23) RTC_Tiempo.HOUR = 00;
											if(RTC_Tiempo.MIN > 59) RTC_Tiempo.MIN = 00;
										//Escribimos en pantalla
										LCD_WrtData((RTC_Tiempo.DOM/10)+48);LCD_WrtData((RTC_Tiempo.DOM%10)+48);
										LCD_WrtData('/');
										LCD_WrtData((RTC_Tiempo.MONTH/10)+48);LCD_WrtData((RTC_Tiempo.MONTH%10)+48);
										LCD_WrtData('/');
										LCD_WrtData(((RTC_Tiempo.YEAR%100)/10)+48);LCD_WrtData((RTC_Tiempo.YEAR%10)+48);
										LCD_WrtData(' ');
										LCD_WrtData((RTC_Tiempo.HOUR/10)+48);LCD_WrtData((RTC_Tiempo.HOUR%10)+48);
										LCD_WrtData(':');
										LCD_WrtData((RTC_Tiempo.MIN/10)+48);LCD_WrtData((RTC_Tiempo.MIN%10)+48);
										LCD_WrtData('?');
										while(!TECLADO_HayTecla())vTaskDelay(100/portTICK_RATE_MS);//tecla presionada? osea distinto de NO_KEY										
										mi_tecla = TECLADO_LeerTecla()+48;
										if(mi_tecla == ';')
										{
											LCD_WrtCmd(LCD_DISP_ON_CUR_OFF);				//Cursor a la normalidad
											RTC_SetFullTime (LPC_RTC, &RTC_Tiempo); //Copiamos los datos al RTC
											RTC_Cmd(LPC_RTC, ENABLE); 							//Iniciamos la cuenta
											estado_SETEO = SETEO_SD;										//cambiamos al siguiente estado
											BUCLE = FALSE; 													//condicion de salida del loop
										}							
								break;							
								default://Numeros entre 0 - 9
										switch(indice)
										{
											case 1:RTC_Tiempo.DOM = ((mi_tecla - 48)*10);break;
											case 2:RTC_Tiempo.DOM += (mi_tecla - 48);break;
											case 4:RTC_Tiempo.MONTH = ((mi_tecla - 48)*10);break;
											case 5:RTC_Tiempo.MONTH += (mi_tecla - 48);break;	
											case 7:RTC_Tiempo.YEAR = 2000 + ((mi_tecla - 48)*10);break;
											case 8:RTC_Tiempo.YEAR += (mi_tecla - 48);break;
											case 10:RTC_Tiempo.HOUR = ((mi_tecla - 48)*10);break;	
											case 11:RTC_Tiempo.HOUR += (mi_tecla - 48);break;
											case 13:RTC_Tiempo.MIN = ((mi_tecla - 48)*10);break;
											case 14:RTC_Tiempo.MIN += (mi_tecla - 48);break;									
										}
										LCD_WrtData(mi_tecla);	//escribimos numero
										indice++;
										//Si esta entre numeros lo acercamos al prox de la derecha
										if(indice == 3 | indice == 6 | indice == 9 | indice == 12)
										{
											indice++;
											LCD_GoToXY(indice,1);
										}
										//Retornamos el cursor a la pos 1
										if(indice == 15){indice = 1; LCD_GoToXY(indice,1);}
								break;
							}
						}
						vTaskDelay(100/ portTICK_RATE_MS);//Atendemos las demas tareas
					}
					estado_TECLADO = TECLADO_INACTIVO; // Bloqueamos la tarea teclado
			break;
			case SETEO_SD:	//Config y Presencia de SD
						BUCLE = TRUE;
						//UARTSend(LPC_UART0, Texto_SETEO_SD,sizeof(Texto_SETEO_SD)); //Para debug
						MSD_SPI_Configuration();//Configuracion SPI -> SD
						while(BUCLE)
						{
							if( _card_insert() == 0 ) //Verificamos la presencia de la SD
							{
							LCD_WrtCmd(LCD_DISPLAY_CLEAR);
							LCD_WrtString(menu_SD_1);//printf("SD detectada");
							BUCLE = FALSE;//condicion de salida
							f_mount(0,&fs);	//abrimos el dispositivo
							estado_SD = SD_LEER_ACCESO;	//cambiamos al siguiente estado
							estado_SETEO = SETEO_OCIOSO;	
							}else
							{
								LCD_WrtCmd(LCD_DISPLAY_CLEAR);
								LCD_WrtString(menu_SD_2);//printf("SD no detectada");
								LCD_GoToXY(0,1);
								LCD_WrtString(menu_SD_3);//printf("inserte SD...");
								while( _card_insert() != 0 );				
							}
							vTaskDelay (500 / portTICK_RATE_MS);//Delay de 2s
						}
				break;
				case SETEO_OCIOSO:	//Fin del Seteo				
								//UARTSend(LPC_UART0, Texto_SETEO_OCIOSO,sizeof(Texto_SETEO_OCIOSO)); //Para debug
								mi_priori = uxTaskPriorityGet(NULL);
								vTaskPrioritySet(Priori_SD,mi_priori); //Subimos de Prioridad a la Tarea
								vTaskPrioritySet(Priori_SETEO, tskIDLE_PRIORITY); //Bajamos de Prioridad a la Tarea
								vTaskPrioritySet(Priori_TECLADO, tskIDLE_PRIORITY); //Bajamos de Prioridad a la Tarea
								xSemaphoreGive(Semaforo_SD);//Habilitamos Tarea SD
								xSemaphoreTake(Semaforo_SETEO, portMAX_DELAY);	//Bloqueamos la tarea							
				break;
		}
		vTaskDelay (200 / portTICK_RATE_MS);	//retrasamos la tarea por 5s
	}
} 
/*********************************************************************
 * @fn void vSD (void *Parameters)
 * @brief		Realiza 3 Tareas: detectar presencia de SD, escribir y leer
 * @param[in]	void 		 		
 * @return	void
 * Note: 
 **********************************************************************/
void vSD (void * pvParameters)
{
	static volatile Bool BUCLE;
	unsigned portBASE_TYPE mi_priori;
	estado_SD = SD_INACTIVO;
	//xSemaphoreTake( Semaforo_SD, portMAX_DELAY );
	while(1)
	{
		xSemaphoreTake( Semaforo_SD, portMAX_DELAY );
		switch(estado_SD)
		{
			case SD_INACTIVO:
						vTaskDelay(200/ portTICK_RATE_MS);
			break;
			
			case SD_LEER_ACCESO:
						//UARTSend(LPC_UART0, Texto_SD_LEER_ACCESO,sizeof(Texto_SD_LEER_ACCESO)); //Para debug
						res = f_open(&Acceso,"Acceso.txt",FA_OPEN_EXISTING|FA_READ);
						if (res == FR_OK)
						{
							//Leemos los ID de la SD
							f_read(&Acceso,ID_0,btr,&br); //Levanto el string
							//UARTSend(LPC_UART0,ID_0,br); //para debug			
							f_lseek(&Acceso, f_tell(&Acceso) + 1);//Adelantamos la pos del puntero
							f_read(&Acceso,ID_1,btr,&br); //Levanto el string
							//UARTSend(LPC_UART0,ID_1,br); //para debug
							f_lseek(&Acceso, f_tell(&Acceso) + 1);//Adelantamos la pos del puntero
							f_read(&Acceso,ID_2,btr,&br); //Levanto el string
							//UARTSend(LPC_UART0,ID_2,br); //para debug
							f_lseek(&Acceso, f_tell(&Acceso) + 1);//Adelantamos la pos del puntero
							f_read(&Acceso,ID_3,btr,&br); //Levanto el string
							//UARTSend(LPC_UART0,ID_3,br); //paradebug
							f_close(&Acceso); //cerramos el archivo
							vTaskDelay(2000 / portTICK_RATE_MS); //Tiempo para msj SD detectada
							LCD_WrtCmd(LCD_DISPLAY_CLEAR);
							LCD_WrtString(menu_SD_4);//Encontrado
							LCD_GoToXY(0,1);
							LCD_WrtString(menu_SD_6);//Acceso.txt
							//cambiamos a los siguientes estados
							estado_ID = ID_ESPERANDO;
							mi_priori = uxTaskPriorityGet(NULL);
							vTaskPrioritySet(Priori_ID,mi_priori);
							xSemaphoreGive(Semaforo_ID); //Habilitamos la Tarea ID
						}else
						{
							LCD_WrtCmd(LCD_DISPLAY_CLEAR);
							LCD_WrtString(menu_SD_5);//No encontrado
							LCD_GoToXY(0,1);
							LCD_WrtString(menu_SD_6);//Acceso.txt
							estado_SETEO = SETEO_SD; //Volvemos a setear la SD
							mi_priori = uxTaskPriorityGet(NULL);
							vTaskPrioritySet(Priori_SD, tskIDLE_PRIORITY);//Bajamos de Prioridad a la Tarea
							vTaskPrioritySet(Priori_SETEO, mi_priori);  //Subimos de Prioridad a la Tarea
							xSemaphoreGive(Semaforo_SETEO); //Habilitamos la Tarea SETEO
						}
						estado_SD = SD_INACTIVO;
						vTaskDelay(2000/portTICK_RATE_MS);
						LCD_WrtCmd(LCD_DISPLAY_CLEAR);
			break;
			case SD_LEER_REGISTRO:
					//UARTSend(LPC_UART0, Texto_SD_LEER_REGISTRO,sizeof(Texto_SD_LEER_REGISTRO)); //Para debug
					res = f_open(&Registro,"Registro.txt",FA_OPEN_EXISTING|FA_READ);//Para debug
					//hasta que no se alcanze el final del documento
					br = btr;
					while(br == btr)
					{
						res = f_read(&Registro, buffer,btr,&br);
						UARTSend(LPC_UART0, buffer,br);
						while(TxIntStat == SET);//Esperamos que termine la Tx
					}
					f_close(&Registro);//se alcanzo el final del documento
					estado_SD = SD_INACTIVO;
			break;
			case SD_ESCRIBIR_REGISTRO:
					//UARTSend(LPC_UART0, Texto_SD_ESCRIBIR_REGISTRO,sizeof(Texto_SD_ESCRIBIR_REGISTRO)); //Para debug
					res = f_open(&Registro,"Registro.txt", FA_OPEN_ALWAYS|FA_WRITE);//Abrimos o creamos registro en la SD
					if(res == FR_OK)	
					{
						RTC_GetFullTime(LPC_RTC, &RTC_Tiempo); //Levantamos el tiempo actual
						//movemos el cursor al final del documento (res = OK = 0)
						res = f_lseek(&Registro, Registro.fsize);
						//escribimos segun el formato propuesto "_dd/mm hh:mm ID:xxxxxxxxxx"
						f_putc('_',&Registro);
						f_putc(48 + (UNS_8)(RTC_Tiempo.DOM/10),&Registro);f_putc(48 + (UNS_8)(RTC_Tiempo.DOM%10),&Registro);
						f_putc('/',&Registro);
						f_putc(48 + (UNS_8)(RTC_Tiempo.MONTH/10),&Registro);f_putc(48 + (UNS_8)(RTC_Tiempo.MONTH%10),&Registro);
						f_putc(' ',&Registro);
						f_putc(48 + (UNS_8)(RTC_Tiempo.HOUR/10),&Registro);f_putc(48 + (UNS_8)(RTC_Tiempo.HOUR%10),&Registro);
						f_putc(':',&Registro);
						f_putc(48 + (UNS_8)(RTC_Tiempo.MIN/10),&Registro);f_putc(48 + (UNS_8)(RTC_Tiempo.MIN%10),&Registro);
						f_putc(' ',&Registro);f_putc('I',&Registro);f_putc('D',&Registro);f_putc(':',&Registro);
						f_write(&Registro,buffer+1,btw,&bw);
						f_close(&Registro);//cerramos el documento
					}else
					{
						LCD_WrtCmd(LCD_DISPLAY_CLEAR);
						LCD_WrtString(menu_SD_5); //No Encontrado
						LCD_GoToXY(0,1);
						LCD_WrtString(menu_SD_7); //Registro.txt
						vTaskDelay (2000 / portTICK_RATE_MS);	//retrasamos la tarea por 2s
						LCD_WrtCmd(LCD_DISPLAY_CLEAR);
					}
					estado_SD = SD_INACTIVO;
			break;
		}
	}
}

/*********************************************************************
 * @fn void vPUERTA (void *Parameters)
 * @brief		Abre y Cierra la puerta,desbloquea otras tareas y pasa a bloquearse
 * @param[in]	void 		 		
 * @return	void
 * Note: 
 **********************************************************************/
void vPUERTA (void * pvParameters)
{
	estado_PUERTA = PUERTA_CERRAR;
	//xSemaphoreTake( Semaforo_Puerta, portMAX_DELAY );
	while(1)
	{
		xSemaphoreTake( Semaforo_Puerta, portMAX_DELAY );
		//UARTSend(LPC_UART0, Texto_PUERTA,sizeof(Texto_PUERTA)); //Para debug
		switch(estado_PUERTA)
		{
			case PUERTA_ABRIR:
				//PIN de la puerta arriba
				LCD_WrtCmd(LCD_DISPLAY_CLEAR);
				LCD_WrtString(menu_PUERTA_ABRIR);
				vTaskDelay (3000 / portTICK_RATE_MS);
				//PIN de la Puerta abajo
				estado_PUERTA = PUERTA_CERRAR;
				estado_ID = ID_ESPERANDO; //cargo proximos estado
				xSemaphoreGive(Semaforo_ID);
			break;
			case PUERTA_CERRAR:
				vTaskDelay (1000 / portTICK_RATE_MS);		
			break;
		}		
	}
}
/*********************************************************************
 * @fn void vID (void *Parameters)
 * @brief	Inactivo se Bloquea; Esperando mira los intentos por ingresar,
 * pregunta si se recibio trama, detecta presencia de SD, Imprime en LCD el menu animado;
 * VERIFICANDO compara la trama con los ID de memoria; Bloqueado hace nada por 3s
 * @param[in]	void 		 		
 * @return	void
 * Note: 
 **********************************************************************/

 void vID (void * pvParameters)
{
	Bool aux;
	static volatile UNS_8 estado_MENU = 0,intentos = 0;
	static char buffer_hora[17];
	unsigned portBASE_TYPE mi_priori;
	estado_ID = ID_INACTIVO;
	//xSemaphoreTake( Semaforo_ID, portMAX_DELAY );
	while(1)
	{
		switch(estado_ID)
		{
			case ID_INACTIVO:				
						//UARTSend(LPC_UART0, Texto_ID_INACTIVO,sizeof(Texto_ID_INACTIVO)); //Para debug
						xSemaphoreTake( Semaforo_ID, portMAX_DELAY ); //Bloqueamos la Tarea
			break;
			case ID_ESPERANDO:	//cambiada en vUART,
						//UARTSend(LPC_UART0, Texto_ID_ESPERANDO,sizeof(Texto_ID_ESPERANDO)); //Para debug
						taskENTER_CRITICAL();
						if(intentos >= 3)
						{
							estado_ID = ID_BLOQUEADO;	//cambio al siguiente estado
							intentos = 0; 
						}		
							if((bc.rx_head-bc.rx_tail) >= (_BYTESdeTRAMA-1))//Se recibio trama?
						{
							UARTReceive(LPC_UART0, buffer, sizeof(buffer));	//cargamos los datos recibidos en el buffer
							estado_ID = ID_VERIFICANDO;
						}
						if( _card_insert() == 0 ) //Verificamos la presencia de la SD
						{
							if(Ra_Delay(10000))
							{
								LCD_WrtCmd(LCD_DISPLAY_CLEAR);
								//Pedimos al RTC la hora
								RTC_GetFullTime(LPC_RTC,&RTC_Tiempo);
								//Imprimimos la Fecha
								buffer_hora[0] = ' ';buffer_hora[15] = ' ';buffer_hora[16] = NULL;
								buffer_hora[1] = (RTC_Tiempo.DOM/10)+48;buffer_hora[2] = (RTC_Tiempo.DOM%10)+48;
								buffer_hora[3] = '/';
								buffer_hora[4] = (RTC_Tiempo.MONTH/10)+48;buffer_hora[5] = (RTC_Tiempo.MONTH%10)+48;
								buffer_hora[6] = '/';
								buffer_hora[7] = ((RTC_Tiempo.YEAR%100)/10)+48;buffer_hora[8] = (RTC_Tiempo.YEAR%10)+48;
								buffer_hora[9] = ' ';
								buffer_hora[10] = (RTC_Tiempo.HOUR/10)+48;buffer_hora[11] = (RTC_Tiempo.HOUR%10)+48;
								buffer_hora[12] = ':';
								buffer_hora[13] = (RTC_Tiempo.MIN/10)+48;buffer_hora[14] = (RTC_Tiempo.MIN%10)+48;
								
								LCD_WrtString(buffer_hora);
							
								LCD_GoToXY (0,1);
								switch(estado_MENU)	//Menu Animado para LCD
								{
									case 0:estado_MENU = 1;LCD_WrtString(menu_SD_9);break;
									case 1:estado_MENU = 2;LCD_WrtString(menu_SD_10);break;
									case 2:estado_MENU = 3;LCD_WrtString(menu_SD_11);break;
									case 3:estado_MENU = 4;LCD_WrtString(menu_SD_12);break;
									case 4:estado_MENU = 0;LCD_WrtString(menu_SD_13);break;
									default : estado_MENU = 0;break;
								}
						 }
						}else
						{
							estado_SETEO = SETEO_SD;//Volvemos a inicializar la SD
							estado_ID = ID_INACTIVO;	
							mi_priori = uxTaskPriorityGet(NULL);
							vTaskPrioritySet(Priori_ID, tskIDLE_PRIORITY);//Bajamos de Prioridad a la Tarea
							vTaskPrioritySet(Priori_SETEO, mi_priori);  //Subimos de Prioridad a la Tarea
							xSemaphoreGive(Semaforo_SETEO); //Habilitamos la tarea seteo
						}
						taskEXIT_CRITICAL();
			break;
			case ID_VERIFICANDO:
						//UARTSend(LPC_UART0, Texto_ID_VERIFICANDO,sizeof(Texto_ID_VERIFICANDO));//Para debug
						//!<Se compara el buffer con cada uno de los ID
						//taskENTER_CRITICAL();
						aux = FALSE;
						if(Ra_Comparar(ID_0,buffer,_BYTESdeDATO))aux = TRUE;
						if(Ra_Comparar(ID_1,buffer,_BYTESdeDATO))aux = TRUE;
						if(Ra_Comparar(ID_2,buffer,_BYTESdeDATO))aux = TRUE;
						if(Ra_Comparar(ID_3,buffer,_BYTESdeDATO))aux = TRUE;
						if(aux == TRUE) //Alguna tarjeta es valida
						{
							estado_ID = ID_INACTIVO;
							estado_SD = SD_ESCRIBIR_REGISTRO;//cambiamos al siguiente estado
							estado_PUERTA = PUERTA_ABRIR; 
							xSemaphoreGive(Semaforo_Puerta);//Habilitamos Dichas tareas
							xSemaphoreGive(Semaforo_SD);
						}else
						{
							intentos++;
							estado_ID = ID_ESPERANDO;	//volvemos al estado anterior
							LCD_WrtCmd(LCD_DISPLAY_CLEAR);
							LCD_WrtString(menu_SD_8); //Trama No Valida
							vTaskDelay (2000 / portTICK_RATE_MS);//Delay de 2s la Tarea
							LCD_WrtCmd(LCD_DISPLAY_CLEAR);
						}
						//taskEXIT_CRITICAL();
			break;
			case ID_BLOQUEADO:
						LPC_GPIO3->FIOCLR |= LED2; //Alarma ON
						//UARTSend(LPC_UART0, Texto_ID_BLOQUEADO,sizeof(Texto_ID_BLOQUEADO)); //Para debug
						vTaskDelay (3000 / portTICK_RATE_MS);//Delay de 500s la Tarea
						LPC_GPIO3->FIOSET |= LED2; //Alarma OFF
						estado_ID = ID_ESPERANDO;
			break;	
		}
		vTaskDelay (400 / portTICK_RATE_MS);//Delay de 500s la Tarea
	}
}
/*********************************************************************
 * @fn void vAplicationIdleHook(void)
 * @brief		en contruccion
 * @param[in]	 	void
 * @return	void
 * Note: Por el momento vacio
 **********************************************************************/

void vApplicationIdleHook(void)
{

}

/*-----------------------FIN DE TEXTO--------------------------------------*/
