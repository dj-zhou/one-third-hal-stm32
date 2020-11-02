#include "general-utils.h"

// ============================================================================
#if defined( RTOS_IS_USED )
static RtosState_e rtos_state_;
#endif

// ============================================================================
// clang-format off
#if defined(_LED_HEARTBEAT_PORT)
    #define ERROR_LED_PORT _LED_HEARTBEAT_PORT
#else 
    #define ERROR_LED_PORT GPIOA // not sure if this will conflict with something
#endif
#if defined(_LED_HEARTBEAT_PIN)
    #define ERROR_LED_PIN _LED_HEARTBEAT_PIN
#else 
    #define ERROR_LED_PIN 5 // not sure if this will conflict with something
#endif
// clang-format on

// ============================================================================
static void VerifyClocks( uint32_t hclk, uint32_t pclk1, uint32_t pclk2 ) {
    // why it is not HAL_RCC_GetSysClockFreq()
    uint32_t hclk_freq  = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();
#if !defined( STM32F030x8 )
    uint32_t pclk2_freq = HAL_RCC_GetPCLK2Freq();
#else
    uint32_t pclk2_freq = 0;  // to avoid writing code
#endif
    // no addclk?
    int step;
    if ( hclk < 50000000 ) {
        step = 50;
    }
    else if ( hclk < 100000000 ) {
        step = 100;
    }
    else if ( hclk < 200000000 ) {
        step = 2000;
    }
    else if ( hclk < 300000000 ) {
        step = 5000;
    }
    else {
        step = 10000;
    }
    if ( ( hclk_freq != hclk ) || ( pclk1_freq != pclk1 )
         || ( pclk2_freq != pclk2 ) ) {
        utils.pin.mode( ERROR_LED_PORT, ERROR_LED_PIN, GPIO_MODE_OUTPUT_PP );
        while ( 1 ) {
            for ( int i = 0; i < step; i++ ) {
                for ( int j = 0; j < 500; j++ ) {
                    ;
                }
            }
            utils.pin.toggle( ERROR_LED_PORT, ERROR_LED_PIN );
        }
    }
}

// ============================================================================
#if defined( STM32F030x8 )
static HAL_StatusTypeDef InitClock_F030x8( void ) {
    if ( HSE_VALUE == 8000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL6;
        RCC_OscInitStruct.PLL.PREDIV     = RCC_PREDIV_DIV1;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            // Error_Handler();  // todo
        }

        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

        if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 )
             != HAL_OK ) {
            // Error_Handler();  // todo
        }
    }
    else {
        // #error not supported.
    }
    VerifyClocks( 48000000, 48000000, 0 );  // no PCLK2
    return HAL_OK;
}
#endif

// ============================================================================
#if defined( STM32F103xB )
static HAL_StatusTypeDef InitClock_F103xB( void ) {
    if ( HSE_VALUE == 8000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
        RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            ;  // Error_Handler(); TODO
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
            // Error_Handler(); TODO
        }
    }
    else {
        // #error not supported.
    }
    VerifyClocks( 72000000, 36000000, 72000000 );
    return HAL_OK;
}
#endif  // STM32F103xB

// ============================================================================
#if defined( STM32F107xC )
static HAL_StatusTypeDef InitClock_F107xC( void ) {
    if ( HSE_VALUE == 25000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        // 25Mhz / 5 * 8 / 5 * 9 = 72 Mhz
        RCC_OscInitStruct.OscillatorType       = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState             = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue       = RCC_HSE_PREDIV_DIV5;
        RCC_OscInitStruct.HSIState             = RCC_HSI_ON;
        RCC_OscInitStruct.Prediv1Source        = RCC_PREDIV1_SOURCE_PLL2;
        RCC_OscInitStruct.PLL2.PLL2State       = RCC_PLL2_ON;
        RCC_OscInitStruct.PLL2.PLL2MUL         = RCC_PLL2_MUL8;
        RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
        RCC_OscInitStruct.PLL.PLLState         = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource        = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL           = RCC_PLL_MUL9;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            ;  // Error_Handler(); TODO
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
            ;  // Error_Handler(); TODO
        }
        // DJ: is this needed?
        __HAL_RCC_PLLI2S_ENABLE();
    }
    else {
        // #error not supported.
    }
    VerifyClocks( 72000000, 36000000, 72000000 );
    return HAL_OK;
}
#endif

// ============================================================================
#if defined( STM32F407xx )
static HAL_StatusTypeDef InitClock_F407xx( void ) {
    if ( HSE_VALUE == 12000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        /** Configure the main internal regulator output voltage
         */
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM       = 8;
        RCC_OscInitStruct.PLL.PLLN       = 224;
        RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ       = 4;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            // Error_Handler(); // TODO
        }

        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

        if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_5 )
             != HAL_OK ) {
            // Error_Handler(); // TODO
        }
    }
    else if ( HSE_VALUE == 8000000 ) {
        RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
        RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

        /** Configure the main internal regulator output voltage
         */
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
        /** Initializes the RCC Oscillators according to the specified
         * parameters in the RCC_OscInitTypeDef structure.
         */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM       = 8;
        RCC_OscInitStruct.PLL.PLLN       = 336;
        RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ       = 4;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            // Error_Handler();  // todo
        }
        /** Initializes the CPU, AHB and APB buses clocks
         */
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

        if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_5 )
             != HAL_OK ) {
            // Error_Handler();  // todo
        }
    }
    else {
        // #error not supported.
    }
    VerifyClocks( 168000000, 42000000, 84000000 );
    return HAL_OK;
}
#endif  // STM32F407xx

// ============================================================================
#if defined( STM32F767xx )
static HAL_StatusTypeDef InitClock_F767xx( void ) {
    if ( HSE_VALUE == 8000000 ) {
        RCC_OscInitTypeDef       RCC_OscInitStruct   = { 0 };
        RCC_ClkInitTypeDef       RCC_ClkInitStruct   = { 0 };
        RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

        /** Configure the main internal regulator output voltage
         */
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );
        /** Initializes the RCC Oscillators according to the specified
         * parameters in the RCC_OscInitTypeDef structure.
         */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM       = 8;
        RCC_OscInitStruct.PLL.PLLN       = 432;
        RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ       = 2;
        if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
            // Error_Handler();  // TODO
        }
        /** Activate the Over-Drive mode
         */
        if ( HAL_PWREx_EnableOverDrive() != HAL_OK ) {
            // Error_Handler();  // TODO
        }
        /** Initializes the CPU, AHB and APB buses clocks
         */
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                      | RCC_CLOCKTYPE_PCLK1
                                      | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

        if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_7 )
             != HAL_OK ) {
            // Error_Handler(); // TODO
        }
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
        PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
        if ( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
            // Error_Handler(); // TODO
        }
    }
    else {
        // #error not supported.
    }
    VerifyClocks( 216000000, 54000000, 108000000 );
    return HAL_OK;
}
#endif

// ============================================================================
static HAL_StatusTypeDef InitSystemClock( void ) {
#if defined( STM32F030x8 )
    return InitClock_F030x8();
#elif defined( STM32F103xB )
    return InitClock_F103xB();
#elif defined( STM32F107xC )
    return InitClock_F107xC();
#elif defined( STM32F407xx )
    return InitClock_F407xx();
#elif defined( STM32F767xx )
    return InitClock_F767xx();
#else
#error InitSystemClock(): TO IMPLEMENT
#endif
    // it will call HAL_MspInit() in stm32f1xx_hal_msp.c
    // thte HAL_MspInit() initialized the SysTick to 1K Hz frequency
    // however, our stime.config() will override it.
    HAL_Init();
    return HAL_OK;
}

// ============================================================================
static void InitNvicInterrupt( uint8_t group ) {
#if defined( STM32F030x8 )
    // seems no grouping for Cortex M0
    ( void )group;
    return;
#else
    switch ( group ) {
    case 0:
        HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_0 );
        break;
    case 1:
        HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_1 );
        break;
    case 2:
        HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_2 );
        break;
    case 3:
        HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_3 );
        break;
    case 4:
    default:
        HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );
        break;
        // can not do anything, because this function is usually called
        // before console setup and led_he setup
        break;
    }
#endif
}

// ============================================================================
static void enableGpioClock( GPIO_TypeDef* GPIOx ) {
    if ( GPIOx == GPIOA ) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if ( GPIOx == GPIOB ) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if ( GPIOx == GPIOC ) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
#if !defined( STM32F030x8 )
    else if ( GPIOx == GPIOD ) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if ( GPIOx == GPIOE ) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
#endif
    else if ( GPIOx == GPIOF ) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else {
        // do nothing, if the GPIO group is not designed for one family of
        // microcontroller, it will throw an error
    }
}

// ============================================================================
static void enableTimerClock( TIM_TypeDef* TIMx ) {
    if ( TIMx == TIM1 ) {
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if ( TIMx == TIM3 ) {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
#if !defined( STM32F030x8 )
    else if ( TIMx == TIM2 ) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
    else if ( TIMx == TIM4 ) {
        __HAL_RCC_TIM4_CLK_ENABLE();
    }
#endif
// the following may also work for F407, F767, etc. TODO
#if defined( STM32F107xC )
    else if ( TIMx == TIM5 ) {
        __HAL_RCC_TIM5_CLK_ENABLE();
    }
    else if ( TIMx == TIM6 ) {
        __HAL_RCC_TIM6_CLK_ENABLE();
    }
    else if ( TIMx == TIM7 ) {
        __HAL_RCC_TIM7_CLK_ENABLE();
    }
#endif
    else {
        // do nothing, if the timer is not designed for one family of
        // microcontroller, it will throw an error
    }
}

// ============================================================================
// to be extended
static void enableUartClock( USART_TypeDef* USARTx ) {
    if ( USARTx == USART1 ) {
        __HAL_RCC_USART1_CLK_ENABLE();
    }
    else if ( USARTx == USART2 ) {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
#if !defined( STM32F030x8 )
    else if ( USARTx == USART3 ) {
        __HAL_RCC_USART3_CLK_ENABLE();
    }
#endif
// the following may also work for F407, F767, etc. TODO
#if defined( STM32F107xC )
    else if ( USARTx == UART4 ) {
        __HAL_RCC_UART4_CLK_ENABLE();
    }
    else if ( USARTx == UART5 ) {
        __HAL_RCC_UART5_CLK_ENABLE();
    }
#endif
    else {
        // do nothing, if the uart/usart is not designed for one family of
        // microcontroller, it will throw an error
    }
}

// ============================================================================
// good for GPIO_MODE_INPUT (not GPIO_Mode_IPU??), GPIO_MODE_OUTPUT_PP
static void setPinMode( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t io ) {
    assert_param( IS_GPIO_MODE( io ) );
    uint16_t GPIO_PIN_x = 1 << pin_n;

#if defined( STM32F107xC )  // || ( defined STM32F10Xxxxx)
    // PB3/PB4 and PA15 is used as JTDO/TRACESWO after reset,
    // therefore we must first disable JTAG and async trace functions to
    // release PB3/PB4 for GPIO use this operation is confirmed on STM32F1
    // micro controllers

    // these need to be tested ------------------
    if ( ( ( GPIOx == GPIOB )
           && ( ( GPIO_PIN_x & GPIO_PIN_3 ) || ( GPIO_PIN_x & GPIO_PIN_4 ) ) )
         || ( ( GPIOx == GPIOA ) && ( GPIO_PIN_x == GPIO_PIN_15 ) ) ) {
        // RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
        __HAL_RCC_AFIO_CLK_ENABLE();
        DBGMCU->CR &= ~( DBGMCU_CR_TRACE_IOEN );
        // GPIO_PINRemapConfig( GPIO_Remap_SWJ_JTAGDisable, ENABLE );
        __HAL_AFIO_REMAP_SWJ_NONJTRST();
    }
#endif

    enableGpioClock( GPIOx );

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin   = GPIO_PIN_x;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode  = io;
    HAL_GPIO_Init( GPIOx, &GPIO_InitStructure );
}

// ============================================================================
static void setPinPull( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p ) {
    assert_param( IS_GPIO_PULL( p ) );

    enableGpioClock( GPIOx );

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin   = 1 << pin_n;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode  = p;
    HAL_GPIO_Init( GPIOx, &GPIO_InitStructure );
}

// ============================================================================
static void setPin( GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v ) {
    uint16_t GPIO_Pin_x = 1 << pin_n;
    if ( v ) {
        HAL_GPIO_WritePin( GPIOx, GPIO_Pin_x, GPIO_PIN_SET );
    }
    else {
        HAL_GPIO_WritePin( GPIOx, GPIO_Pin_x, GPIO_PIN_RESET );
    }
}

// ============================================================================
static void togglePin( GPIO_TypeDef* GPIOx, uint8_t pin_n ) {
    GPIOx->ODR ^= 1 << pin_n;
}

// ============================================================================
#if defined( RTOS_IS_USED )
// ----------------------------------------------------------------------------
static void setRtosState( RtosState_e state ) {
    rtos_state_ = state;
}

// ----------------------------------------------------------------------------
static RtosState_e getRtosState( void ) {
    return rtos_state_;
}

#endif  // RTOS_IS_USED

// ============================================================================
// clang-format off
CoreUtilsApi_t utils = {
    .system.initClock  = InitSystemClock   ,
    .system.initNvic   = InitNvicInterrupt ,
    .clock.enableGpio  = enableGpioClock   ,
    .clock.enableTimer = enableTimerClock  ,
    .clock.enableUart  = enableUartClock   ,
    .pin.mode          = setPinMode        ,
    .pin.pull          = setPinPull        ,
    .pin.set           = setPin            ,
    .pin.toggle        = togglePin         ,
#if defined( RTOS_IS_USED )
    .rtos.setState     = setRtosState      ,
    .rtos.getState     = getRtosState      ,
#endif
};
// clang-format on
