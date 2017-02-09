/*******************************************************************************
  * @file DriveController.c
  * @brief Implements the functions for controlling the drive motors
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
#include "stm8s.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define PWM_TIMER_MAX_COUNT 1000


/*******************************************************************************
  * @brief Setup timer 2 (TIM2) for two PWM outputs on TIM2 channels 1 and 2
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void InitMotorPwmTimer(void)
{
    unsigned short startingDutyCycle = 0;
    
    //TIM2 Peripheral Configuration
    TIM2_DeInit();

    //Set TIM2 Frequency to 2Mhz
    TIM2_TimeBaseInit(TIM2_PRESCALER_1, PWM_TIMER_MAX_COUNT - 1);
    
    //Channel 1 PWM configuration
    TIM2_OC1Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, startingDutyCycle, TIM2_OCPOLARITY_LOW ); 
    TIM2_OC1PreloadConfig(ENABLE);
    
    //Channel 2 PWM configuration
    TIM2_OC2Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, startingDutyCycle, TIM2_OCPOLARITY_LOW );
    TIM2_OC2PreloadConfig(ENABLE);
    
    //Enables TIM2 peripheral Preload register on ARR
    TIM2_ARRPreloadConfig(ENABLE);
        
    //Enable TIM2
    TIM2_Cmd(ENABLE);
}

/*******************************************************************************
  * @brief Configures GPIOs used for motor control
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void InitMotorGpio(void)
{
    //Reset GPIO port A and G
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOG);
    
    //Configure PA3 and PA4 as output push-pull low
    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOA, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST);
    
    //Configure PG0 and PG1 as output push-pull low
    GPIO_Init(GPIOG, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOG, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST);
}

/*******************************************************************************
  * @brief Set a specific motor to turn in the specified direction or stop
  * @par Parameters:
  * motor - the motor ID
  * direction - the desired direction of travel
  * @retval None
  *****************************************************************************/
void Motor(unsigned char motor, unsigned char direction)
{
    GPIO_TypeDef *port;
    unsigned char pin1, pin2;
    
    //Get the port and pins for the desired motor
    switch(motor)
    {
        case LEFT:
            port = GPIOA;
            pin1 = GPIO_PIN_3;
            pin2 = GPIO_PIN_4;
            break;
        
        case RIGHT:
            port = GPIOG;
            pin1 = GPIO_PIN_0;
            pin2 = GPIO_PIN_1;
            break;
        
        default:
            //Invalid motor, do nothing
            return;
            break;
    };
    
    //Set the motor inputs for the desired direction of travel
    switch(direction)
    {
        case STOP:
            GPIO_WriteLow(port, pin1);
            GPIO_WriteLow(port, pin2);
            break;
        
        case FORWARD:
            GPIO_WriteHigh(port, pin1);
            GPIO_WriteLow(port, pin2);
            break;
            
        case BACKWARD:
            GPIO_WriteLow(port, pin1);
            GPIO_WriteHigh(port, pin2);
            break;
        
        default:
            break;
    };
}

/*******************************************************************************
  * @brief Initialize the motor drive controller
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void DriveCtrl_Initialize()
{
    //Configures motor GPIOs
    InitMotorGpio();
    
    //Setup the motor PWM timer
    InitMotorPwmTimer();  
}

/*******************************************************************************
  * @brief Set the motor PWM for the desired speed
  * @par Parameters:
  * percentSpeed - motor speed percentage (0 to 100)
  * @retval None
  *****************************************************************************/
void DriveCtrl_SetSpeed(unsigned char percentSpeed)
{
    const float SCALE_FACTOR = PWM_TIMER_MAX_COUNT / 100;
    
    //Do not allow speed greater than 100%
    if(percentSpeed > 100)
    {
        percentSpeed = 100;
    }
    
    //Set PWM for each motor
    TIM2_SetCompare1((SCALE_FACTOR * percentSpeed) + 0.5);
    TIM2_SetCompare2((SCALE_FACTOR * percentSpeed) + 0.5);
}

/*******************************************************************************
  * @brief Set the motors to stop movement
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void DriveCtrl_Stop()
{
    Motor(LEFT, STOP);
    Motor(RIGHT, STOP);
}

/*******************************************************************************
  * @brief Set the motors for forward movement
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void DriveCtrl_Forward()
{
    Motor(LEFT, FORWARD);
    Motor(RIGHT, FORWARD);
}

/*******************************************************************************
  * @brief Set the motors for backward movement
  * @par Parameters: None
  * @retval None
  *****************************************************************************/
void DriveCtrl_Backward()
{
    Motor(LEFT, BACKWARD);
    Motor(RIGHT, BACKWARD);
}

/*******************************************************************************
  * @brief Set the motors to turn the vehicle
  * @par Parameters:
  * direction - the direction to turn
  * @retval None
  *****************************************************************************/
void DriveCtrl_Turn(unsigned char direction)
{
    switch(direction)
    {
        case LEFT:
            Motor(LEFT, STOP);
            Motor(RIGHT, FORWARD);
            break;
            
        case SHARP_LEFT:
            Motor(LEFT, BACKWARD);
            Motor(RIGHT, FORWARD);
            break;
            
        case RIGHT:
            Motor(LEFT, FORWARD);
            Motor(RIGHT, STOP);
            break;        
            
        case SHARP_RIGHT:
            Motor(LEFT, FORWARD);
            Motor(RIGHT, BACKWARD);
            break;
    };
}



