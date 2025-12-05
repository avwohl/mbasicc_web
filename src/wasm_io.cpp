// MBASIC WebAssembly - Browser I/O Handler Implementation

#include "wasm_io.hpp"
#include <emscripten.h>
#include <cstdlib>

namespace mbasic {

// JavaScript functions implemented via EM_JS
EM_JS(void, js_print, (const char* text), {
    if (typeof Module.onPrint === 'function') {
        Module.onPrint(UTF8ToString(text));
    } else {
        console.log(UTF8ToString(text));
    }
});

EM_ASYNC_JS(char*, js_input, (const char* prompt), {
    if (typeof Module.onInput !== 'function') {
        console.error('Module.onInput not defined');
        return 0;
    }

    // Display prompt
    if (prompt) {
        Module.onPrint(UTF8ToString(prompt));
    }

    // Wait for input from JavaScript
    const result = await Module.onInput();

    // Allocate memory for result string and copy
    const len = lengthBytesUTF8(result) + 1;
    const ptr = _malloc(len);
    stringToUTF8(result, ptr, len);
    return ptr;
});

EM_JS(int, js_inkey, (), {
    if (typeof Module.onInkey === 'function') {
        const key = Module.onInkey();
        if (key !== null && key !== undefined) {
            return key.charCodeAt(0);
        }
    }
    return -1;
});

EM_JS(void, js_clear_screen, (), {
    if (typeof Module.onClearScreen === 'function') {
        Module.onClearScreen();
    }
});

void WasmIO::print(const std::string& text) {
    js_print(text.c_str());

    // Track column position
    for (char c : text) {
        if (c == '\n' || c == '\r') {
            column_ = 0;
        } else if (c == '\t') {
            column_ = ((column_ / 8) + 1) * 8;
        } else {
            column_++;
            if (column_ >= width_) {
                column_ = 0;
            }
        }
    }
}

std::string WasmIO::input(const std::string& prompt) {
    char* result = js_input(prompt.c_str());
    if (result) {
        std::string s(result);
        std::free(result);
        column_ = 0;  // Input ends with newline
        return s;
    }
    return "";
}

std::optional<char> WasmIO::inkey() {
    int key = js_inkey();
    if (key >= 0) {
        return static_cast<char>(key);
    }
    return std::nullopt;
}

void WasmIO::clear_screen() {
    js_clear_screen();
    column_ = 0;
}

} // namespace mbasic
