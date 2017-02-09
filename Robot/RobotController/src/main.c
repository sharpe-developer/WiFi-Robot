/*******************************************************************************
  * @file main.c
  * @brief This file contains the main function for the robot driver
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "DriveController.h"
#include "Esp8266.h"
#include "Uart.h"
#include "stm8s.h"
#include "stm8_tsl_api.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define TICK_TIMEOUT    1 //ms


/*******************************************************************************
  * @brief Configures STM8 clocks
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void CLK_Configuration(void)
{
    //Fmaster = 16MHz

    //Scale master clock prescaler 
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    //Set CPU clock prescaler
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);

    //Disable unused peripheral clocks to save power
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, DISABLE);    
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU, DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3, DISABLE); //For TSL
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);
    //CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2, DISABLE);
}

/*******************************************************************************
  * @brief Configures timer used for timing/scheduling
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Init_Timer(void)
{
    //Number of periods that pass before update event occurs
    uint32_t timeout = TICK_TIMEOUT;

    //Set prescaler for master clock
    //Note final prescaler register value is prescaler + 1
    uint16_t prescaler = 1000 - 1;

    //Set base period for 1ms
    uint16_t period = CLK_GetClockFreq() / 1000000 - 1;

    //Timer 1 Peripheral Configuration
    TIM1_DeInit();

    //Init timer 1 registers  
    TIM1_TimeBaseInit(prescaler, TIM1_COUNTERMODE_UP, period, timeout - 1);

    //Enables timer peripheral Preload register on ARR
    TIM1_ARRPreloadConfig(ENABLE);

    //Enable timer
    TIM1_Cmd(ENABLE);
}

/*******************************************************************************
  * @brief Configures LED GPIO
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void InitLED(void)
{
    //Reset GPIO port A, D, and G
    GPIO_DeInit(GPIOD);
  
    //Configure PD0 (LED1) as output push-pull low (led switched on)
    GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
}

/*******************************************************************************
  * @brief Toggle PD0 (Led LD1)
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void ToggleLED(void)
{
    GPIO_WriteReverse(GPIOD, GPIO_PIN_0);
}

/*******************************************************************************
  * @brief Initialize the capacitive touch button
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void TouchSensePadInit(void)
{
    u8 i;

    //Initialize Touch Sensing library
    TSL_Init();
    
    //All keys are implemented and enabled

    for (i = 0; i < NUMBER_OF_SINGLE_CHANNEL_KEYS; i++)
    {
        sSCKeyInfo[i].Setting.b.IMPLEMENTED = 1;
        sSCKeyInfo[i].Setting.b.ENABLED = 1;
        sSCKeyInfo[i].DxSGroup = 0x01; //Put 0x00 to disable the DES on these pins
    }

#if NUMBER_OF_MULTI_CHANNEL_KEYS > 0
    for (i = 0; i < NUMBER_OF_MULTI_CHANNEL_KEYS; i++)
    {
        sMCKeyInfo[i].Setting.b.IMPLEMENTED = 1;
        sMCKeyInfo[i].Setting.b.ENABLED = 1;
        sMCKeyInfo[i].DxSGroup = 0x01; //Put 0x00 to disable the DES on these pins
    }
#endif
    
    //Start the 100ms timebase Timer
    TSL_Tick_Flags.b.User1_Start_100ms = 1;
}

/*******************************************************************************
  * @brief Check if touch sense pad has been touched
  * @par Parameters: None
  * @retval 1 if touched, 0 otherwise
  *****************************************************************************/
int IsTouchSensePressed(void)
{
    if ((TSL_GlobalSetting.b.CHANGED) && (TSLState == TSL_IDLE_STATE))
    {
        TSL_GlobalSetting.b.CHANGED = 0;

        // If KEY 1 touched
        if (sSCKeyInfo[0].Setting.b.DETECTED) 
        {
            return 1;
        }
    }

    return 0; 
}

/*******************************************************************************
  * @brief Check timer status
  * @par Parameters: None
  * @retval 1 if timer update event is set, 0 otherwise
  *****************************************************************************/
int IsTimerExpired(void)
{
    if (TIM1_GetFlagStatus(TIM1_FLAG_UPDATE) == SET)
    {
      TIM1_ClearFlag(TIM1_FLAG_UPDATE);
      return 1;
    }
    
    return 0;
}

/*******************************************************************************
  * @brief Initialize the system
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void Initialize(void)
{
    int i = 1000;
    
    //Configures clocks
    CLK_Configuration();

    //Configures LED GPIO
    InitLED();
    
    //Initialize the timer for process timing
    Init_Timer();
    
    //Initialize Touch Sensing button
    TouchSensePadInit();
    
    //Initialize the motor drive controller
    DriveCtrl_Initialize();
    
    while(i)
    {  
        if(IsTimerExpired())
        {
            i--;          
        }
    }
  
    enableInterrupts();
    
    //Initialize WiFi interface
    Esp8266_Initialize();
    
    //Set the access point name
    Esp8266_SetAccessPointName("STM8S_Robot");
    
    //Set up a UDP socket
    Esp8266_StartClient(ESP8266_UDP, "192.168.4.2", 49999);
    
    //Start TCP server
    //Esp8266_StartTcpServer(49999);
}

/*******************************************************************************
  * @brief main entry point
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void main(void)
{
    int led_count = 0;

    unsigned char packet[ESP8266_RX_BUFFER_SIZE] = {0};
    unsigned short length = 0;
    
    //Initialize the system
    Initialize();
    
    //Set initial state to stopped
    DriveCtrl_SetSpeed(0);
    DriveCtrl_Stop();
    
    // Main loop
    while (1)
    {    
        if (IsTimerExpired())
        {     
            if(++led_count >= 250)
            {
                ToggleLED();
                led_count = 0;          
            }
        }     

        //Check for received Wifi packets
        if(length = Esp8266_ReceiveMsg(packet))
        {
            //Process the message received from the controller. 
            switch(packet[0])
            {
                case STOP:
                    DriveCtrl_Stop();
                    DriveCtrl_SetSpeed(0);
                    break;
                case FORWARD:
                    DriveCtrl_Forward();
                    DriveCtrl_SetSpeed(packet[1]);
                    break;
                case BACKWARD:
                    DriveCtrl_Backward();
                    DriveCtrl_SetSpeed(packet[1]);
                    break;
                case LEFT:
                    DriveCtrl_Turn(LEFT);
                    DriveCtrl_SetSpeed(packet[1]);
                    break;
                case RIGHT:
                    DriveCtrl_Turn(RIGHT);
                    DriveCtrl_SetSpeed(packet[1]);
                    break;
            };
            
            //Echo packet
            Esp8266_SendMsg(packet, length);
        }
  
        //Main function of the Touch Sensing library
        TSL_Action();
        
        //Has touch sense button been touched
        if(IsTouchSensePressed())
        {       
            Esp8266_SendMsg("Hello", 5);
        }
    }
}

