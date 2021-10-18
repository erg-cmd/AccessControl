/**
 * @file		:firm_uart.c
 * @brief		: Tipos de Datos y Funciones para el manejo de la UART
 * @version	: 1.0
 * @date		: 19/10/2013
 * @author	: by_Ra!
===============================================================================
*/

#include "firm_uart.h"
#include "lpc17xx_uart.h"

/*--------------Declaracion de variables locales---------------*/

// Buffer Circular p/ UART
 volatile UART_BUFFER bc;
// Estado actual de la interrupcion por transmision
volatile FlagStatus TxIntStat;

/*-------------Funciones para la UART---------------------------*/

/*********************************************************************//**
 * @brief	UART0 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
void UART0_IRQHandler(void)
{
	// Call Standard UART 0 interrupt handler
	UART0_StdIntHandler();
}

/********************************************************************
 * @brief 		Recepcion usando el buffer ciclico, se considera al buffer
 * lo bastante grande pero en caso de buffer lleno no se escribe y se deshecha
 * el dato. Para FIFO disparado de a 1 byte de recepcion
 * @param[in]	None
 * @return 		None
 * Note: Supuestamente leyendo el RHB se borra el flag de interrupcion
 *********************************************************************/
void UART_IntReceive(void)
{
	UNS_8 aux;
	aux = LPC_UART0->RBR& UART_RBR_MASKBIT;
	//Preguntamos si el buffer NO esta lleno
	if(!__BUF_IS_FULL(bc.rx[bc.rx_head],bc.rx[bc.rx_tail]))
	{
		//guardamos el dato en el buffer
		bc.rx[bc.rx_head] = aux;
		//incrementamos el indice de buff de recepcion
		__BUF_INCR(bc.rx_head);
	}
}

/********************************************************************//**
 * @brief 		IRQ por Tx, habilitacion o  continuacion (1 dato a mano)
 * @param[in]	None
 * @return 		None
 * Note : supuestamente el flag de la IRQ se baja por 
 *********************************************************************/
void UART_IntTransmit(void)
{
    // Deshabilito la interrupcion por transmision
    UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, DISABLE);

		//esperamos a que termine de transmitir
    while (UART_CheckBusy(LPC_UART0) == SET);
		//Si el buffer NO esta vacio cargamos el próximo dato
		if(!__BUF_IS_EMPTY(bc.tx_head,bc.tx_tail))
    {
      //Cargamos en dato en FIFO de salida	
			LPC_UART0->THR = bc.tx[bc.tx_tail] & UART_THR_MASKBIT;
			//Incrementamos la cola de nuestro buffer de transmision
      __BUF_INCR(bc.tx_tail);
    } 
    //si el buffer esta vacio queda deshabilitada la IRQ por Tx
		if (__BUF_IS_EMPTY(bc.tx_head, bc.tx_tail)) 
		{
			// Estado de la Tx por interrupcion OFF
    	TxIntStat = RESET;
    }
    else
		{
      // Estado de la Tx por interrupcion ON
			TxIntStat = SET;
			//activamos la interrupcion por Tx
    	UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);
    }
	
}

/*********************************************************************//**
 * @brief		UART Line Status Error callback
 * @param[in]	bLSErrType	UART Line Status Error Type
 * @return		None
 **********************************************************************/
void UART_IntErr(uint8_t bLSErrType)
{
	uint8_t test;
	// Loop forever
	while (1){
		// For testing purpose
		test = bLSErrType;
	}
}


/*********************************************************************//**
 * @brief		UART transmit function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	txbuf Pointer to Transmit buffer
 * @param[in]	buflen Length of Transmit buffer
 * @return 		Number of bytes actually sent to the ring buffer
 **********************************************************************/
#pragma O0
uint32_t UARTSend(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint8_t buflen)
{
    uint8_t *data = (uint8_t *) txbuf;
    uint32_t bytes = 0;
	//	static UNS_32 aux_head,aux_tail;
	//	aux_head = bc.tx_head;aux_tail = bc.tx_tail;
	/* Temporarily lock out UART transmit interrupts during this
	   read so the UART transmit interrupt won't cause problems
	   with the index values */
    UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	   expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(bc.tx_head,bc.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		bc.tx[bc.tx_head] = *data;
		data++;

		/* Increment head pointer */
//		aux_head++;
		__BUF_INCR(bc.tx_head);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}
	//bc.tx_head = aux_head;bc.tx_tail = aux_tail;
	/*
	 * Check if current Tx interrupt enable is reset,
	 * that means the Tx interrupt must be re-enabled
	 * due to call UART_IntTransmit() function to trigger
	 * this interrupt type
	 */
	if (TxIntStat == RESET) {
		UART_IntTransmit();
	}
	/*
	 * Otherwise, re-enables Tx Interrupt
	 */
	else {
		UART_IntConfig(UARTPort, UART_INTCFG_THRE, ENABLE);
	}

    return bytes;
}
#pragma O2
/*********************************************************************//**
 * @brief		UART read function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	rxbuf Pointer to Received buffer
 * @param[in]	buflen Length of Received buffer
 * @return 		Number of bytes actually read from the ring buffer
 **********************************************************************/
uint32_t UARTReceive(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint8_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	   read so the UART receive interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
		until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(bc.rx_head, bc.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = bc.rx[bc.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(bc.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

    return bytes;
}

