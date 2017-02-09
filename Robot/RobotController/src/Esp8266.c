/*******************************************************************************
  * @file Esp8266.c
  * @brief Implements functions for interfacing to ESP8266 WIFI chip
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "Esp8266.h"
#include "Uart.h"
#include "stm8s.h"
#include "stdio.h"
#include "string.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
const char OK_MSG[] = "OK\r\n";
const char READY_MSG[] = "ready\r\n";
const char TX_READY_MSG[] = "> ";
const char RX_PACKET_MSG[] = "+IPD,1,";


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
unsigned char packet[ESP8266_RX_BUFFER_SIZE] = {0};
int packetSize = 0;
unsigned char status = 0;


/*******************************************************************************
  * @brief Initialize the Esp8266 and any associated communications interfaces
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Esp8266_Initialize(void)
{ 
    status = 0;

    //Setup UART used for Esp8266 card communications
    Uart_Initialize(ESP8266_BAUD);
    Uart_SetRxCallback(Esp8266_ProcessRxByte);
    Uart_EnableRxInterrupt();
    
    //Make sure communications are working
    Esp8266_Validate();
    
    //Reset ESP8266
    Esp8266_Reset();
    
    //Stop ESP8266 from echoing all the commands we send it
    Esp8266_DisableEcho();
    
    //Clear the status word
    status = 0;
}

/*******************************************************************************
  * @brief Validate communications with the Esp8266 are functioning
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Esp8266_Validate()
{  
    char cmd[] = "AT\r\n";
    
    Uart_Send(cmd, sizeof(cmd)-1);
  
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
}

/*******************************************************************************
  * @brief Reset the Esp8266
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Esp8266_Reset()
{  
    char cmd[] = "AT+RST\r\n";
    
    Uart_Send(cmd, sizeof(cmd)-1);
    
    //wait for ready message
    WaitForResponse(ESP8266_READY_MESSAGE, TIMEOUT_LONG);
}

/*******************************************************************************
  * @brief Set the Esp8266 WIFI access point name
  * @par Parameters:
  * name - WIFI access point name
  * @retval None
  *****************************************************************************/
void Esp8266_SetAccessPointName(const char *name)
{
    long size = 0; 
    char buf[64] = {0};
    
    //Build set AP command and send it  
    size = sprintf(buf, "AT+CWSAP=\"%s\",\"\",5,0\r\n", name);  
    Uart_Send(buf, size);
    
    //wait for OK response message
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
}

/*******************************************************************************
  * @brief Disable the Esp8266 from echoing back all commands on the 
  *        communications interface
  * @par Parameters:
  * name - WIFI access point name
  * @retval None
  *****************************************************************************/
void Esp8266_DisableEcho()
{  
    char cmd[] = "ATE0\r\n";
    
    Uart_Send(cmd, sizeof(cmd)-1);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
}

/*******************************************************************************
  * @brief Start a client connection 
  * @par Parameters:
  * type - Connection type (TCP or UDP)
  * ip - IP address string
  * port - port number
  * @retval None
  *****************************************************************************/
void Esp8266_StartClient(const char *type, const char *ip, const unsigned short port)
{
    long size = 0; 
    char buf[64] = {0};
    
    //Set MUX for multi  
    size = sprintf(buf, "AT+CIPMUX=1\r\n");  
    Uart_Send(buf, size);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
    
    //Setup the socket  
    size = sprintf(buf, "AT+CIPSTART=1,\"%s\",\"%s\",%u,%u,0\r\n", type, ip, port, port);  
    Uart_Send(buf, size);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
}

/*******************************************************************************
  * @brief Start a TCP server
  * @par Parameters:
  * port - server port number
  * @retval None
  *****************************************************************************/
void Esp8266_StartTcpServer(const unsigned short port)
{
    long size = 0; 
    char buf[64] = {0};
    
    //Set MUX for multi  
    size = sprintf(buf, "AT+CIPMUX=1\r\n");  
    Uart_Send(buf, size);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
    
    //Setup TCP server socket  
    size = sprintf(buf, "AT+CIPSERVER=1,%u\r\n", port);  
    Uart_Send(buf, size);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
    
    //Set default server timeout
    Esp8266_SetTcpServerTimeout(ESP8266_SERVER_TIMEOUT);
}

/*******************************************************************************
  * @brief Set the TCP timeout
  * @par Parameters:
  * seconds - timeout in seconds
  * @retval None
  *****************************************************************************/
void Esp8266_SetTcpServerTimeout(const unsigned short seconds)
{
    long size = 0; 
    char buf[64] = {0};
        
    //Setup TCP server timeout  
    size = sprintf(buf, "AT+CIPSTO=%u\r\n", seconds);  
    Uart_Send(buf, size);
    
    //wait for OK
    WaitForResponse(ESP8266_OK_MESSAGE, TIMEOUT_SHORT);
}

/*******************************************************************************
  * @brief Get the remote client's IP address
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Esp8266_GetRemoteClientIp()
{
    //TODO
}

/*******************************************************************************
  * @brief Send a message 
  * @par Parameters:
  * buffer - data to send
  * length - data length in bytes
  * @retval None
  *****************************************************************************/
void Esp8266_SendMsg(unsigned char *buffer, unsigned short length)
{ 
    long size = 0; 
    char buf[32] = {0};
    
    //Send the command
    size = sprintf(buf, "AT+CIPSEND=1,%d\r\n", length);  
    Uart_Send(buf, size);
    
    //Wait for TX ready
    WaitForResponse(ESP8266_TX_READY_MESSAGE, TIMEOUT_SHORT);
        
    //Send the data
    Uart_Send(buffer, length);
    
    //Terminate command
    Uart_Send("\r\n", 2);
}

/*******************************************************************************
  * @brief Receive a message 
  * @par Parameters:
  * p - pointer to packet buffer
  * @retval number of bytes received
  *****************************************************************************/
int Esp8266_ReceiveMsg(unsigned char *p)
{
    //If the RX status bit is set
    if(status & ESP8266_RX_PACKET_MESSAGE)
    {
        //Clear the status bit, copy received data for caller 
        //and return number of bytes received
        status &= ~ESP8266_RX_PACKET_MESSAGE;
        memcpy(p, packet, packetSize);
        return packetSize;
    }
        
    return 0;
}

/*******************************************************************************
  * @brief State machine to process incoming bytes on the ESP8266 serial 
  *        interface
  * @par Parameters:
  * byte - byte received from ESP8266
  * @retval None
  *****************************************************************************/
void Esp8266_ProcessRxByte(unsigned char byte)
{
    static long esp_state = ESP8266_RESET;
    static long count     = 0;
    
    int size = 0;

    //State machine
    switch(esp_state)
    {
        ////////////////////////////////////////////
        //The initial state for the state machine
        case ESP8266_RESET:
        
            //Reset state variables
            count = 0;
            
            //Check for first byte in the desired message
            switch(byte)
            {        
                case (const)OK_MSG[0]:               
                    //Found first byte of OK message
                    esp_state = ESP8266_GET_OK;          
                    break;
                    
                case (const)READY_MSG[0]:
                    //Found first byte of READY message
                    esp_state = ESP8266_GET_READY;        
                    break;
                    
                case (const)RX_PACKET_MSG[0]:
                    //Found first byte of RX message
                    esp_state = ESP8266_GET_RX_HEADER;
                    break;
                
                case (const)TX_READY_MSG[0]:
                    //Found first byte of TX message
                    esp_state = ESP8266_GET_TX;
                    break;
            };
            count++;
            break;
        
        ////////////////////////////////////////////
        //Process ready message state
        case ESP8266_GET_READY:
            if(byte == READY_MSG[count])
            {    
                //Increment count and check if all bytes have been received
                if(++count >= sizeof(READY_MSG) - 1)
                {
                    //Got the entire ready message. 
                    //Reset state machine and set return value for ready message
                    esp_state = ESP8266_RESET;
                    status |= ESP8266_READY_MESSAGE;
                }
            }
            else
            {
                //Message did not match, reset state machine
                esp_state = ESP8266_RESET; 
            }
            break;
        
        ////////////////////////////////////////////
        //Process OK message state
        case ESP8266_GET_OK:
            if(byte == OK_MSG[count])
            {    
                //Increment count and check if all bytes have been received
                if(++count >= sizeof(OK_MSG) - 1)
                {
                    //Got the entire OK message. 
                    //Reset state machine and set return value for OK message
                    esp_state = ESP8266_RESET;
                    status |= ESP8266_OK_MESSAGE;
                }
            }
            else
            {
                //Message did not match, reset state machine
                esp_state = ESP8266_RESET; 
            }
            break;
        
        ////////////////////////////////////////////
        //Process RX header state
        case ESP8266_GET_RX_HEADER:
            if(byte == RX_PACKET_MSG[count])
            {    
                //Increment count and check if all bytes have been received
                if(++count >= sizeof(RX_PACKET_MSG) - 1)
                {
                    //Got the entire RX ready message header. 
                    //Next get length
                    esp_state = ESP8266_GET_RX_PACKET_SIZE;
                    count = 0;
                }
            }
            else
            {
                //Message did not match, reset state machine
                esp_state = ESP8266_RESET; 
            }
            break;
            
        ////////////////////////////////////////////
        //Process RX size state
        case ESP8266_GET_RX_PACKET_SIZE:
            //Keep reading length string until colon character is found
            if(byte != ':')
            {
                packet[count++] = byte;
            }
            else
            {        
                //Null terminate the lenght string
                packet[count] = 0;
                
                //Try to convert packet length string into a decimal number
                if(sscanf(packet, "%d", &size) == 1)
                {
                    //Record the packet size and clear packet buffer 
                    //for receiving packet data
                    packetSize = size;
                    esp_state = ESP8266_GET_RX_PACKET;
                    count = 0;
                }
                else
                {
                    //Error reading length, Reset state machine
                    esp_state = ESP8266_RESET;
                }
            }
            break;
        
        ////////////////////////////////////////////
        //Process RX packet data state
        case ESP8266_GET_RX_PACKET:
            packet[count++] = byte;
            if(count >= packetSize)
            {
                //Reset state machine and set return value for OK message
                esp_state = ESP8266_RESET;
                status |= ESP8266_RX_PACKET_MESSAGE;
            }
            break;
            
        ////////////////////////////////////////////
        //Process TX ready state
        case ESP8266_GET_TX:
            if(byte == TX_READY_MSG[count])
            {    
                //Increment count and check if all bytes have been received
                if(++count >= sizeof(TX_READY_MSG) - 1)
                {
                    //Got the entire TX Ready message. 
                    //Reset state machine and set return value for OK message
                    esp_state = ESP8266_RESET;
                    status |= ESP8266_TX_READY_MESSAGE;
                }
            }
            else
            {
                //Message did not match, reset state machine
                esp_state = ESP8266_RESET; 
            }
            break;
        
        ////////////////////////////////////////////
        //Catch invalid states and reset
        default:
            esp_state = ESP8266_RESET;
            break;
        
    };
}

/*******************************************************************************
  * @brief Poll for a response message 
  * @par Parameters:
  * msgType - message type to poll for
  * timeout - delay count
  * @retval None
  *****************************************************************************/
int WaitForResponse(unsigned char msgType, unsigned long timeout)
{  
    while(1/*timeout--*/)
    {
        //If the status bit for the desired message to be set
        if((status & msgType) == msgType)
        {
            //Clear the status bit and return success
            status &= ~msgType;
            return 1;
        }
    }
    
    return 0;
}