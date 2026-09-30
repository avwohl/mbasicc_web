# mbasicc_web

A browser-based Microsoft BASIC-80 (version 5.21) interpreter compiled to WebAssembly. Run classic BASIC programs directly in your web browser with a retro terminal-style interface.

## Overview

This project takes the [mbasicc](https://github.com/avwohl/mbasicc) C++ interpreter and compiles it to WebAssembly using Emscripten. It provides a complete BASIC programming environment in the browser with:

- Interactive command mode (type commands directly)
- Program editor with syntax highlighting
- Virtual file system for saving/loading programs
- Terminal-style output with classic green-on-black aesthetics

## Requirements

### Build Requirements

- **Emscripten SDK** - C++ to WebAssembly compiler ([installation guide](https://emscripten.org/docs/getting_started/downloads.html))
- **mbasicc library** - Must be located at `../mbasicc` relative to this directory
- **Make** - Build tool
- **Python 3** - For the development server (optional)

### Runtime Requirements

- Modern web browser with WebAssembly support (Chrome, Firefox, Safari, Edge)

## Building

1. Ensure Emscripten SDK is installed and activated:
   ```bash
   source /path/to/emsdk/emsdk_env.sh
   ```

2. Clone the mbasicc library (if not already present):
   ```bash
   cd ..
   git clone https://github.com/avwohl/mbasicc.git
   cd mbasicc_web
   ```

3. Build the project:
   ```bash
   make
   ```

This produces:
- `web/mbasic.js` - JavaScript WebAssembly loader
- `web/mbasic.wasm` - Compiled WebAssembly binary

## Running Locally

Start the development server:
```bash
make serve
```

Then open http://localhost:8080 in your browser.

Alternatively, run manually:
```bash
cd web
python3 -m http.server 8080
```

## Usage

Type BASIC commands such as `RUN`, `LIST` and `SAVE "filename"` in the terminal input, or write code in the editor panel and click **Run**. [docs/usage.md](docs/usage.md) lists the terminal commands and the editor buttons.

### Example Program

```basic
10 PRINT "Hello, World!"
20 FOR I = 1 TO 10
30   PRINT I; " squared is "; I * I
40 NEXT I
50 INPUT "Enter your name: "; N$
60 PRINT "Hello, "; N$; "!"
```

## Documentation

- [docs/usage.md](docs/usage.md): terminal commands and the editor panel
- [docs/deployment.md](docs/deployment.md): deploying the `web/` directory to a static web server, the `.wasm` MIME type
- [docs/architecture.md](docs/architecture.md): project structure, architecture, limitations

## License

See the [mbasicc repository](https://github.com/avwohl/mbasicc) for license information.
## Related Projects

- [80un](https://github.com/avwohl/80un) - Unpacker for the CP/M archive and compression formats LBR, ARC, squeeze, crunch, and CrLZH.
- [cpmdroid](https://github.com/avwohl/cpmdroid) - Z80/CP/M emulator for Android phones and tablets. It emulates the RomWBW HBIOS interface and a VT100 terminal.
- [cpmemu](https://github.com/avwohl/cpmemu) - Z80/CP/M emulator for Linux and Windows, with Z80 and 8080 CPU cores. It translates the BDOS and BIOS calls of CP/M 2.2 programs to the host file system.
- [ioscpm](https://github.com/avwohl/ioscpm) - Z80/CP/M emulator for iOS and macOS. It emulates the RomWBW HBIOS interface and runs CP/M 2.2 and CP/M 3.
- [learn-ada-z80](https://github.com/avwohl/learn-ada-z80) - Collection of more than 90 Ada example programs for uada80, the Ada compiler for the Z80 processor and CP/M.
- [mbasic](https://github.com/avwohl/mbasic) - Python interpreter for MBASIC 5.21, the Microsoft BASIC-80 for CP/M. Two compiler backends compile the programs to CP/M .COM files or to JavaScript.
- [mbasic2025](https://github.com/avwohl/mbasic2025) - Reconstruction of the lost source code of MBASIC 5.21, the Microsoft BASIC-80 for CP/M. The MACRO-80 source code assembles to a binary that matches mbasic.com byte for byte.
- [mbasicc](https://github.com/avwohl/mbasicc) - C++17 interpreter for MBASIC 5.21, the Microsoft BASIC-80 for CP/M. It runs on Linux and macOS.
- [mpm2](https://github.com/avwohl/mpm2) - Z80 emulator for MP/M II, the multi-user CP/M operating system. Users connect over SSH, and SFTP clients transfer files.
- [romwbw_emu](https://github.com/avwohl/romwbw_emu) - Hardware-level Z80/CP/M emulator for Linux and macOS. It emulates the RomWBW HBIOS interface and switches banks in 512 KB of ROM and 512 KB of RAM.
- [scelbal](https://github.com/avwohl/scelbal) - Floating-point BASIC interpreter for the 8080 processor and CP/M. A translator converts the original 8008 source code to 8080 source code.
- [uada80](https://github.com/avwohl/uada80) - Ada compiler for the Z80 processor and CP/M 2.2. It compiles a subset of Ada 2012 to CP/M .COM files.
- [uc80](https://github.com/avwohl/uc80) - C compiler for the Z80 processor and CP/M. It optimizes for small code size.
- [ucow](https://github.com/avwohl/ucow) - Cowgol compiler for the Z80 processor and CP/M. It runs on Linux in Python.
- [um80_and_friends](https://github.com/avwohl/um80_and_friends) - Linux toolchain that is compatible with Microsoft MACRO-80. It has an assembler, a linker, a librarian, and a disassembler.
- [upeepz80](https://github.com/avwohl/upeepz80) - Peephole optimizer for Z80 compilers that write lowercase Z80 assembly language. It shortens jumps to jr, builds djnz loops, and removes dead stores.
- [uplm80](https://github.com/avwohl/uplm80) - PL/M-80 compiler for the Z80 processor and CP/M. It writes Intel 8080 and Zilog Z80 assembly language.
- [z80cpmw](https://github.com/avwohl/z80cpmw) - Z80/CP/M emulator for Windows. It emulates the RomWBW HBIOS interface and boots CP/M from disk images.
