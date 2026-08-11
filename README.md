# Caffeinix root filesystem

This repository builds the initial Caffeinix root filesystem from two external
inputs:

- an already installed RISC-V musl sysroot
- an unmodified BusyBox source tree

The rootfs build does not download, configure, or compile musl. It also does
not assume that musl, BusyBox, the kernel, and this repository are siblings.
Their locations must be supplied explicitly when invoking `make`.

## Supported target

- Architecture: RISC-V 64-bit little-endian
- ISA and ABI: RV64GC with LP64D
- Executable: static non-PIE musl ELF
- Tested libc: musl 1.2.6
- Tested BusyBox: 1.38.0
- Enabled applets: `ash`, `cat`, `cp`, `echo`, `ls`, `mkdir`, `pwd`, `rm`,
  and `touch`

## Prerequisites

Both supported hosts use their standard `riscv64-linux-gnu-*` packages. The
Makefile does not detect the host distribution.

### On Ubuntu

Ubuntu 22.04 or later:

```bash
sudo apt update
sudo apt install \
  build-essential git curl \
  gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

### On Arch

```bash
sudo pacman -S --needed \
  base-devel git curl riscv64-linux-gnu-gcc
```

A packaged RISC-V musl compiler is not required. The following section builds
the sysroot separately with the standard GNU cross compiler.

## Build an external musl sysroot

Download the tested musl release wherever you keep third-party source:

```bash
curl -LO \
  https://musl.libc.org/releases/musl-1.2.6.tar.gz
printf '%s  %s\n' \
  d585fd3b613c66151fc3249e8ed44f77020cb5e6c1e635a616d3f9f82460512a \
  musl-1.2.6.tar.gz | sha256sum --check
tar -xzf musl-1.2.6.tar.gz
```

Choose three independent absolute paths. The build and installation paths must
not be inside this rootfs repository.

```bash
MUSL_SOURCE_DIR=/absolute/path/to/musl-1.2.6
MUSL_BUILD_DIR=/absolute/path/to/build/musl-1.2.6
MUSL_INSTALL_DIR=/absolute/path/to/sysroots/riscv64-linux-musl

mkdir -p "$MUSL_BUILD_DIR" "$MUSL_INSTALL_DIR"
cd "$MUSL_BUILD_DIR"

"$MUSL_SOURCE_DIR/configure" \
  --target=riscv64-linux-musl \
  --prefix="$MUSL_INSTALL_DIR" \
  --disable-shared \
  --enable-wrapper=gcc \
  'CFLAGS=-march=rv64gc -mabi=lp64d' \
  CROSS_COMPILE=riscv64-linux-gnu-

make -j"$(nproc)"
make install
```

The result is a standalone build input:

```text
/absolute/path/to/sysroots/riscv64-linux-musl/
├── bin/musl-gcc
├── include/
└── lib/
    ├── crt1.o
    ├── libc.a
    └── musl-gcc.specs
```

The generated `musl-gcc` wrapper records the installation path. Do not move
the sysroot after installation; rebuild it with a new `--prefix` instead.

## Get rootfs and BusyBox sources

The repositories may live anywhere:

```bash
git clone \
  https://github.com/TroyMitchell911/caffeinix-rootfs.git \
  /absolute/path/to/caffeinix-rootfs

git clone --depth 1 --branch 1_38_0 \
  https://git.busybox.net/busybox \
  /absolute/path/to/busybox-1.38.0
```

No BusyBox patch or Caffeinix userspace syscall layer is applied.

## Building

Pass absolute paths for both required external inputs:

```bash
make -C /absolute/path/to/caffeinix-rootfs \
  -j"$(nproc)" \
  MUSL_SYSROOT=/absolute/path/to/sysroots/riscv64-linux-musl \
  BUSYBOX_DIR=/absolute/path/to/busybox-1.38.0
```

The build checks that:

- `MUSL_SYSROOT/bin/musl-gcc` is executable
- `MUSL_SYSROOT/lib/libc.a` exists
- `BUSYBOX_DIR/Makefile` exists
- the selected GNU cross binutils are available

It then applies `busybox.config`, compiles BusyBox with the supplied sysroot,
and creates `fs.img`. It never modifies or rebuilds the musl sysroot.

There are no default values for `MUSL_SYSROOT` or `BUSYBOX_DIR`. Omitting
either path is an error. Alternative GNU tool prefixes and wrapper paths remain
configurable:

```bash
make -C /absolute/path/to/caffeinix-rootfs \
  MUSL_SYSROOT=/absolute/path/to/musl-sysroot \
  BUSYBOX_DIR=/absolute/path/to/busybox \
  CROSS_COMPILE=another-riscv64-linux-gnu- \
  MUSL_CC=/absolute/path/to/musl-gcc
```

## Cleaning

Remove only rootfs-local output:

```bash
make -C /absolute/path/to/caffeinix-rootfs clean
```

Also clean the selected external BusyBox source tree:

```bash
make -C /absolute/path/to/caffeinix-rootfs clean \
  BUSYBOX_DIR=/absolute/path/to/busybox-1.38.0
```

Neither command removes or changes the musl sysroot.

## Image contents and format

`fs.img` is a 20,000-block raw Caffeinix filesystem image. Each block is
1 KiB. The little-endian on-disk layout contains a boot block, superblock,
journal, inode table, free-block bitmap, and data blocks. It is derived from
xv6 and is not ext4.

The image builder creates this root directory:

```text
/
├── busybox
├── LICENSE
└── console
```

BusyBox is configured for standalone-shell applets, so commands such as `ls`
do not need a `/bin/ls` symlink. `/console` is a character device used for
standard input, output, and error.

## Running with Caffeinix

The kernel source is not needed to build `fs.img`. To boot the completed image,
build the kernel separately and pass the image path explicitly:

```bash
make -C /absolute/path/to/caffeinix-kernel \
  -j"$(nproc)"
make -C /absolute/path/to/caffeinix-kernel qemu \
  FS_IMG=/absolute/path/to/caffeinix-rootfs/fs.img
```

The kernel build does not use `MUSL_SYSROOT` or `BUSYBOX_DIR`. Press `Ctrl-a`,
then `x`, to leave QEMU.

## Current limitations

- The image contains one static executable rather than a normal Linux tree.
- Only the applets listed above are enabled and tested.
- Basic `ash` works, but pipelines, job control, and real signals do not.
- Dynamic linking, shared libraries, networking, and package management are
  not available yet.

## Contributing

Report rootfs build and image issues through
[GitHub issues](https://github.com/TroyMitchell911/caffeinix-rootfs/issues).

## License

The image tooling is distributed under the GNU GPL version 3. See
[LICENSE](LICENSE). BusyBox and musl retain their respective upstream licenses.
