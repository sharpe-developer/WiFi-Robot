/*******************************************************************************
  * @file Esp8266.h
  * @brief Defines the functions for interfacing to ESP8266 WIFI chip
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/
#ifndef ESP_8266_H
#define ESP_8266_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define ESP8266_BAUD            115200

#define ESP8266_RX_BUFFER_SIZE  64

#define ESP8266_SERVER_TIMEOUT  300 //seconds

#define ESP8266_UDP             "UDP"
#define ESP8266_TCP             "TCP"

#define TIMEOUT_LONG            0xFFFFFFFF
#define TIMEOUT_SHORT           0x00FFFFFF


enum RxState
{
    ESP8266_RESET,
    ESP8266_GET_READY,
    ESP8266_GET_OK,
    ESP8266_GET_RX_HEADER,
    ESP8266_GET_RX_PACKET_SIZE,
    ESP8266_GET_RX_PACKET,
    ESP8266_GET_TX
};

//Status word bits
#define ESP8266_OK_MESSAGE        0x01
#define ESP8266_READY_MESSAGE     0x02
#define ESP8266_TX_READY_MESSAGE  0x04
#define ESP8266_RX_PACKET_MESSAGE 0x08


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void Esp8266_Initialize(void);
void Esp8266_Validate(void);
void Esp8266_Reset(void);
void Esp8266_SetAccessPointName(const char *name);
void Esp8266_DisableEcho(void);
void Esp8266_StartClient(const char *type, const char *ip, 
                         const unsigned short port);
void Esp8266_StartTcpServer(const unsigned short port);
void Esp8266_SetTcpServerTimeout(const unsigned short seconds);
void Esp8266_GetRemoteClientIp();
void Esp8266_SendMsg(unsigned char *buffer, unsigned short length);
int  Esp8266_ReceiveMsg(unsigned char *packet);
void Esp8266_ProcessRxByte(unsigned char byte);
int  WaitForResponse(unsigned char msgType, unsigned long timeout);

#endif