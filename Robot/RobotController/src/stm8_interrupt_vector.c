/**
  ******************************************************************************
  * @file STM8_interrupt_vector.c
  * @brief Basic interrupt vector table for STM8S devices
  * @author STMicroelectronics - MCD Application Team
  * @version V2.0.0
  * @date 15-March-2011
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2008 STMicroelectronics</center></h2>
  * @image html logo.bmp
  ******************************************************************************
  */

#include "STM8_TSL_API.h"
#include "STM8_TSL_timebase.h"
#include "Uart.h"

typedef void @far (*interrupt_handler_t)(void);

struct interrupt_vector
{
  unsigned char interrupt_instruction;
  interrupt_handler_t interrupt_handler;
};

@far @interrupt void Uart2RxInterrupt (void)
{
  Uart_ReceiveISR();
  return;
}

@far @interrupt void NonHandledInterrupt (void)
{
  /* in order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction
  */
  return;
}

extern void _stext();     /* startup routine */
void main(void);

struct interrupt_vector const _vectab[] =
  {
    {
      0x82, (interrupt_handler_t)_stext
    }
    , /* reset */
    //{0x82, (interrupt_handler_t)main}, /* reset */
    {0x82, NonHandledInterrupt}, /* trap */
    {0x82, NonHandledInterrupt}, /* irq0 - tli */
    {0x82, NonHandledInterrupt}, /* irq1 - awu */
    {0x82, NonHandledInterrupt}, /* irq2 - clk */
    {0x82, NonHandledInterrupt}, /* irq3 - exti0 */
    {0x82, NonHandledInterrupt}, /* irq4 - exti1 */
    {0x82, NonHandledInterrupt}, /* irq5 - exti2 */
    {0x82, NonHandledInterrupt}, /* irq6 - exti3 */
    {0x82, NonHandledInterrupt}, /* irq7 - exti4 */
    {0x82, NonHandledInterrupt}, /* irq8 - can rx */
    {0x82, NonHandledInterrupt}, /* irq9 - can tx */
    {0x82, NonHandledInterrupt}, /* irq10 - spi*/
    {0x82, NonHandledInterrupt}, /* irq11 - tim1 */
    {0x82, NonHandledInterrupt}, /* irq12 - tim1 */
    {0x82, NonHandledInterrupt}, /* irq13 - tim2 */
    {0x82, NonHandledInterrupt}, /* irq14 - tim2 */
    {0x82, NonHandledInterrupt}, /* irq15 - tim3 */
    {0x82, NonHandledInterrupt}, /* irq16 - tim3 */
    {0x82, NonHandledInterrupt}, /* irq17 - uart1 */
    {0x82, NonHandledInterrupt}, /* irq18 - uart1 */
    {0x82, NonHandledInterrupt}, /* irq19 - i2c */
    {0x82, NonHandledInterrupt}, /* irq20 - uart2/3 */
    //{0x82, NonHandledInterrupt},  /* irq21 - uart2/3 */
    {0x82, (interrupt_handler_t)Uart2RxInterrupt}, /* irq21 - uart2/3 */
    {0x82, NonHandledInterrupt}, /* irq22 - adc */
    {0x82, (interrupt_handler_t)TSL_Timer_ISR}, /* irq23 - tim4 */
    {0x82, NonHandledInterrupt}, /* irq24 - flash */
    {0x82, NonHandledInterrupt}, /* irq25 - reserved */
    {0x82, NonHandledInterrupt}, /* irq26 - reserved */
    {0x82, NonHandledInterrupt}, /* irq27 - reserved */
    {0x82, NonHandledInterrupt}, /* irq28 - reserved */
    {0x82, NonHandledInterrupt}  /* irq29 - reserved */
  };
