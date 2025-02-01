## Build on Mac OS (wip)

### Setup

```bash
% brew install homebrew/cask/gcc-arm-embedded
```

after install, check its version:

```bash
% arm-none-eabi-gcc --version
arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

However, I am not sure if that was because I have downloaded a pkg file and installed it from this page: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

### Build

#### Single Project

```bash
% cd examples/001-sysclk-led/f107vc
% make
```

Warnings and Errors:
```text
-L../../../one-third-hal/STM32F107xC-hal/ -o bin/STM32F107xC-binary.elf
/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/bin/ld: /Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/lib/thumb/v7-m/nofp/libc_nano.a(libc_a-closer.o): in function `_close_r':
closer.c:(.text._close_r+0xc): warning: _close is not implemented and will always fail
/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/bin/ld: /Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/lib/thumb/v7-m/nofp/libc_nano.a(libc_a-lseekr.o): in function `_lseek_r':
lseekr.c:(.text._lseek_r+0x10): warning: _lseek is not implemented and will always fail
/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/bin/ld: /Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/lib/thumb/v7-m/nofp/libc_nano.a(libc_a-readr.o): in function `_read_r':
readr.c:(.text._read_r+0x10): warning: _read is not implemented and will always fail
/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/bin/ld: /Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/lib/thumb/v7-m/nofp/libc_nano.a(libc_a-writer.o): in function `_write_r':
writer.c:(.text._write_r+0x10): warning: _write is not implemented and will always fail
/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../arm-none-eabi/bin/ld: cannot find -lSTM32F107xC: No such file or directory
collect2: error: ld returned 1 exit status
make: *** [bin/STM32F107xC-binary.elf] Error 1
```

It was because the library was not build. So the corect comand to build is:
```bash
$ make hal -j10
$ make
// delete a lot of output
arm-none-eabi-objcopy -O ihex bin/STM32F107xC-binary.elf bin/STM32F107xC-binary.hex
arm-none-eabi-objcopy -O binary -S bin/STM32F107xC-binary.elf bin/STM32F107xC-binary.bin
arm-none-eabi-size bin/STM32F107xC-binary.elf
   text	   data	    bss	    dec	    hex	filename
   6256	     72	   1568	   7896	   1ed8	bin/STM32F107xC-binary.elf
```

#### Whole Example

```bash
% cd examples
% ./make.sh hal
% ./make.sh
```

### Question

How to integrate into Dockerfile? Maybe I should not?
