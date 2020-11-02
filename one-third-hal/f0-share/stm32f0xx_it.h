/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f0xx_it.h
 * @brief   This file contains the headers of the interrupt handlers.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F0xx_IT_H
#define __STM32F0xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__( ( weak ) ) void NMI_Handler( void );
__attribute__( ( weak ) ) void HardFault_Handler( void );
__attribute__( ( weak ) ) void SVC_Handler( void );
__attribute__( ( weak ) ) void PendSV_Handler( void );
__attribute__( ( weak ) ) void SysTick_Handler( void );

#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_IT_H */
