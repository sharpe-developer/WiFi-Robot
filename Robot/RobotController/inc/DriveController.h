/*******************************************************************************
  * @file DriveController.h
  * @brief Defines the functions for controlling the drive motors
  * @author David Sharpe
  * @version V1.0.0
  * @date 02-June-2015
  *
  * ST Visual Develop 4.2.1 using STM8 Cosmic C Compiler
  *****************************************************************************/
#ifndef DRIVE_CONTROLLER_H
#define DRIVE_CONTROLLER_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
enum Directions
{
    STOP,          
    FORWARD,       
    BACKWARD, 
    LEFT,
    SHARP_LEFT,    
    RIGHT,         
    SHARP_RIGHT        
};

#define SPEED_STOP    0
#define SPEED_FULL    100

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void DriveCtrl_Initialize(void);
void DriveCtrl_SetSpeed(unsigned char percentSpeed);
void DriveCtrl_Stop(void);
void DriveCtrl_Forward(void);
void DriveCtrl_Backward(void);
void DriveCtrl_Turn(unsigned char direction);

#endif