/*********************************************************************************************************//**
 * @file    Mono_LCD/Demo/ht32f5xxxx_01_it.c
 * @version $Rev:: 4459         $
 * @date    $Date:: 2020-01-06 #$
 * @brief   This file provides all interrupt service routine.
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

/** @addtogroup HT32_Series_Peripheral_Examples HT32 Peripheral Examples
  * @{
  */

/** @addtogroup Mono_LCD_Examples Mono_LCD
  * @{
  */

/** @addtogroup Demo
  * @{
  */


/* Global functions ----------------------------------------------------------------------------------------*/
/*********************************************************************************************************//**
 * @brief   This function handles NMI exception.
 * @retval  None
 ************************************************************************************************************/
void NMI_Handler(void)
{
}

/*********************************************************************************************************//**
 * @brief   This function handles Hard Fault exception.
 * @retval  None
 ************************************************************************************************************/
void HardFault_Handler(void)
{
  while (1);
}
/*********************************************************************************************************//**
 * @brief   This function handles LCD Handler.
 * @retval  None
 ************************************************************************************************************/
void LCD_IRQHandler(void)
{
  extern vu32 isUpdateDisplayFinish;
  if(LCD_GetFlagStatus(LCD_FLAG_UDD))
  {
    /* Update Display Done */
    isUpdateDisplayFinish = TRUE;
  }
  /* Clear Update Display Flag */
  LCD_ClearFlag(LCD_CLR_UDD);
  /* Clear Start of Frame Flag */
  LCD_ClearFlag(LCD_CLR_SOF);
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
