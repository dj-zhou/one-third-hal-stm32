#include "general-utils.h"

// ============================================================================
#if defined(RTOS_IS_USED)
static RtosState_e rtos_state_;
#endif

// ============================================================================
// clang-format off
#if defined(_LED_HEARTBEAT_PORT)
    #define ERROR_LED_PORT _LED_HEARTBEAT_PORT
#else
    #define ERROR_LED_PORT GPIOA  // not sure if this will conflict with something
#endif
#if defined(_LED_HEARTBEAT_PIN)
    #define ERROR_LED_PIN _LED_HEARTBEAT_PIN
#else
    #define ERROR_LED_PIN 5  // not sure if this will conflict with something
#endif
// clang-format on

static uint32_t hclk_;
static uint32_t pclk1_;
static uint32_t pclk2_;

// ============================================================================
static void Error_Handler(uint32_t hclk) {
    int step;
    if (hclk < 10000000) {
        step = 10;
    }
    else if (hclk < 20000000) {
        step = 20;
    }
    else if (hclk < 50000000) {
        step = 50;
    }
    else if (hclk < 100000000) {
        step = 100;
    }
    else if (hclk < 200000000) {
        step = 2000;
    }
    else if (hclk < 300000000) {
        step = 5000;
    }
    else {
        step = 10000;
    }

    utils.pin.mode(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_MODE_OUTPUT_PP);
    while (1) {
        for (int i = 0; i < step; i++) {
            for (int j = 0; j < 500; j++) {
                ;
            }
        }
        utils.pin.toggle(ERROR_LED_PORT, ERROR_LED_PIN);
    }
}

// ============================================================================
static HAL_StatusTypeDef VerifyClocks(uint32_t hclk, uint32_t pclk1,
                                      uint32_t pclk2) {
    hclk_ = HAL_RCC_GetHCLKFreq();
    pclk1_ = HAL_RCC_GetPCLK1Freq();
#if !defined(STM32F030x8)
    pclk2_ = HAL_RCC_GetPCLK2Freq();
#else
    pclk2_ = 0;
#endif
    // no addclk?
    if ((hclk_ != hclk) || (pclk1_ != pclk1) || (pclk2_ != pclk2)) {
        Error_Handler(hclk);
    }
    return HAL_OK;
}

// ============================================================================
#if defined(STM32F030x8)
static HAL_StatusTypeDef InitClock_F030x8(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 48) || (pclk1_m != 48) || (pclk2_m != 0)) {
        Error_Handler(hclk_m * 5000000);
    }
    if (HSE_VALUE == 8000000) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
        RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)
            != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }
    }
    else {
        // #error not supported.
    }
    return VerifyClocks(48000000, 48000000, 0);  // no PCLK2
}
#endif

// ============================================================================
#if defined(STM32F103xB)
static HAL_StatusTypeDef InitClock_F103xB(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 72) || (pclk1_m != 36) || (pclk2_m != 72)) {
        Error_Handler(hclk_m * 5000000);
    }
    if (HSE_VALUE == 8000000) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
            != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }
    }
    else {
        // #error not supported.
    }
    return VerifyClocks(72000000, 36000000, 72000000);
}
#endif  // STM32F103xB

// ============================================================================
#if defined(STM32F107xC)
static HAL_StatusTypeDef InitClock_F107xC(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 72) || (pclk1_m != 36) || (pclk2_m != 72)) {
        Error_Handler(hclk_m * 5000000);
    }
    if (HSE_VALUE == 25000000) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        // 25Mhz / 5 * 8 / 5 * 9 = 72 Mhz
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
        RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
        RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
        RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
            != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }
        // DJ: is this needed?
        __HAL_RCC_PLLI2S_ENABLE();
    }
    else {
        // #error not supported.
    }
    return VerifyClocks(72000000, 36000000, 72000000);
}
#endif

// ============================================================================
#if defined(STM32F303xE)
static HAL_StatusTypeDef InitClock_F303xE(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 72) || (pclk1_m != 36) || (pclk2_m != 72)) {
        Error_Handler(hclk_m * 5000000);
    }
    if (HSE_VALUE == 8000000) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
        RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
            != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }
    }
    else {
        // #error not supported.
    }
    return VerifyClocks(72000000, 36000000, 72000000);
}
#endif

// ============================================================================
#if defined(STM32F407xx)
static HAL_StatusTypeDef InitClock_F407xx(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 168) || (pclk1_m != 42) || (pclk2_m != 84)) {
        Error_Handler(hclk_m * 5000000);
    }
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    if (HSE_VALUE == 12000000) {
        RCC_OscInitStruct.PLL.PLLM = 8;
        RCC_OscInitStruct.PLL.PLLN = 224;
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ = 4;
    }
    else if (HSE_VALUE == 8000000) {
        RCC_OscInitStruct.PLL.PLLM = 8;
        RCC_OscInitStruct.PLL.PLLN = 336;
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ = 4;
    }
    else {
        // #error not supported.
    }
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }

    return VerifyClocks(168000000, 42000000, 84000000);
}
#endif  // STM32F407xx

// ============================================================================
#if defined(STM32F427xx)
static HAL_StatusTypeDef InitClock_F427xx(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    uint32_t hclk, pclk1, pclk2;
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    if ((hclk_m == 180) && (pclk1_m != 45) && (pclk2_m != 90)) {

        if (HSE_VALUE == 8000000) {
            RCC_OscInitStruct.PLL.PLLM = 8;
            RCC_OscInitStruct.PLL.PLLN = 360;
            RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
            RCC_OscInitStruct.PLL.PLLQ = 4;
        }
        else if (HSE_VALUE == 24000000) {
            RCC_OscInitStruct.PLL.PLLM = 24;
            RCC_OscInitStruct.PLL.PLLN = 360;
            RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
            RCC_OscInitStruct.PLL.PLLQ = 4;
        }
        else {
            // #error not supported.
        }
        hclk = 180000000;
        pclk1 = 45000000;
        pclk2 = 90000000;
    }
    else if ((hclk_m == 176) && (pclk1_m != 44) && (pclk2_m != 88)) {
        if (HSE_VALUE == 8000000) {
            RCC_OscInitStruct.PLL.PLLM = 8;
            RCC_OscInitStruct.PLL.PLLN = 352;
            RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
            RCC_OscInitStruct.PLL.PLLQ = 4;
        }
        else if (HSE_VALUE == 24000000) {
            RCC_OscInitStruct.PLL.PLLM = 24;
            RCC_OscInitStruct.PLL.PLLN = 352;
            RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
            RCC_OscInitStruct.PLL.PLLQ = 4;
        }
        else {
            // #error not supported.
        }
        hclk = 176000000;
        pclk1 = 44000000;
        pclk2 = 88000000;
    }
    else {
        hclk = 0;
        pclk1 = 0;
        pclk2 = 0;
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }
    return VerifyClocks(hclk, pclk1, pclk2);
}
#endif  // STM32F427xx

// ============================================================================
#if defined(STM32F746xx) || defined(STM32F767xx)
static HAL_StatusTypeDef InitClock_F7xxxx(uint16_t hclk_m, uint16_t pclk1_m,
                                          uint16_t pclk2_m) {
    if ((hclk_m != 216) || (pclk1_m != 54) || (pclk2_m != 108)) {
        Error_Handler(hclk_m * 5000000);
    }
    if (HSE_VALUE == 8000000) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM = 8;
        RCC_OscInitStruct.PLL.PLLN = 432;
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ = 2;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7)
            != HAL_OK) {
            Error_Handler(HSE_VALUE * 5);
        }
    }
    else {
        // #error not supported.
    }

    return VerifyClocks(216000000, 54000000, 108000000);
}
#endif

// ============================================================================
static HAL_StatusTypeDef InitSystemClock(uint16_t hclk_m, uint16_t pclk1_m,
                                         uint16_t pclk2_m) {
#if defined(STM32F030x8)
    return InitClock_F030x8(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F103xB)
    return InitClock_F103xB(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F107xC)
    return InitClock_F107xC(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F303xE)
    return InitClock_F303xE(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F407xx)
    return InitClock_F407xx(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F427xx)
    return InitClock_F427xx(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F746xx)
    return InitClock_F7xxxx(hclk_m, pclk1_m, pclk2_m);
#elif defined(STM32F767xx)
    return InitClock_F7xxxx(hclk_m, pclk1_m, pclk2_m);
#else
#error InitSystemClock(): to implement and verify!
#endif
    // it will call HAL_MspInit() in stm32f1xx_hal_msp.c
    // thte HAL_MspInit() initialized the SysTick to 1K Hz frequency
    // however, our stime.config() will override it.
    HAL_Init();
    return HAL_OK;
}

// ============================================================================
static void InitNvicInterrupt(uint8_t group) {
#if defined(STM32F030x8)
    // seems no grouping for Cortex M0
    ( void )group;
    return;
#else
    switch (group) {
    case 0:
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
        break;
    case 1:
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_1);
        break;
    case 2:
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
        break;
    case 3:
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
        break;
    case 4:
    default:
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
        break;
    }
#endif
}

// ============================================================================
static void enableCanClock(CAN_TypeDef* CANx) {
// clang-format off
#if defined(CAN1_EXISTS)
    if (CANx == CAN1) { __HAL_RCC_CAN1_CLK_ENABLE(); return; }
#endif
#if defined(CAN2_EXISTS)
    if (CANx == CAN2) { __HAL_RCC_CAN2_CLK_ENABLE(); return; }
#endif
#if defined(CAN3_EXISTS)
    if (CANx == CAN3) { __HAL_RCC_CAN3_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )CANx;
}

// ============================================================================
static void enableGpioClock(GPIO_TypeDef* GPIOx) {
// clang-format off
#if defined(GPIOA_EXISTS)
    if (GPIOx == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); return; }
#endif
#if defined(GPIOB_EXISTS)
    if (GPIOx == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); return; }
#endif
#if defined(GPIOC_EXISTS)
    if (GPIOx == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); return; }
#endif
#if defined(GPIOD_EXISTS)
    if (GPIOx == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); return; }
#endif
#if defined(GPIOE_EXISTS)
    if (GPIOx == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); return; }
#endif
#if defined(GPIOF_EXISTS)
    if (GPIOx == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); return; }
#endif
#if defined(GPIOG_EXISTS)
    if (GPIOx == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); return; }
#endif
#if defined(GPIOH_EXISTS)
    if (GPIOx == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); return; }
#endif
#if defined(GPIOI_EXISTS)
    if (GPIOx == GPIOI) { __HAL_RCC_GPIOI_CLK_ENABLE(); return; }
#endif
#if defined(GPIOJ_EXISTS)
    if (GPIOx == GPIOJ) { __HAL_RCC_GPIOJ_CLK_ENABLE(); return; }
#endif
#if defined(GPIOK_EXISTS)
    if (GPIOx == GPIOK) { __HAL_RCC_GPIOK_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )GPIOx;
}

// ============================================================================
static void enableIicClock(I2C_TypeDef* IICx) {
// clang-format off
#if defined(IIC1_EXISTS)
    if (IICx == I2C1) { __HAL_RCC_I2C1_CLK_ENABLE(); return; }
#endif
#if defined(IIC2_EXISTS)
    if (IICx == I2C2) { __HAL_RCC_I2C2_CLK_ENABLE(); return; }
#endif
#if defined(IIC3_EXISTS)
    if (IICx == I2C3) { __HAL_RCC_I2C3_CLK_ENABLE(); return; }
#endif
#if defined(IIC4_EXISTS)
    if (IICx == I2C4) { __HAL_RCC_I2C4_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )IICx;
}

// ============================================================================
static void enableTimerClock(TIM_TypeDef* TIMx) {
// clang-format off
#if defined(TIM1_EXISTS)
    if (TIMx == TIM1) { __HAL_RCC_TIM1_CLK_ENABLE(); return; }
#endif
#if defined(TIM2_EXISTS)
    if (TIMx == TIM2) { __HAL_RCC_TIM2_CLK_ENABLE(); return; }
#endif
#if defined(TIM3_EXISTS)
    if (TIMx == TIM3) { __HAL_RCC_TIM3_CLK_ENABLE(); return; }
#endif
#if defined(TIM4_EXISTS)
    if (TIMx == TIM4) { __HAL_RCC_TIM4_CLK_ENABLE(); return; }
#endif
#if defined(TIM5_EXISTS)
    if (TIMx == TIM5) { __HAL_RCC_TIM5_CLK_ENABLE(); return; }
#endif
#if defined(TIM6_EXISTS)
    if (TIMx == TIM6) { __HAL_RCC_TIM6_CLK_ENABLE(); return; }
#endif
#if defined(TIM7_EXISTS)
    if (TIMx == TIM7) { __HAL_RCC_TIM7_CLK_ENABLE(); return; }
#endif
#if defined(TIM8_EXISTS)
    if (TIMx == TIM8) { __HAL_RCC_TIM8_CLK_ENABLE(); return; }
#endif
#if defined(TIM9_EXISTS)
    if (TIMx == TIM9) { __HAL_RCC_TIM9_CLK_ENABLE(); return; }
#endif
#if defined(TIM10_EXISTS)
    if (TIMx == TIM10) { __HAL_RCC_TIM10_CLK_ENABLE(); return; }
#endif
#if defined(TIM11_EXISTS)
    if (TIMx == TIM11) { __HAL_RCC_TIM11_CLK_ENABLE(); return; }
#endif
#if defined(TIM12_EXISTS)
    if (TIMx == TIM12) { __HAL_RCC_TIM12_CLK_ENABLE(); return; }
#endif
#if defined(TIM13_EXISTS)
    if (TIMx == TIM13) { __HAL_RCC_TIM13_CLK_ENABLE(); return; }
#endif
#if defined(TIM14_EXISTS)
    if (TIMx == TIM14) { __HAL_RCC_TIM14_CLK_ENABLE(); return; }
#endif
#if defined(TIM15_EXISTS)
    if (TIMx == TIM15) { __HAL_RCC_TIM15_CLK_ENABLE(); return; }
#endif
#if defined(TIM16_EXISTS)
    if (TIMx == TIM16) { __HAL_RCC_TIM16_CLK_ENABLE(); return; }
#endif
#if defined(TIM17_EXISTS)
    if (TIMx == TIM17) { __HAL_RCC_TIM17_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )TIMx;
}

// ============================================================================
// to be extended
static void enableUartClock(USART_TypeDef* USARTx) {
    // clang-format off
#if defined(USART1_EXISTS)
    if (USARTx == USART1) { __HAL_RCC_USART1_CLK_ENABLE(); return; }
#endif
#if defined(USART2_EXISTS)
#if defined(STM32F746xx) || defined(STM32F767xx)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct2 = { 0 };
    PeriphClkInitStruct2.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct2.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct2) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }
#endif // STM32F746xx || STM32F767xx
    if (USARTx == USART2) { __HAL_RCC_USART2_CLK_ENABLE(); return; }
#endif
#if defined(USART3_EXISTS)
#if defined(STM32F746xx) || defined(STM32F767xx)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct3 = { 0 };
    PeriphClkInitStruct3.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInitStruct3.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct3) != HAL_OK) {
        Error_Handler(HSE_VALUE * 5);
    }
#endif // STM32F746xx || STM32F767xx
    if (USARTx == USART3) { __HAL_RCC_USART3_CLK_ENABLE(); return; }
#endif
#if defined(UART4_EXISTS)
    if (USARTx == UART4) { __HAL_RCC_UART4_CLK_ENABLE(); return; }
#endif
#if defined(UART5_EXISTS)
    if (USARTx == UART5) { __HAL_RCC_UART5_CLK_ENABLE(); return; }
#endif
#if defined(USART6_EXISTS)
    if (USARTx == USART6) { __HAL_RCC_USART6_CLK_ENABLE(); return; }
#endif
#if defined(UART7_EXISTS)
    if (USARTx == UART7) { __HAL_RCC_UART7_CLK_ENABLE(); return; }
#endif
#if defined(UART8_EXISTS)
    if (USARTx == UART8) { __HAL_RCC_UART8_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )USARTx;
}

// ============================================================================
static void enableSpiClock(SPI_TypeDef* SPIx) {
    // clang-format off
#if defined(SPI1_EXISTS)
    if (SPIx == SPI1) { __HAL_RCC_SPI1_CLK_ENABLE(); return; }
#endif
#if defined(SPI2_EXISTS)
    if (SPIx == SPI2) { __HAL_RCC_SPI2_CLK_ENABLE(); return; }
#endif
#if defined(SPI3_EXISTS)
    if (SPIx == SPI3) { __HAL_RCC_SPI3_CLK_ENABLE(); return; }
#endif
#if defined(SPI4_EXISTS)
    if (SPIx == SPI4) { __HAL_RCC_SPI4_CLK_ENABLE(); return; }
#endif
#if defined(SPI5_EXISTS)
    if (SPIx == SPI5) { __HAL_RCC_SPI5_CLK_ENABLE(); return; }
#endif
#if defined(SPI6_EXISTS)
    if (SPIx == SPI6) { __HAL_RCC_SPI6_CLK_ENABLE(); return; }
#endif
    // clang-format on
    ( void )SPIx;
}

// ============================================================================
// good for GPIO_MODE_INPUT (not GPIO_Mode_IPU??), GPIO_MODE_OUTPUT_PP
static void setPinMode(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t mode) {
    assert_param(IS_GPIO_MODE(mode));
    uint16_t GPIO_PIN_x = 1 << pin_n;

// #if defined(STM32F107xC )  // || ( defined STM32F10Xxxxx)
// PB3/PB4 and PA15 is used as JTDO/TRACESWO after reset,
// therefore we must first disable JTAG and async trace functions to
// release PB3/PB4 for GPIO use this operation is confirmed on STM32F1
// micro controllers

// these need to be tested ------------------
#if !defined(STM32F030x8) && !defined(STM32F303xE) && !defined(STM32F407xx) \
    && !defined(STM32F427xx) && !defined(STM32F746xx) && !defined(STM32F767xx)
    if (((GPIOx == GPIOB)
         && ((GPIO_PIN_x & GPIO_PIN_3) || (GPIO_PIN_x & GPIO_PIN_4)))
        || ((GPIOx == GPIOA) && (GPIO_PIN_x == GPIO_PIN_15))) {
        __HAL_RCC_AFIO_CLK_ENABLE();
        DBGMCU->CR &= ~(DBGMCU_CR_TRACE_IOEN);
        __HAL_AFIO_REMAP_SWJ_NONJTRST();
    }
#endif

    enableGpioClock(GPIOx);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = GPIO_PIN_x;
    GPIO_InitStructure.Speed = GPIO_13RD_SPEED_HIGH;
    GPIO_InitStructure.Mode = mode;
    GPIO_InitStructure.Pull = GPIO_NOPULL;  // to verify

    HAL_GPIO_Init(GPIOx, &GPIO_InitStructure);
}

// ============================================================================
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
static void setPinAlter(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint8_t alt) {

    uint16_t GPIO_PIN_x = 1 << pin_n;

    enableGpioClock(GPIOx);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = GPIO_PIN_x;

    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = alt;
    GPIO_InitStructure.Pull = GPIO_NOPULL;  // this may differ for platforms
    HAL_GPIO_Init(GPIOx, &GPIO_InitStructure);
}
#endif

// ============================================================================
// GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN
static void setPinPull(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p) {
    assert_param(IS_GPIO_PULL(p));

    enableGpioClock(GPIOx);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = 1 << pin_n;
    GPIO_InitStructure.Speed = GPIO_13RD_SPEED_HIGH;
    GPIO_InitStructure.Pull = p;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStructure);
}

// ============================================================================
static void setPin(GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v) {
    uint16_t GPIO_Pin_x = 1 << pin_n;
    if (v) {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin_x, GPIO_PIN_RESET);
    }
}

// ============================================================================
static void togglePin(GPIO_TypeDef* GPIOx, uint8_t pin_n) {
    GPIOx->ODR ^= 1 << pin_n;
}

// ============================================================================
static bool readPin(GPIO_TypeDef* GPIOx, uint8_t pin_n) {
    return HAL_GPIO_ReadPin(GPIOx, 1 << pin_n);
}

// ============================================================================
#if defined(RTOS_IS_USED)
// ----------------------------------------------------------------------------
static void setRtosState(RtosState_e state) {
    rtos_state_ = state;
}

// ----------------------------------------------------------------------------
static RtosState_e getRtosState(void) {
    return rtos_state_;
}

#endif  // RTOS_IS_USED

// ============================================================================
// clang-format off
UtilsApi_t utils = {
    .system.initClock  = InitSystemClock  ,
    .system.initNvic   = InitNvicInterrupt,
    .clock.enableCan   = enableCanClock   ,
    .clock.enableGpio  = enableGpioClock  ,
    .clock.enableIic   = enableIicClock   ,
    .clock.enableSpi   = enableSpiClock   ,
    .clock.enableTimer = enableTimerClock ,
    .clock.enableUart  = enableUartClock  ,
    .pin.mode          = setPinMode       ,
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    .pin.alter = setPinAlter,
#endif
    .pin.pull   = setPinPull,
    .pin.set    = setPin    ,
    .pin.toggle = togglePin ,
    .pin.read   = readPin   ,
#if defined(RTOS_IS_USED)
    .rtos.setState = setRtosState,
    .rtos.getState = getRtosState,
#endif
};
// clang-format on
