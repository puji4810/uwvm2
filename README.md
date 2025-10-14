<div align="center">
  <img src="documents/logo/uwvm2.svg" alt="uwvm2 logo" width="240"/>
  <br/>
  <h1>Ultimate WebAssembly Virtual Machine 2</h1>
</div>

> [!IMPORTANT]
> This project is currently only developer stable
>

<div style="text-align:center">
    <a href="https://github.com/UlteSoft/uwvm2/actions?query=workflow%3ACI">
        <img src="https://img.shields.io/github/actions/workflow/status/UlteSoft/uwvm2/ci.yml?branch=master" alt="github-ci" />
    </a>
    <a href="LICENSE.md">
        <img src="https://img.shields.io/badge/License-Apache%202.0-green.svg" , alt="License" />
    </a>
    <a href="https://en.cppreference.com">
        <img src="https://img.shields.io/badge/language-c++26-blue.svg", alt="cppreference" />
    </a>
</div>

## Introduction
Ultimate WebAssembly Virtual Machine 2

## Features
### WASM Feature Extensions
Most wasm standards are supported. See [feature.md](documents/features.md). For detailed changes in each WebAssembly release, see [wasm-release.md](documents/wasm-release.md).

### Supports multiple platforms
Supports over 100 triplet platforms, including DOS series, POSIX series, Windows 9x series, Windows NT series, and the Host C Library Series. Supports interpretation execution (INT), just-in-time compilation (JIT), and tiered compilation (TC). See [support.md](documents/support.md) for details.

## Commandline interface
* Get version information
```bash
$ uwvm --version
```
* Get a list of commands
```bash
$ uwvm --help
```
* Running uwvm2 virtual machine
```bash
$ uwvm <param0> <param1> ... --run <wasm> <argv1> <argv2> ...
```
* WASI mount dir
```bash
$ uwvm --wasi-mount-dir <wasi dir> <system dir> ... --run ...
```

## How to build
* Windows (aka. unknown-windows-msvc). See [windows.md](documents/how-to-build/windows.md)
* MinGW (aka. unknown-windows-gnu, unknown-w64-mingw32). See [mingw.md](documents/how-to-build/mingw.md)
* Linux (aka. unknown-linux-unknown). See [linux.md](documents/how-to-build/linux.md)
* Darwin (aka. unknown-apple-darwin). See [darwin.md](documents/how-to-build/darwin.md)
* FreeBSD (aka. unknown-freebsd(Version)). See [freebsd.md](documents/how-to-build/freebsd.md)
* WASM-WASI (self-hosting) (aka. [wasm32|wasm64]-[wasip1|wasip2]-(threads)). See [wasm-wasi.md](documents/how-to-build/wasm-wasi.md)
