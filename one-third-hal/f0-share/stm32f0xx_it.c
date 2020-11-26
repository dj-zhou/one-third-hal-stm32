#include "stm32f0xx_it.h"
#include "config.h"

__attribute__((weak)) void NMI_Handler(void) {}

__attribute__((weak)) void HardFault_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void SVC_Handler(void) {}

__attribute__((weak)) void PendSV_Handler(void) {}

__attribute__((weak)) void SysTick_Handler(void) {
    HAL_IncTick();
}
