### Example Projects

#### `001-sysclk-led`

Basic project to setup the system clock (not the SysTick) and Toggle a LED in a while loop.

Tested on:

* F107VC
* F407ZG

Try different builds:

```bash
make release -j12
make download
```

or:

```bash
make debug -j12
make download
```

Run `make clean` before building different target.

#### `002-f107vc-stime`

Setup the SysTick to 4KHz/2KHz/1Khz/500Hz/400Hz/200Hz, and toggle a GPIO pin in `SysTick_Handler()` (need to add it manually in `stime-scheduler.c`, since it is not a part of the library). Configure the module `stime` in `config.h` as:

```c
#define _STIME_USE_SYSTICK
#define _STIME_2K_TICK
#include "stime-scheduler.h"
```

where we configure the system tick to 2Khz. We can revise it to `_STIME_4K_TICK`, etc, while the revision will not change the GPIO blink rate.

#### `003-f107vc-console-printf`

Setup a UART/USART port as the console, and use the console to print data, just like using it as `printf()` in `stdio.h`.

* The example code uses `UART5` on PC12 (TXD) and PD2 (RXD) as the console, configure it in `config.h` as:

  ```c
  #define _CONSOLE_USE_UART5_PC12PD2
  #include "uart-console.h"
  ```

* Color `printf()` is used in this example. Use `screen /dev/ttyUSB0 2000000` to see its color effect:

  ```bash
  ---------------------------------------------
  003-f107vc-console-printf
      74 = 0b 0000 0000 0100 1010
  pi = 3.14159
   this is a one-third demo program
   data_int = 3752, 7250, EA8
   data_int = ea8, 3752, 
   printf(%lu) not supported.
  ```

  (should be in different color)

#### `004-stime-delay`

This project demos the delay functions using the delay functions defined in the module `stime`, and print the time difference through the console.

The delay functions are blocking.

#### `005-f107vc-stime-scheduler`

This project uses the module `stime ` as a task scheduler. To enable it, just add:

```c
#define _STIME_USE_SCHEDULER
```

The scheduler will run CLI (refer to example 006) by default.

#### `006-f107vc-console-cli`

This project demonstrate the interaction from `screen` with the micro-controller using the built in `cli`. On the other hand, this project uses:

1. `scheduler` module to manage the tasks;

<img src="./console-cli.gif" width="700px">

2. If set the log level to 1, only the CLI interface will be shown. From there, you can try:

   ```bash
   cli-suspend 5
   ```

   after [ENTER], the CLI will be suspended for  5 seconds.

Built in CLI interface can be found by command:

```bash
help
```

Help information for any command can be seen by:

```bash
[command] help
```

For example:

```bash
1/3: log help
log
   view: see available log levels
   down: lower the log level
   rise: rise the log level
    [x]: set log level to x
1/3: cli-suspend help
cli-suspend
    [x]: suspend the cli for x seconds
```

#### `007-led-status`

This project is based on previous example project, and add the following:

1. `led-status` module to run the heartbeat task.

Tested on:

* F030R8
* F103RB
* F107VC
* F303VE
* F407VG
* F427VI
* F746ZG
* F767ZI

#### `008-spi`

Try the SPI interface, need logic analyzer to see how it works. Examples of devices using the SPI interface will be added later.

Tested on:

* F427VI
* F767ZI

#### `009-iic-eeprom`

Example of using the IIC interface to write to and read from a EEPROM chip: AT24C02. `EepromNode_t` related code will be added later.

<span style="color:red">Important Notice:</span> the IIC1 on PB8/PB9 on STM32F1 series does not work! I am 99% sure it is a hardware bug in STM32F1 micro-controllers. Avoid to use it.

Tested on:

* F407ZG
* F107VC (failed, need to test on some other pins)

#### `010-f767zi-ringbuffer`

Show how to use the `ringbuffer` APIs.

#### `011-uart-w-ringbuffer`

* `f407zg-usart1-rx-irq`: push each byte of data received from UART to its ringbuffer.
* `f407zg-usart1-rx-idle-dma`: using DMA to move data directly to the UART ringbuffer, and revise the ringbuffer parameter.
* `f767zi-usart2-tfmini`: run a TF-MINI-S sensor and show its data.

#### `012-can`

Demos to use CAN port(s).

#### `013-use-id`

Example of using the system ID defined in `one-third-hal/feature/id`.
#### `014-pc-uart-comm`

Example of communication between a PC and F407ZG board via uart (USART1) port.

#### `015-crc-calculate`
Use CRC soft and CRC hard on `f407zg` and `f767zi`. Hardware CRC is very fast.
