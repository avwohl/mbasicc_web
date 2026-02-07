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

## Deployment

Copy the contents of the `web/` directory to any static web server:

```
web/
├── index.html      # Main page
├── style.css       # Styling
├── mbasic-ui.js    # UI controller
├── mbasic.js       # WASM loader (generated)
└── mbasic.wasm     # WebAssembly binary (generated)
```

**Important:** Your web server must serve `.wasm` files with the correct MIME type:
```
Content-Type: application/wasm
```

Most modern web servers handle this automatically. If you encounter issues, configure your server to add this MIME type.

## Usage

### Terminal Commands

Type commands directly in the terminal input:

| Command | Description |
|---------|-------------|
| `NEW` | Clear the current program |
| `LIST` | Display the current program |
| `RUN` | Execute the current program |
| `CLS` | Clear the terminal screen |
| `FILES` | List files in virtual filesystem |
| `LOAD "filename"` | Load a program from virtual storage |
| `SAVE "filename"` | Save current program to virtual storage |

### Editor

- Write or paste BASIC code in the editor panel
- Click **Run** to execute (or press the Run button)
- Click **Stop** to halt a running program
- Use **Load/Save** buttons to manage files

### Example Program

```basic
10 PRINT "Hello, World!"
20 FOR I = 1 TO 10
30   PRINT I; " squared is "; I * I
40 NEXT I
50 INPUT "Enter your name: "; N$
60 PRINT "Hello, "; N$; "!"
```

## Project Structure

```
mbasicc_web/
├── makefile                 # Build configuration
├── include/
│   ├── wasm_io.hpp         # Browser I/O interface
│   └── wasm_filesystem.hpp # Virtual filesystem interface
├── src/
│   ├── wasm_io.cpp         # Terminal I/O implementation
│   ├── wasm_filesystem.cpp # Virtual filesystem implementation
│   └── wasm_bindings.cpp   # Emscripten/JavaScript bindings
└── web/
    ├── index.html          # Main HTML page
    ├── style.css           # Terminal styling
    ├── mbasic-ui.js        # UI controller
    ├── mbasic.js           # Generated WASM loader
    └── mbasic.wasm         # Compiled interpreter
```

## Technical Details

### Architecture

- **C++ Layer**: Wraps the mbasicc interpreter with custom I/O handlers for browser environments
- **Emscripten Embind**: Exposes C++ classes and functions to JavaScript
- **ASYNCIFY**: Enables blocking I/O operations (like `INPUT`) in WebAssembly by transforming them into async/await patterns
- **Virtual Filesystem**: In-memory file storage implemented in JavaScript

### Limitations

- **No persistent storage**: Files are lost on page reload (use download to save)
- **Text-only**: No graphics or sound support
- **Single-threaded**: One program runs at a time
- **Memory-bound**: Limited by browser available memory

## License

See the [mbasicc repository](https://github.com/avwohl/mbasicc) for license information.
## Related Projects

- [80un](https://github.com/avwohl/80un) - Unpacker for CP/M compression and archive formats (LBR, ARC, squeeze, crunch, CrLZH)
- [cpmdroid](https://github.com/avwohl/cpmdroid) - Z80/CP/M emulator for Android with RomWBW HBIOS compatibility and VT100 terminal
- [cpmemu](https://github.com/avwohl/cpmemu) - CP/M 2.2 emulator with Z80/8080 CPU emulation and BDOS/BIOS translation to Unix filesystem
- [ioscpm](https://github.com/avwohl/ioscpm) - Z80/CP/M emulator for iOS and macOS with RomWBW HBIOS compatibility
- [learn-ada-z80](https://github.com/avwohl/learn-ada-z80) - Ada programming examples for the uada80 compiler targeting Z80/CP/M
- [mbasic](https://github.com/avwohl/mbasic) - Modern MBASIC 5.21 Interpreter & Compilers
- [mbasic2025](https://github.com/avwohl/mbasic2025) - MBASIC 5.21 source code reconstruction - byte-for-byte match with original binary
- [mbasicc](https://github.com/avwohl/mbasicc) - C++ implementation of MBASIC 5.21
- [mpm2](https://github.com/avwohl/mpm2) - MP/M II multi-user CP/M emulator with SSH terminal access and SFTP file transfer
- [romwbw_emu](https://github.com/avwohl/romwbw_emu) - Hardware-level Z80 emulator for RomWBW with 512KB ROM + 512KB RAM banking and HBIOS support
- [scelbal](https://github.com/avwohl/scelbal) - SCELBAL BASIC interpreter - 8008 to 8080 translation
- [uada80](https://github.com/avwohl/uada80) - Ada compiler targeting Z80 processor and CP/M 2.2 operating system
- [ucow](https://github.com/avwohl/ucow) - Unix/Linux Cowgol to Z80 compiler
- [um80_and_friends](https://github.com/avwohl/um80_and_friends) - Microsoft MACRO-80 compatible toolchain for Linux: assembler, linker, librarian, disassembler
- [upeepz80](https://github.com/avwohl/upeepz80) - Universal peephole optimizer for Z80 compilers
- [uplm80](https://github.com/avwohl/uplm80) - PL/M-80 compiler targeting Intel 8080 and Zilog Z80 assembly language
- [z80cpmw](https://github.com/avwohl/z80cpmw) - Z80 CP/M emulator for Windows (RomWBW)

