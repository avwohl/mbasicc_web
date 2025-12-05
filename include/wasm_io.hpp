#pragma once
// MBASIC WebAssembly - Browser I/O Handler
// Implements IOHandler interface for web browser environment

#include <mbasic/io_handler.hpp>
#include <string>
#include <optional>

namespace mbasic {

// JavaScript callback functions (implemented in JavaScript, called from C++)
extern "C" {
    // Print text to the terminal
    void js_print(const char* text);

    // Get input from user (blocking via ASYNCIFY)
    // Prompt is displayed, returns dynamically allocated string
    char* js_input(const char* prompt);

    // Non-blocking key check (returns -1 if no key, else character code)
    int js_inkey();

    // Clear the terminal screen
    void js_clear_screen();
}

// WebAssembly IOHandler implementation
class WasmIO : public IOHandler {
public:
    WasmIO() = default;
    ~WasmIO() override = default;

    void print(const std::string& text) override;
    std::string input(const std::string& prompt) override;
    std::optional<char> inkey() override;
    int get_column() const override { return column_; }
    void set_column(int col) override { column_ = col; }
    int get_width() const override { return width_; }
    void set_width(int w) override { width_ = w; }
    void clear_screen() override;

private:
    int column_ = 0;
    int width_ = 80;
};

} // namespace mbasic
