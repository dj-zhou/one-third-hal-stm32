#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__( ( weak ) ) void NMI_Handler( void );
__attribute__( ( weak ) ) void HardFault_Handler( void );
__attribute__( ( weak ) ) void MemManage_Handler( void );
__attribute__( ( weak ) ) void BusFault_Handler( void );
__attribute__( ( weak ) ) void UsageFault_Handler( void );
__attribute__( ( weak ) ) void SVC_Handler( void );
__attribute__( ( weak ) ) void DebugMon_Handler( void );
__attribute__( ( weak ) ) void PendSV_Handler( void );
__attribute__( ( weak ) ) void SysTick_Handler( void );

#ifdef __cplusplus
}
#endif

#endif  // __STM32F1xx_IT_H
