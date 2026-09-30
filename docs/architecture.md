# Architecture

The source layout of mbasicc_web and how the C++ interpreter runs in the browser.

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

## Architecture

- **C++ Layer**: Wraps the mbasicc interpreter with custom I/O handlers for browser environments
- **Emscripten Embind**: Exposes C++ classes and functions to JavaScript
- **ASYNCIFY**: Enables blocking I/O operations (like `INPUT`) in WebAssembly by transforming them into async/await patterns
- **Virtual Filesystem**: In-memory file storage implemented in JavaScript

## Limitations

- **No persistent storage**: Files are lost on page reload (use download to save)
- **Text-only**: No graphics or sound support
- **Single-threaded**: One program runs at a time
- **Memory-bound**: Limited by browser available memory
