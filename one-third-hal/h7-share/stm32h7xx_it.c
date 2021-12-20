#include "stm32h7xx_it.h"

__attribute__((weak)) void NMI_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void HardFault_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void MemManage_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void BusFault_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void UsageFault_Handler(void) {
    while (1) {
    }
}

__attribute__((weak)) void SVC_Handler(void) {}

__attribute__((weak)) void DebugMon_Handler(void) {}

__attribute__((weak)) void PendSV_Handler(void) {}

__attribute__((weak)) void SysTick_Handler(void) {
    // HAL_IncTick();
}
