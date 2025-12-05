#pragma once
// MBASIC WebAssembly - Browser File System Handler
// Implements FileSystem interface for web browser environment
// Uses JavaScript callbacks to interact with browser File API

#include <mbasic/file_handler.hpp>
#include <string>
#include <memory>
#include <map>

namespace mbasic {

// JavaScript callback functions for file operations
extern "C" {
    // Open a file, returns handle ID or -1 on error
    int js_file_open(const char* filename, int mode, int record_length);

    // Close a file
    void js_file_close(int handle);

    // Read a line, returns dynamically allocated string or nullptr on EOF
    char* js_file_read_line(int handle);

    // Write a line
    void js_file_write_line(int handle, const char* line);

    // Write string without newline
    void js_file_write(int handle, const char* data);

    // Read n characters
    char* js_file_read_chars(int handle, int n);

    // Check EOF
    int js_file_eof(int handle);

    // Get position
    int js_file_position(int handle);

    // Get length
    int js_file_length(int handle);

    // Seek to record (for random access)
    void js_file_seek_record(int handle, int record, int record_length);

    // Read raw bytes
    void js_file_read_raw(int handle, char* buffer, int size);

    // Write raw bytes
    void js_file_write_raw(int handle, const char* buffer, int size);

    // Flush
    void js_file_flush(int handle);

    // Check if file exists
    int js_file_exists(const char* filename);

    // Delete file
    int js_file_remove(const char* filename);

    // Rename file
    int js_file_rename(const char* old_name, const char* new_name);
}

// WebAssembly FileHandle implementation
class WasmFileHandle : public FileHandle {
public:
    explicit WasmFileHandle(int handle);
    ~WasmFileHandle() override;

    bool is_open() const override;
    void close() override;
    bool read_line(std::string& line) override;
    void write_line(const std::string& line) override;
    void write(const std::string& data) override;
    std::string read_chars(int n) override;
    bool eof() const override;
    int64_t position() const override;
    int64_t length() const override;
    void seek_record(int record, int record_length) override;
    void read_raw(char* buffer, int size) override;
    void write_raw(const char* buffer, int size) override;
    void flush() override;

private:
    int handle_;
    bool open_ = true;
};

// WebAssembly FileSystem implementation
class WasmFileSystem : public FileSystem {
public:
    std::unique_ptr<FileHandle> open(
        const std::string& filename,
        Mode mode,
        int record_length = 128) override;

    bool exists(const std::string& filename) override;
    bool remove(const std::string& filename) override;
    bool rename(const std::string& old_name, const std::string& new_name) override;
};

} // namespace mbasic
