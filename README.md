## one-third-hal-stm32

This is a middle layer of library for **STM32 series** of micro-controllers using the HAL library.

The purpose of this library is to make the code STM32 projects neat and simple.

This library is under development.

* Example projects are managed by **Makefile**
* Use **`arm-none-eabi-gcc`** to compile the code
* Use [**st-link**](https://github.com/stlink-org/stlink) to flash the binary into the micro-controllers
* Tested environment: **Ubuntu 16.04/18.04/20.04
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
├── f1-v1.8.4
├── f3-share
├── f3-v1.11.1
├── f4-share
├── f4-v1.25.2
├── f7-share
├── f7-v1.16.0
├── h7-share
├── h7-v1.8.0
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

* **f1-v1.8.4**: the HAL library for STM32F1 series of version v1.8.4. The files are generated from **STM32CubeMx**, with unused files removed.

* **f0-share** && **f0-v1.11.1**: the same as above, except that they are for F0 series.

* **f3-share** &&**f3-v1.11.1**: the same as above, except that they are for F3 series.

* **f4-share** && **f4-v1.25.2**: the same as above, except that they are for F4 series.

* **f7-share** && **f7-v1.16.0**: the same as above, except that they are for F7 series.

* **h7-share** && **h7-v1.8.0**: the same as above, except that they are for H7 series.

* **lds**: the linker scripts for different micro-controllers.
* **startups**: the startup source code for different micro-controllers.

#### core

##### `general-utils`

* `utils.system.initClock(...)`: setup the system clock. Note 1: we can only setup supported HCLK, PCLK1 and PCLK2 values, if they are set differently, ERROR LED will blink fast. This is the only way to show error before `uart-console` been setup correctly. Note 2: this function calls `HAL_Config()` to set the **interrupt group priority** to `NVIC_PRIORITYGROUP_4`.
* `utils.system.initNvic(uint8_t group)`: setup the interrupt group priority.
* `utils.clock.enableGpio(GPIO_TypeDef* GPIOx)`: To enable the clock of a GPIO group. This function is used by other modules.
* `utils.clock.enableTimer(TIM_TypeDef* TIMx)`: To enable the clock of a Timer. This function is used by other modules.
* `utils.clock.enableUart(USART_TypeDef* USARTx)`: To enable the clock of a UART/USART group. This function is used by other modules.
* `utils.clock.enableSpi(SPI_TypeDef* SPIx)`: to enable the clock of a SPI module. This function is used by other modules.
* `utils.pin.mode(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t io)`: to set the GPIO mode.
* `utils.pin.pull(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p)`: to enable internal pull up or pull down resister of a GPIO pin. 
* `utils.pin.set(GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v)`: to set a pin as HIGH (true) or LOW (false).
* `utils.pin.toggle(GPIO_TypeDef* GPIOx, uint8_t pin_n)`: to toggle an output GPIO pin.
* It also defines some common used MACROs as well, like `_SWAP16`, `_SWAP32`, `_CHECK_BIT`, `_SIZE_OF_ARRAY`, `_PACK`, and `offsetof()` and `container_of()`.

If some RTOS is used as with this library, two more functions are used to deal with the status of the RTOS:

* `rtos.setState(RtosState_t stat)`: set it to `FREERTOS_NOSTART` or `FREERTOS_STARTED`, and maybe other RTOS MACROs.
* `RtosState_t rtos.getState(void `: get the running status of a RTOS.

##### `stime-scheduler`

System time of this `core` . The default timer for this system time is `SysTick`. It is used as the reference clock for other modules, for example, the task scheduler submodule of `stime`.

The tick frequency can be set to `4000`, `2000`, `1000`, `500`, `400` or `200` tick per second, by using a MACRO in `config.h` in the project.

* `stime.config()`: just to configure the module.
* `stime.getTime()`: fetch the system time, which is defined as a structure `Stime_t`. 

* `stime.delay.us()`: delay in time us. This function will block the system.
* `stime.delay.ms()`: delay in time ms.
* `stime.scheduler.config()`: configuration to use the scheduler.
* `stime.scheduler.attach():` to attach a task to the scheduler.
* `stime.scheduler.show()`: to show the tasks attached to the scheduler, need to run it from the CLI.
* `stime.scheduler.run()`: to run the scheduler.
* `stime.scheduler.cliSuspend()`: to suspend the CLI for a while in seconds.

##### `uart-console`

We use console to interact with the micro-controller to check its status, for example, firmware information, includes git commit hash value, branch name, tag name, etc. This module uses functions from `console-printf` and `console-cli`.

* If no CLI (command line interface) is not used, the `console` only output information by calling `console.printf()` function, which is quite similar to `printf()` function in `stdio.h`.
* The `console.printf()` function uses one of the UART port, so we can use `screen`, or `putty`, etc, on the Ubuntu system, to connect to the micro-controller.
* CLI: the code is implemented in the file `console-cli.c`. When use `screen` to connect to the micro-controller, if there is any output from the micro-controller, you can enter `log 1` to stop it, then enter `help` to start use the CLI.

##### `led-status`

This module defines the action of using LEDs as heartbeat and error indicators.

This module is under development.

#### Supported ICs

The library is tested with the following micro-controllers:

| micro-controller | AHB/APB1/APB2 (MHz) |  HSE   | FLASH/RAM  | Tested Platform              |
| :--------------: | :--------------------: | :----: | :--------: | ---------------------------- |
|  STM32F030R8T6   |        48/48/--        |   8M   |   64K/8K   | NUCLEO-F030R8                |
|  STM32F103RBT6   |        72/36/72        |   8M   |  128K/20K  | NUCLEO-F103RB                |
|  STM32F107VCT6   |        72/36/72        |  25M   |  256K/64K  | Olimex-H107                  |
| STM32F303RET6 | 72/36/72 | 8M | 512K/80K | NUCLEO-F303RE |
|  STM32F407VGT6   |       168/42/84        | 8M/12M | 1024K/192K | STM32F4-DISCOVERY (modified) |
|  STM32F427VIT6   |       180/45/90        | 8M/24M | 2048K/256K | FireDragon Hexcopter         |
|  STM32F746ZGT6   |       216/54/108       |   8M   | 1024K/320K | NUCLEO-F746ZG                |
|  STM32F767ZIT6   |       216/54/108       |   8M   | 2048K/512K | NUCLEO-F767ZI                |

### Setup Development Environment

Install dependencies:

```bash
sudo apt-get install -y build-essential git flex bison libgmp3-dev libmpfr-dev 
sudo apt-get install -y libncurses5-dev libmpc-dev autoconf texinfo libtool
sudo apt-get install -y libftdi-dev libusb-1.0-0-dev zlib1g zlib1g-dev python-yaml
sudo apt-get install -y libncurses-dev
```

then do different things in different system.

#### On Ubuntu 16.04

Install `gcc-arm-embedded` (yes, we remove `gcc-arm-none-eabi`):

```bash
sudo apt-get remove gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
sudo apt-add-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install -y gcc-arm-embedded
```

#### On Ubuntu 18.04

Install `gcc-arm-embedded` (yes, we remove `gcc-arm-none-eabi`):

```bash
sudo echo "deb http://kr.archive.ubuntu.com/ubuntu bionic main universe" \
    | sudo tee -a /etc/apt/sources.list
sudo apt-get remove gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
sudo apt-add-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install -y gcc-arm-embedded
```

#### On Ubuntu 20.04

It turned out that ARM decided to make our life easier. So, the steps are as follows.

Install `gcc-arm-none-eabi` (yes, we install it again, and we use the version of September, 2020):

```bash
compiler_date="9-2020"
compiler_q="q2"
url="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm"
file="gcc-arm-none-eabi-${compiler_date}-${compiler_q}-update"
filename="${file}-x86_64-linux.tar.bz2"
wget ${url}/${compiler_date}${compiler_q}/${filename}
sudo tar xjf ${filename} -C /usr/share/
```

Then we need to create some symbolic links:

```bash
sudo ln -sf /usr/share/${file}/bin/arm-none-eabi-gcc     /usr/bin/arm-none-eabi-gcc 
sudo ln -sf /usr/share/${file}/bin/arm-none-eabi-g++     /usr/bin/arm-none-eabi-g++
sudo ln -sf /usr/share/${file}/bin/arm-none-eabi-gdb     /usr/bin/arm-none-eabi-gdb
sudo ln -sf /usr/share/${file}/bin/arm-none-eabi-size    /usr/bin/arm-none-eabi-size
sudo ln -sf /usr/share/${file}/bin/arm-none-eabi-objcopy /usr/bin/arm-none-eabi-objcopy
```

#### Use `djtools`

If you use [djtools](https://github.com/dj-zhou/djtools), the setup steps are simplified to a single command:

```bash
dj setup gcc-arm-stm32
```

It works for Either of the Ubuntu system above.

#### Setup ST-Link V2.1

We use st-link v2.1 to download the firmware to the STM32 micro-controllers.

Install dependencies:

```bash
sudo apt-get install -y libusb-1.0.0-dev gtk+-3.0
sudo apt-get install -y cu cutecom putty screen
sudo apt-get install -y cmake
```

Clone the `st-link` and switch to `v1.6.1` (only tested on Ubuntu 18.04/20.04):

```bash
git clone https://github.com/stlink-org/stlink
git checkout v1.6.1
```

Remove existing tools and install it:

```bash
sudo rm -rf /usr/local/bin/st-*
make release -j$(cat /proc/cpuinfo | grep processor | wc -l)
cd build/Release/
sudo make install
sudo ldconfig
```

The above setup steps are also simplified into:

```bash
dj setup stm32-tools
```

#### Setup Docker

Run the command from `djtools`:

```bash
dj setup container docker
```

### Examples

Examples can be found in `examples` directory.

#### Native Build

You can build the examples in following ways:

1. Build all projects in `examples`:

   ```bash
   cd path/to/one-third-hal-stm32
   ./examples/make.sh
   ```

   or:

   ```bash
   cd path/to/one-third-hal-stm32/examples
   ./make.sh
   ```

   it will take a long time to build every project.

   You can also add targets `hal`, `all`, `hal-clean` or `clean` to use `make.sh`.

2. Build one sub directory in `examples`:

   ```bash
   cd path/to/one-third-hal-stm32
   ./examples/make.sh examples/001-sysclk-led
   ```

   or:

   ```bash
   cd path/to/one-third-hal-stm32/examples
   ./make.sh 001-sysclk-led
   ```

   It will takes shorter time to finish because only one directory in `examples` is built.

   You CAN NOT add a target here since the directory is the target. and it will build `hal` first and then `all`.

3. Build one project use its `Makefile` directory:

   ```bash
   cd path/to/one-third-hal-stm32/examples/001-sysclk-led/f407zg/
   make
   ```

   all targets are supported, e.g., `hal`, `all`, `hal-clean` or `clean`.

#### Docker Build

You can compile the example code in docker container, no matter of your host system.

1. Build all projects in `examples`:

   ```bash
   cd path/to/one-third-hal-stm32
   ./build-in-docker
   ```

   it will use target `hal` first and then `all`, for all projects in `examples` directory.

2. Build one sub directory in `examples`:

   ```bash
   cd path/to/one-third-hal-stm32
   ./build-in-docker examples/001-sysclk-led
   ```

   it will use `hal` and then `all` as the target.

3. Build one project using its `Makefile`:

   ```bash
   cd path/to/one-third-hal-stm32
   ./build-in-docker examples/001-sysclk-led/f407zg

#### Download Firmware Natively

#### Download Firmware Using Docker Container

TODO
