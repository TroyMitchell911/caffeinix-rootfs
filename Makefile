CROSS_COMPILE ?= riscv64-linux-gnu-
MUSL_SYSROOT ?=
BUSYBOX_DIR ?=
MUSL_SYSROOT_ABS = $(abspath $(MUSL_SYSROOT))
BUSYBOX_DIR_ABS = $(abspath $(BUSYBOX_DIR))
MUSL_CC ?= $(MUSL_SYSROOT_ABS)/bin/musl-gcc
MKFS := mkfs/mkfs
MKFS_SOURCES := mkfs/mkfs.c $(wildcard mkfs/include/*.h) \
	$(wildcard include/*.h)
BUSYBOX_STATE := .busybox-configured

.PHONY: all check-inputs configure-busybox busybox clean

all: fs.img

check-inputs:
	@test -n "$(MUSL_SYSROOT)" || { \
		echo "MUSL_SYSROOT is required"; exit 1; \
	}
	@test -x "$(MUSL_CC)" || { \
		echo "missing musl compiler at $(MUSL_CC)"; exit 1; \
	}
	@test -f "$(MUSL_SYSROOT_ABS)/lib/libc.a" || { \
		echo "missing libc.a in $(MUSL_SYSROOT_ABS)"; exit 1; \
	}
	@test -n "$(BUSYBOX_DIR)" || { \
		echo "BUSYBOX_DIR is required"; exit 1; \
	}
	@test -f "$(BUSYBOX_DIR_ABS)/Makefile" || { \
		echo "missing BusyBox source at $(BUSYBOX_DIR_ABS)"; exit 1; \
	}
	@for tool_name in ar ld nm objcopy ranlib strip; do \
		command -v "$(CROSS_COMPILE)$$tool_name" >/dev/null || { \
			echo "missing $(CROSS_COMPILE)$$tool_name"; exit 1; \
		}; \
	done

configure-busybox: check-inputs
	@set -e; \
	busybox_path="$(BUSYBOX_DIR_ABS)"; \
	if [ ! -f "$(BUSYBOX_STATE)" ] || \
	   [ "$$(sed -n '1p' "$(BUSYBOX_STATE)")" != "$$busybox_path" ] || \
	   [ busybox.config -nt "$(BUSYBOX_STATE)" ] || \
	   [ Makefile -nt "$(BUSYBOX_STATE)" ]; then \
		$(MAKE) -C "$$busybox_path" distclean; \
		$(MAKE) -C "$$busybox_path" allnoconfig >/dev/null; \
		while IFS= read -r setting; do \
			symbol=$${setting%%=*}; \
			sed -i "s|^# $$symbol is not set$$|$$setting|; \
				s|^$$symbol=.*|$$setting|" \
				"$$busybox_path/.config"; \
		done < busybox.config; \
		yes '' | $(MAKE) -C "$$busybox_path" oldconfig >/dev/null; \
		printf '%s\n' "$$busybox_path" > "$(BUSYBOX_STATE)"; \
	fi

busybox: configure-busybox
	$(MAKE) -C "$(BUSYBOX_DIR_ABS)" ARCH=riscv \
		CROSS_COMPILE="$(CROSS_COMPILE)" CC="$(MUSL_CC)"

$(MKFS): $(MKFS_SOURCES)
	$(MAKE) -C mkfs all

fs.img: LICENSE busybox $(MKFS)
	$(MKFS) $@ LICENSE "$(BUSYBOX_DIR_ABS)/busybox"

clean:
	$(MAKE) -C mkfs clean
	@if [ -n "$(BUSYBOX_DIR)" ] && \
	   [ -d "$(BUSYBOX_DIR_ABS)" ]; then \
		$(MAKE) -C "$(BUSYBOX_DIR_ABS)" distclean; \
	fi
	$(RM) $(BUSYBOX_STATE) fs.img
