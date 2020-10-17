#include "config.h"
#include "crystal.h"

#include "core-utils.h"

// ============================================================================
#ifdef STM32F107xC
static HAL_StatusTypeDef f107xC_clockInit( void ) {

    if ( HSE_VALUE == 25000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        // 25Mhz / 5 * 8 / 5 * 9 = 72 Mhz
        RCC_OscInitStruct.OscillatorType       = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState             = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue       = RCC_HSE_PREDIV_DIV5;
        RCC_OscInitStruct.HSIState             = RCC_HSI_ON;
        RCC_OscInitStruct.Prediv1Source        = RCC_PREDIV1_SOURCE_PLL2;
        RCC_OscInitStruct.PLL.PLLState         = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource        = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL           = RCC_PLL_MUL9;
        RCC_OscInitStruct.PLL2.PLL2State       = RCC_PLL2_ON;
        RCC_OscInitStruct.PLL2.PLL2MUL         = RCC_PLL2_MUL8;
        RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            Error_Handler();
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

        if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 )
             != HAL_OK ) {
            Error_Handler();
        }
        // DJ: is this needed?
        __HAL_RCC_PLLI2S_ENABLE();
    }
    else {
        // #error not supported.
    }

    return HAL_OK;
}
#endif

// ============================================================================
static HAL_StatusTypeDef systemClockInit( void ) {
#if ( defined STM32F107xC )
    return f107xC_clockInit();
#else
#error systemClockInit(): TO IMPLEMENT
#endif

    HAL_Init();
    return HAL_OK;
}

// ============================================================================
// clang-format off
CoreUtilsApi_t utils = {
    .systemClockInit   = systemClockInit    ,
};
// clang-format on
