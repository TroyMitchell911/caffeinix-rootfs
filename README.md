<h4 align="center">A rootfs for Caffeinix</h4>

<p align="center">
    <a href="https://github.com/TroyMitchell911/Caffeinix-rootfs/commits/main/">
    <img src="https://img.shields.io/github/last-commit/TroyMitchell911/Caffeinix-rootfs.svg?style=flat-square&logo=github&logoColor=white"
         alt="GitHub last commit">
    <a href="https://github.com/TroyMitchell911/Caffeinix-rootfs/issues">
    <img src="https://img.shields.io/github/issues-raw/TroyMitchell911/caffeinix-rootfs.svg?style=flat-square&logo=github&logoColor=white"
         alt="GitHub issues">
    <a href="https://github.com/TroyMitchell911/Caffeinix-rootfs/pulls">
    <img src="https://img.shields.io/github/issues-pr-raw/TroyMitchell911/Caffeinix-rootfs.svg?style=flat-square&logo=github&logoColor=white"
         alt="GitHub pull requests">
</p>


<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#updating">Updating</a> •
  <a href="#features">Features</a> •
  <a href="#wiki">Wiki</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#credits">Credits</a> •
  <a href="#support">Support</a> •
  <a href="#license">License</a>
</p>

---
<!--
<table>
<tr>
<td>

Rootfs using newlib c library.

</td>
</tr>
</table>
-->
## Operating environment

This is not **absolute**, just what we **recommend**.

```bash
Distributor ID: Ubuntu
Description:    Ubuntu 22.04.4 LTS
Release:        22.04
Codename:       jammy
```

## Prerequisites

Same as for [caffeinix](https://github.com/TroyMitchell911/caffeinix)

## Getting the sources

Congratulations, you have completed the above steps. Now let's get the code.

```bash
$ git clone https://github.com/TroyMitchell911/caffeinix-rootfs.git
```

## Building and usage

- `make`：Compile and build

You should have obtained the rootfs image after `make` command, named fs.img. please copy it to the root directory of the Caffeinix kernel. Now you can boot the kernel through qemu.

Enjoy Caffeinix :)
