/*******************************************************************************
  * @file Uart.h
  * @brief Defines the functions for interfacing to the STMS8 UART
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/
#ifndef UART_H
#define UART_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define UART_BUFFER_SIZE  64

typedef void(*Callback)(unsigned char);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void Uart_Initialize(unsigned long baud);
void Uart_Send(unsigned char *buffer, unsigned long length);
void Uart_SendByte(unsigned char byte);
void Uart_ReceiveISR(void);
int  Uart_IsRxDataReady(void);
unsigned char Uart_GetRxData(void);
void Uart_ClearRxFifo(void);
void Uart_EnableRxInterrupt(void);
void Uart_SetRxCallback(Callback func);
void Uart_Print(char *str);

#endif