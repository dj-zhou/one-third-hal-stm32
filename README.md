## one-third-hal-stm32

This is a middle layer of library for **STM32 series** of micro-controllers using the HAL library.

The purpose of this library is to make the code STM32 projects neat and simple.

This library is under development.

* Example projects are managed by **Makefile**
* Use **`arm-none-eabi-gcc`** to compile the code
* Use [**st-link**](https://github.com/stlink-org/stlink) to flash the binary into the micro-controllers
* Tested environment: **Ubuntu 18.04/20.04**
* Supported chips: STM32F107VCT6, (will support more)
* The library is **extensible** to add support to different micro-controllers in STM32 series
* The library source code is compiled with project code, because **macro** is used to select the component to compile. However, the HAL library will be compiled into a static library at some point of time, to decrease the compile time
* use **designated initializer** (GNU C style)  to make API functions more readable
* Project uses **`config.h`** to configure projects

### Library Structure

The library is in directory `one-third-hal`, its structure is as following:

```text
.
├── CMSIS
├── core
├── f1-share
├── f1-v1.8.2
├── lds
└── startups
```

* **CMSIS**: is the core library files of ARM

* **f1-share**: is a project related shared directory, including files:

  ```text
  .
  ├── stm32f1xx_hal_conf.h
  ├── stm32f1xx_hal_msp.c
  ├── stm32f1xx_it.c
  ├── stm32f1xx_it.h
  └── system_stm32f1xx.c
  ```

  The are modified to fit the library, and are not supposed to be revised again for different project, that why those files are in the library.

* **f1-v1.8.2**: the HAL library for STM32F1 series of version v1.8.2. The files are generated from **STM32CubeMx**. Unused files are removed

* **hal-lds**: the linker scripts for different micro-controllers

* **hal-startup**: the startup source code for different micro-controllers

* **one-third-core**: the core components of this middle layer of library, the details are in the following sub section

#### core

##### `general-utils`

* `initSystemClock( void )`: setup the system clock. Note: this function calls `HAL_Config()` to set the **interrupt group priority** to `NVIC_PRIORITYGROUP_4`.
* `initNvic( uint8_t group )`: setup the interrupt group priority.
* `enableGpioClock( GPIO_TypeDef* GPIOx )`: To enable the clock of a GPIO group. This function is used by other modules.
* `enableTimerClock( TIM_TypeDef* TIMx )`: To enable the clock of a Timer. This function is used by other modules.
* `enableUartClock( USART_TypeDef* USARTx )`: To enable the clock of a UART/USART group. This function is used by other modules.
* `setPinMode( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t io )`: to set the GPIO mode.
* `setPinPull( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p )`: to enable internal pull up or pull down resister of a GPIO pin. 
* `setPin( GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v )`: to set a pin as HIGH (true) or LOW (false).
* `togglePin( GPIO_TypeDef* GPIOx, uint8_t pin_n )`: to toggle an output GPIO Pin.
* It defines some common used marcos as well.

If some RTOS is used as with this library, two more functions are used to deal with the status of the RTOS:

* `setRtosState( RtosState_t state )`
* `RtosState_t getRtosState( void )`.

##### `stime-scheduler`

System time of this `core` . The default timer for this system time is `SysTick`. It is used as the reference clock for other modules, for example, the task scheduler submodule of `stime`.

##### `uart-console`

We use console to interact with the micro-controller to check its status, for example, firmware information, includes git commit hash value, branch name, tag name, etc.

* If no CLI (command line interface) is not used, the `console` only output information by calling `console.printf()` function, which is quite similar to `printf()` function in `stdio.h`.
* The `console.printf()` function uses one of the UART port, so we can use `screen`, or `putty`, etc, to connect to the micro-controller.
* CLI: todo

### Example Projects

#### `001-f107vct6-sysclk-led`

Basic project to setup the system clock (not the SysTick) and Toggle a LED in a while loop.

#### `002-f107vct6-stime`

Setup the SysTick to 4KHz/2KHz/1Khz/500Hz/400Hz/200Hz, and toggle a GPIO pin in `SysTick_Handler()` (need to add it manually in `core-stime.c`, since it is not a part of the library). Configure the module `stime` in `config.h` as:

```c
#define _STIME_USE_SYSTICK
#define _STIME_2K_TICK
#include "stime-scheduler.h"
```



#### `003-f107vct6-console-printf`

Setup a UART/USART port as the console, and use the console to print data, just like use it as `printf()` in `stdio.h`.

* The example code uses `UART5` on PC12 (TXD) and PD2 (RXD) as the console, configure it in `config.h` as:

  ```c
  #define _CONSOLE_USE_UART5_PC12PD2
  #include "uart-console.h"
  ```

* Color `printf()` is used in this example. Use `screen /dev/ttyUSB0 921600` to see its color effect.

#### `004-f107vct6-stime-delay`

This project demos the delay functions using the delay functions in `stime`, and print the time difference in the terminal.

Notice that these delay functions would block the program.

#### `005-f107vct6-stime-scheduler`

This project uses the module `stime ` as a task scheduler. To enable it, just add:

```c
#define _STIME_USE_SCHEDULER
```

