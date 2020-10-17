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
├── f1-share
├── f1-v1.8.2
├── hal-lds
├── hal-startup
└── one-third-core
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

#### one-third-core

##### core-utils

* `initSystemClock()`: setup the system clock. Note: this function calls `HAL_Config()` to set the **interrupt group priority** to `NVIC_PRIORITYGROUP_4`.
* `initNvic()`: setup the interrupt group priority.
* `enableGpioClock()`: To enable the clock of a GPIO group. This function is used by other modules.
* `enableTimerClock()`: To enable the clock of a Timer. This function is used by other modules.
* `enableUartClock()`: To enable the clock of a UART/USART group. This function is used by other modules.

### Example Projects

#### `001-f107vct6-sysclk-led`

Basic project to setup the system clock (not the SysTick) and Toggle a LED in a while loop.

