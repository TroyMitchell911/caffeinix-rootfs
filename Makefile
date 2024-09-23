ifndef CROSS_COMPILE
CROSS_COMPILE := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake CROSS_COMPILE= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

AS		= $(CROSS_COMPILE)gas
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

OUTPUT = output

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096
export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

UPROGS = \
	$(TOPDIR)/user/_init \
	$(TOPDIR)/user/_sh \
	$(TOPDIR)/user/_tuser \
	$(TOPDIR)/user/_ls \
	$(TOPDIR)/user/_mkdir \
	$(TOPDIR)/user/_touch \
	$(TOPDIR)/user/_cat \
	$(TOPDIR)/user/_cp \
	$(TOPDIR)/user/_rm \
	$(TOPDIR)/user/_pwd \

export UPROGS

TARGET := $(OUTPUT)/kernel

build:
	bear -- make all

fs.img: LICENSE $(UPROGS)
	make -C ./mkfs/ all
	mkfs/mkfs mkfs/fs.img LICENSE $(UPROGS)

all : user_build fs.img
	@echo "rootfs has been built!"

user_build:
	make -C ./user/ all

clean:
	@rm -f $(shell find -name "*.o")
	@rm -f $(shell find -name "*.asm")
	@rm -f $(shell find -name "*.sym")
	@rm -f $(shell find -name "*.d")
	@make -C ./user/ clean
	@make -C ./mkfs/ clean

distclean: clean
	@rm -f compile_commands.json
	@make -C ./user/ distclean
	@make -C ./mkfs/ distclean