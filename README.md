# raspi4_freertos

This repository includes a FreeRTOS UART sample application which can run on Raspberry Pi 4B.

## 1. Overview

This FreeRTOS porting uses UART2(PL011). The sample application is designed to be launched by u-boot and to operate together with 64-bit Linux.

Implementation is based on another FreeRTOS porting for Raspberry Pi 3 by eggman [1] (many thanks to him!).  

The sample application runs on the CPU core #3 on your Raspberry Pi 4B board. A specified memory region (0x20000000 - 0x207FFFFF) is dedicated to the sample application. Modify `FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/src/raspberrypi4.ld` if you want to change the memory usage.

ARMv8-a MMU is available with VA = PA configuration. The current implementation employs 2-level address translation (1GB-page for the 1st level, 2MB-page for the 2nd level). See `FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/src/mmu.c` for the detail.

[1] https://github.com/eggman/FreeRTOS-raspi3

## 2. Prerequisites

#### UART configuration

UART1(mini UART) must be enabled to manipulate u-boot. Add `enable_uart=1` to `config.txt`. See https://www.raspberrypi.org/documentation/configuration/uart.md for the detail.

#### Compiler installation

You need to install a GCC toolset for aarch64, and can get it from [2]. Don't forget to add its binary path to $PATH.

I used AArch64 ELF bare-metal target (aarch64-none-elf) version 9.2.1 for this repository.

[2] https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads

#### u-boot compilation

A pre-built u-boot image provided by Ubuntu or Debian may not have the  `dcache` command on the u-boot prompt. You need compile and install u-boot having cache commands if u-boot provided by your Linux distribution does not have it.

(1) Source code download  
`$ git clone https://github.com/u-boot/u-boot`  

(2) Compilation
```
$ cd u-boot
$ export CROSS_COMPILE=aarch64-none-elf-
$ echo 'CONFIG_CMD_CACHE=y' >> ./configs/rpi_4_defconfig
$ make rpi_4_defconfig
$ make -j4 (if your PC has 4 processor cores)
```
(`CROSS_COMPILE` must be changed depending on a compiler you installed)

(3) Copy the binary to your SD card  
```
$ sudo ./u-boot.bin /path/to/sd_boot_partition/kernel8.img
```
(The new file name must be `kernel8.img`)

## 3. FreeRTOS UART sample build

Very simple! Just execute the following commands.
```
$ cd Demo/CORTEX_A72_64-bit_Raspberrypi4/uart
$ make CROSS=aarch64-none-elf-
```
(`CROSS` must be changed depending on a compiler you installed)

MMU is enabled by default. You can easily disable it by removing or commenting out the configure_mmu() call.
```
(in FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/src/startup.S)
...
start_el1:
    ...
    // configure MMU
    // ldr   x0, =configure_mmu
    // blr   x0
    ...
```

Modify the page table configuration before compiling, if you want to change the memory location.  
(You must modify the linker script file `raspberrypi4.ld` too!)
```
(in FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/src/mmu.c)

/* Page table configuration array */
#define NUM_PT_CONFIGS (5)
static struct ptc_t pt_config[NUM_PT_CONFIGS] =
{
    { /* Code region (Read only) */
        .addr = 0x20000000ULL,
        .size = SIZE_2M,
        .executable = XN_OFF,
        .sharable = NON_SHARABLE,
        .permission = READ_WRITE,
        .policy = TYPE_MEM_CACHE_WB,
    },
    ...
}
```

## 4. Launching FreeRTOS by u-boot

(1) Copy the obtained binary to your SD card
```
$ sudo ./uart.elf /path/to/sd_boot_partition/
```

(2) Get the u-boot command on your Raspberry Pi 4B board  
Insert your SD card into your board, then power it on.

(3) Launch the FreeRTOS sample program on the u-boot prompt
```
dcache off
ext4load mmc 0:2 0x28000000 /path/to/uart.elf
dcache flush
bootelf 0x28000000
```

`mmc 0:2` in the `ext4load` command execution will vary depending on your SD card configuration. Don't forget to use the `fatload` command if you copied the sample program binary to a FAT partition.

You will see output by the UART sample program.
```
****************************

    FreeRTOS UART Sample

  (This sample uses UART2)

****************************

00000FF8
...
```

## 5. Launching FreeRTOS and Linux

This is little bit complicated. Follow the procedure below.

#### Device tree modification
You must remove PL011 related nodes indicating UART[0,2-5] from a device tree file your board uses. You must also add a memory region 0x20000000 - 0x207FFFFF to the `reserved-memory` scope in the device tree file.

I put a modified device tree file derived from ubuntu 20.04 LTS at `./dts/bcm2711-rpi-4-b-rtos.dts` for testing. You can compile it by executing the `dtc` command below.

```
$ dtc -O dtb -I dts ./bcm2711-rpi-4-b-rtos.dts -o ./bcm2711-rpi-4-b-rtos.dtb
```

You will obtain `bcm2711-rpi-4-b-rtos.dtb`. Copy it to an SD card partition where Linux kernel will load it. You may need to rename the binary dtb file or to modify a board configuration file such as `config.txt` to make it possible for Linux kernel to load it properly.


#### Sample program compilation  
You need to add a macro `-D__LINUX__` to `CFLAGS` in Makefile. This macro adds a special function to avoid GIC configuration change by Linux.

```
$ cd Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/
$ grep ^CFLAGS Makefile
CFLAGS = -mcpu=cortex-a72 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra -I$(INCLUDEPATH1) -I$(INCLUDEPATH2) -I$(INCLUDEPATH3) -DGUEST -D__LINUX__
$ make CROSS=aarch64-none-elf-
```
(`CROSS` must be changed depending on a compiler you installed)

#### Copy the obtained binary to your SD card  

Same as 4-(1) above.

#### Linux kernel parameter change

Add `maxcpus=3` to `cmdline.txt`. This enables Linux to use only CPU cores #0-2. The CPU core #3 can be used by FreeRTOS safely.

#### Launching FreeRTOS
Same as 4-(3). Execute the following commands on the u-boot prompt.
```
dcache off
ext4load mmc 0:2 0x28000000 /path/to/uart.elf
dcache flush
bootelf 0x28000000
dcache on
```
But you will see only a message
```
Waiting until Linux starts booting up ...
```
on UART2(PL011) until you launch Linux.

#### Launching Linux

Quite simple. Just execute
```
run bootcmd
```
on the u-boot prompt. You will see Linux boot process output on UART1(mini UART) and FreeRTOS UART output on UART2(PL011).

## 6. Debugging

(1) Compile and install the latest OpenOCD (http://openocd.org/repos/).

(2) Download a OpenOCD configuration file for Raspberry Pi 4B from [3] (Many thanks to the author!).
Then, comment out several lines from the file as shown below.

```
...
#   if {$_core != 0} {
#       set _smp_command "$_smp_command $_TARGETNAME.$_core"
#   } else {
#       set _smp_command "target smp $_TARGETNAME.$_core"
#   }
...
}

# eval $_smp_command
# targets $_TARGETNAME.0
```
[3] https://gist.github.com/tnishinaga/46a3380e1f47f5e892bbb74e55b3cf3e

(3) Start the OpenOCD process.
```
$ openocd -f /path/to/your_debugger.cfg -f raspi4.cfg
```

`your_debugger.cfg` varies depending on a debugger you use. It can be found in `tcl/interface/` included in the OpenOCD source directory.

(4) Connect to OpenOCD by gdb.
```
$ aarch64-none-elf-gdb /path/to/uart.elf

(on gdb console)
target remote localhost:3336
```
(`aarch64-none-elf-` must be changed depending on a compiler you installed)

You are now ready to start debugging FreeRTOS running on Cortex-A72 core#3. You can add the source code path on the gdb console.

## 7. License

MIT License derived from FreeRTOS. See `LICENSE.md` for the detail.
```
./FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/uart/
./FreeRTOS/Source/
```

MIT License derived from musl libc(https://musl.libc.org/). See individual files for the detail.

```
./FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/musl_libc/
```

GPL-2.0 derived from u-boot(https://github.com/u-boot/u-boot). See individual files for the detail.
```
./FreeRTOS/Demo/CORTEX_A72_64-bit_Raspberrypi4/cache/
```

GPL-2.0 derived from Linux(https://github.com/raspberrypi/linux).
```
./dts/
```
