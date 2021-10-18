/**
 * @file		:firm_uart.c
 * @brief		: Tipos de Datos y Funciones para el manejo de la UART
 * @version	: 1.0
 * @date		: 19/10/2013
 * @author	: by_Ra!
===============================================================================
*/
#ifndef __FIRM_UART_H__
#define __FIRM_UART_H__

#include "LPC17xx.h"

/*-------------------------Defines-----------------------------*/
/* buffer size definition */
#define UART_BUFSIZE 256

/* Buf mask */
#define __BUF_MASK (UART_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/*-Cantidad de bytes recibidos---*/
#define __BUF_LEN(head, tail) ((head&__BUF_MASK) - (tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx)	(bufidx=0)
#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)


/*-------------------Estructuras de Datos----------------------*/
/** @brief Buffer del UART */
typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[UART_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_BUFFER;

/*---------------------Prototipos de Funciones---------------------*/
void UART_IntErr(uint8_t);
void UART_IntTransmit(void);
void UART_IntReceive(void);
uint32_t UARTReceive(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint8_t buflen);
uint32_t UARTSend(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint8_t buflen);

#endif
