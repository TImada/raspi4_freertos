CROSS ?= aarch64-none-elf-
CFLAGS = -mcpu=cortex-a72 \
         -fpic \
         -ffreestanding \
         -std=gnu99 \
         -O2 \
         -Wall \
         -Wextra \
         -DGUEST \
         -I$(INCLUDEPATH1) \
         -I$(INCLUDEPATH2) \
         -I$(INCLUDEPATH3) \
         -I$(INCLUDEPATH4) \
         -I$(INCLUDEPATH5)
BUILTIN_OPS = -fno-builtin-memset
ASMFLAGS = -mcpu=cortex-a72

INCLUDEPATH1 ?= ./src
INCLUDEPATH2 ?= ../musl_libc
INCLUDEPATH3 ?= ../mmu
INCLUDEPATH4 ?= ../../../Source/include
INCLUDEPATH5 ?= ../../../Source/portable/GCC/ARM_CA72_64_BIT

# From ./src
OBJS = build/startup.o 
OBJS +=build/FreeRTOS_asm_vector.o
OBJS +=build/FreeRTOS_tick_config.o
OBJS +=build/interrupt.o
OBJS +=build/main.o
OBJS +=build/mmu_cfg.o
OBJS +=build/uart.o

# From ../mmu
OBJS +=build/mmu.o

# From ../cache
OBJS +=build/cache.o

# From ../musl_libc
OBJS +=build/memset.o
OBJS +=build/memcpy.o

# From ../../../Source/portable/GCC/ARM_CA72_64_BIT
OBJS +=build/port.o
OBJS +=build/portASM.o

OBJS +=build/list.o
OBJS +=build/queue.o
OBJS +=build/tasks.o
OBJS +=build/timers.o

OBJS +=build/heap_1.o

uart.elf : src/raspberrypi4.ld $(OBJS)
	$(CROSS)gcc -Wl,--build-id=none -std=gnu99 -T src/raspberrypi4.ld -o $@ -ffreestanding -nostdlib --specs=nosys.specs $(BUILTIN_OPS) $(OBJS)
	$(CROSS)objdump -d uart.elf > uart.list

build/%.o : src/%.S
	$(CROSS)as $(ASMFLAGS) -c -o $@ $<
	
build/%.o : src/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../mmu/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../cache/%.S
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../musl_libc/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../../../Source/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../../../Source/portable/GCC/ARM_CA72_64_BIT/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

build/%.o : ../../../Source/portable/GCC/ARM_CA72_64_BIT/%.S
	$(CROSS)as $(ASMFLAGS) -c -o $@ $<

build/%.o : ../../../Source/portable/MemMang/%.c
	$(CROSS)gcc $(CFLAGS)  -c -o $@ $<

clean :
	rm -f build/*.o
	rm -f *.elf
	rm -f *.list

