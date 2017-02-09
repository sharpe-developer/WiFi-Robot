/*******************************************************************************
  * @file Uart.c
  * @brief Implements the functions for interfacing with the STM8S UART
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "Uart.h"
#include "stm8s.h"
#include "string.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
int Uart_FifoEnqueue(unsigned char data);
unsigned char Uart_FifoDequeue(void);
bool Uart_FifoIsEmpty(void);
bool Uart_FifoIsFull(void);
void Uart_FifoClear(void);
unsigned long Uart_FifoGetNextIndex(unsigned long index);


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
unsigned char fifo[UART_BUFFER_SIZE] = {0};
unsigned long enqueueIndex = 0;
unsigned long dequeueIndex = 0;
Callback rxCallback = 0;


/*******************************************************************************
  * @brief Insert data into the FIFO
  * @par Parameters: 
  * data - byte of data to insert
  * @retval 1 is inserted succesfully, 0 otherwise
  *****************************************************************************/
int Uart_FifoEnqueue(unsigned char data)
{
    //Make sure we have allocated a memory buffer  
    //to store the data in and that it is not full
    if(!Uart_FifoIsFull())
    {
        //Insert a byte into the queue
        fifo[enqueueIndex] = data;

        //Circularly increment the enqueue index
        enqueueIndex = Uart_FifoGetNextIndex(enqueueIndex);
            
        return 1;
    }

    return 0;	
}

/*******************************************************************************
  * @brief Remove an element from the FIFO
  * @par Parameters: 
  * None
  * @retval The dequeued element
  *****************************************************************************/
unsigned char Uart_FifoDequeue(void)
{
    unsigned char data = 0;

    //Make sure we have allocated a memory buffer to 
    //retrieve the data from and that the queue is not empty
    if(!Uart_FifoIsEmpty())
    {
        //Remove a byte from the queue 
        data = fifo[dequeueIndex];

        //Circularly increment the dequeue index 
        dequeueIndex = Uart_FifoGetNextIndex(dequeueIndex);
    }

    return data;		
}

/*******************************************************************************
  * @brief Checks if the FIFO is empty
  * @par Parameters: 
  * None
  * @retval true if FIFO is empty
  *****************************************************************************/
bool Uart_FifoIsEmpty(void)
{
    //If read index equals the write index, then FIFO is empty
    return (dequeueIndex == enqueueIndex);
}

/*******************************************************************************
  * @brief Checks if the FIFO is full
  * @par Parameters: 
  * None
  * @retval true if FIFO is full
  *****************************************************************************/
bool Uart_FifoIsFull(void)
{
    //Compute the next index for a write operation
    //If the next write index equals the read 
    //index then the FIFO is considered full
    return (Uart_FifoGetNextIndex(enqueueIndex) == dequeueIndex);		
}

/*******************************************************************************
  * @brief Clear/Reset the FIFO
  * @par Parameters: 
  * None
  * @retval None
  *****************************************************************************/
void Uart_FifoClear(void)
{
    unsigned long i = 0;

    //Set the FIFO buffer to all zeros
    for(i=0; i<UART_BUFFER_SIZE; ++i)
    {
        fifo[i] = 0;
    }

    //Reset insert and remove indexes
    dequeueIndex = 0;
    enqueueIndex = 0;
}

/*******************************************************************************
  * @brief Compute the next buffer index
  * @par Parameters: 
  * index - Current buffer index value
  * @retval Updated index
  *****************************************************************************/
unsigned long Uart_FifoGetNextIndex(unsigned long index)
{
    unsigned long next_index = index + 1;
	
    //Wrap the index value if it has gone beyond the end of the buffer
    if(next_index >= UART_BUFFER_SIZE)
    {
        next_index = 0;
    }
	
    return next_index;
}
  
    
/*******************************************************************************
  * @brief Initialize the UART
  * @par Parameters:
  * None
  * @retval None
  *****************************************************************************/
void Uart_Initialize(unsigned long baud)
{
    //Setup UART
    UART2_DeInit();
    
    UART2_Init(baud,                          
               UART2_WORDLENGTH_8D,
               UART2_STOPBITS_1,
               UART2_PARITY_NO,
               UART2_SYNCMODE_CLOCK_DISABLE,  //no sync
               UART2_MODE_TXRX_ENABLE);       //TX and RX enabled 

    UART2_Cmd(ENABLE);
}

/*******************************************************************************
  * @brief Send data using the UART
  * @par Parameters:
  * buffer - the data buffer
  * length - number of bytes to send
  * @retval None
  *****************************************************************************/
void Uart_Send(unsigned char *buffer, unsigned long length)
{  
    int i = 0;
    
    for(i = 0; i < length; ++i)
    {
        //Transmit data using UART
        UART2_SendData8(buffer[i]);
        
        while(UART2_GetFlagStatus(UART2_FLAG_TXE) != SET)
        {;}
    }
}

/*******************************************************************************
  * @brief Send a single byte using the UART
  * @par Parameters:
  * byte - the byte to send
  * @retval None
  *****************************************************************************/
void Uart_SendByte(unsigned char byte)
{  
    //Transmit data using UART
    UART2_SendData8(byte);
        
    while(UART2_GetFlagStatus(UART2_FLAG_TXE) != SET)
    {;}
}

/*******************************************************************************
  * @brief Interrupt service routine invoked when received data is ready 
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Uart_ReceiveISR()
{
    unsigned char byte = 0;
    
    //UART2_ClearITPendingBit(UART2_IT_RXNE);
    
    byte = UART2_ReceiveData8();
    
    if(rxCallback)
    {
        rxCallback(byte);
    }
    
    //Uart_FifoEnqueue(byte);
}

/*******************************************************************************
  * @brief Checks if receive data is available in the FIFO
  * @par Parameters: None
  * @retval 1 if data is ready, 0 otherwise
  *****************************************************************************/
int Uart_IsRxDataReady(void)
{
    //If there is data in the RX FIFO
    if(!Uart_FifoIsEmpty())
    {
        return 1;
    }
    
    return 0;
}

/*******************************************************************************
  * @brief Get a byte from the receive FIFO
  * @par Parameters: None
  * @retval byte from FIFO
  *****************************************************************************/
unsigned char Uart_GetRxData(void)
{ 
    return Uart_FifoDequeue();
}

/*******************************************************************************
  * @brief Clear all data from the receive FIFO
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Uart_ClearRxFifo(void)
{ 
    Uart_FifoClear();
}

/*******************************************************************************
  * @brief Enable UART receive interrupt
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Uart_EnableRxInterrupt(void)
{
    UART2_ITConfig(UART2_IT_RXNE, ENABLE);
}

/*******************************************************************************
  * @brief Set a callback to be invoked when UART receive interrupt occurs
  * @par Parameters: callback function pointer
  * @retval None
  *****************************************************************************/
void Uart_SetRxCallback(Callback func)
{
    rxCallback = func;
}

/*******************************************************************************
  * @brief Send a string using the UART
  * @par Parameters: 
  * str - null terminated string to send
  * @retval None
  *****************************************************************************/
void Uart_Print(char *str)
{
    Uart_Send(str, strlen(str));
}