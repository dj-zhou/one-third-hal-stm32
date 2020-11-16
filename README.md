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
├── f0-share
├── f0-v1.11.1
├── f1-share
├── f1-v1.8.2
├── f4-share
├── f4-v1.25.2
├── f7-share
├── f7-v1.16.0
├── lds
└── startups
```

* **CMSIS**: is the core library files of ARM.

* **core**: the core components of this middle layer of library, the details are in the following sub section.

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

* **f1-v1.8.2**: the HAL library for STM32F1 series of version v1.8.2. The files are generated from **STM32CubeMx**. Unused files are removed.

* **f0-share/f0-v1.11.1**: the same as above, except that they are for F0 series.

* **f4-share/f4-v1.25.2**: the same as above, except that they are for F4 series.

* **f7-share/f7-v1.16.0**: the same as above, except that they are for F7 series.

* **lds**: the linker scripts for different micro-controllers.

* **startups**: the startup source code for different micro-controllers.

#### core

##### `general-utils`

* `system.initClock( void )`: setup the system clock. Note: this function calls `HAL_Config()` to set the **interrupt group priority** to `NVIC_PRIORITYGROUP_4`.
* `system.initNvic( uint8_t group )`: setup the interrupt group priority.
* `clock.enableGpio( GPIO_TypeDef* GPIOx )`: To enable the clock of a GPIO group. This function is used by other modules.
* `clock.enableTimer( TIM_TypeDef* TIMx )`: To enable the clock of a Timer. This function is used by other modules.
* `clock.enableUart( USART_TypeDef* USARTx )`: To enable the clock of a UART/USART group. This function is used by other modules.
* `pin.mode( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t io )`: to set the GPIO mode.
* `pin.pull( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p )`: to enable internal pull up or pull down resister of a GPIO pin. 
* `pin.set( GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v )`: to set a pin as HIGH (true) or LOW (false).
* `pin.toggle( GPIO_TypeDef* GPIOx, uint8_t pin_n )`: to toggle an output GPIO Pin.
* It defines some common used marcos as well.

If some RTOS is used as with this library, two more functions are used to deal with the status of the RTOS:

* `rtos.setState( RtosState_t state )`
* `RtosState_t rtos.getState( void )`.

##### `stime-scheduler`

System time of this `core` . The default timer for this system time is `SysTick`. It is used as the reference clock for other modules, for example, the task scheduler submodule of `stime`.

##### `uart-console`

We use console to interact with the micro-controller to check its status, for example, firmware information, includes git commit hash value, branch name, tag name, etc. This module uses functions from `console-printf` and `console-cli`.

* If no CLI (command line interface) is not used, the `console` only output information by calling `console.printf()` function, which is quite similar to `printf()` function in `stdio.h`.
* The `console.printf()` function uses one of the UART port, so we can use `screen`, or `putty`, etc, on the Ubuntu system, to connect to the micro-controller.
* CLI: the code is implemented in the file `console-cli.c`. When use `screen` to connect to the micro-controller, if there is any output from the micro-controller, you can enter `log 1` to stop it, then enter `help` to start use the CLI.

#### Supported ICs

The library is tested with the following micro-controllers:

| micro-controller | AHB/APB1/APB2 (MHz) |  HSE   | FLASH/RAM  | Tested Platform              |
| :--------------: | :--------------------: | :----: | :--------: | ---------------------------- |
|  STM32F030R8T6   |        48/48/--        |   8M   |   64K/8K   | NUCLEO-F030R8                |
|  STM32F103RBT6   |        72/36/72        |   8M   |  128K/20K  | NUCLEO-F103RB                |
|  STM32F107VCT6   |        72/36/72        |  25M   |  256K/64K  | Olimex-H107                  |
|  STM32F407VGT6   |       168/42/84        | 8M/12M | 1024K/192K | STM32F4-DISCOVERY (modified) |
|  STM32F427VIT6   |       180/45/90        | 8M/24M | 2048K/256K | FireDragon Hexcopter         |
|  STM32F746ZGT6   |       216/54/108       |   8M   | 1024K/320K | NUCLEO-F746ZG                |
|  STM32F767ZIT6   |       216/54/108       |   8M   | 2048K/512K | NUCLEO-F767ZI                |

