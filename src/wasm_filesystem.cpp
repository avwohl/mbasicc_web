// MBASIC WebAssembly - Browser File System Implementation

#include "wasm_filesystem.hpp"
#include <emscripten.h>
#include <cstdlib>

namespace mbasic {

// JavaScript functions for file operations
EM_JS(int, js_file_open, (const char* filename, int mode, int record_length), {
    if (typeof Module.fileSystem === 'undefined') {
        Module.fileSystem = {
            files: new Map(),
            nextHandle: 1,
            virtualFiles: new Map()  // In-memory file storage
        };
    }

    const fname = UTF8ToString(filename);

    // mode: 0=INPUT, 1=OUTPUT, 2=APPEND, 3=RANDOM
    const modeStr = ['input', 'output', 'append', 'random'][mode];

    // Check if we have a file access callback
    if (typeof Module.onFileOpen === 'function') {
        const handle = Module.fileSystem.nextHandle++;
        const fileData = Module.onFileOpen(fname, modeStr, record_length);
        if (fileData !== null) {
            Module.fileSystem.files.set(handle, {
                name: fname,
                mode: modeStr,
                recordLength: record_length,
                data: fileData,
                position: 0,
                eof: false
            });
            return handle;
        }
        return -1;
    }

    // Fallback: use virtual file system
    const handle = Module.fileSystem.nextHandle++;
    let data = '';

    if (mode === 0) {  // INPUT
        if (!Module.fileSystem.virtualFiles.has(fname)) {
            return -1;  // File not found
        }
        data = Module.fileSystem.virtualFiles.get(fname);
    } else if (mode === 1) {  // OUTPUT
        // Create/truncate file
        data = '';
    } else if (mode === 2) {  // APPEND
        data = Module.fileSystem.virtualFiles.get(fname) || '';
    } else if (mode === 3) {  // RANDOM
        data = Module.fileSystem.virtualFiles.get(fname) || '';
    }

    Module.fileSystem.files.set(handle, {
        name: fname,
        mode: modeStr,
        recordLength: record_length,
        data: data,
        position: 0,
        eof: false
    });

    return handle;
});

EM_JS(void, js_file_close, (int handle), {
    if (Module.fileSystem && Module.fileSystem.files.has(handle)) {
        const file = Module.fileSystem.files.get(handle);

        // Save to virtual filesystem if it was written
        if (file.mode === 'output' || file.mode === 'append' || file.mode === 'random') {
            Module.fileSystem.virtualFiles.set(file.name, file.data);

            // Notify JavaScript if callback exists
            if (typeof Module.onFileSave === 'function') {
                Module.onFileSave(file.name, file.data);
            }
        }

        Module.fileSystem.files.delete(handle);
    }
});

EM_JS(char*, js_file_read_line, (int handle), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return 0;
    }

    const file = Module.fileSystem.files.get(handle);
    if (file.position >= file.data.length) {
        file.eof = true;
        return 0;
    }

    // Find next newline
    let endPos = file.data.indexOf('\n', file.position);
    if (endPos === -1) {
        endPos = file.data.length;
    }

    const line = file.data.substring(file.position, endPos);
    file.position = endPos + 1;

    if (file.position >= file.data.length) {
        file.eof = true;
    }

    const len = lengthBytesUTF8(line) + 1;
    const ptr = _malloc(len);
    stringToUTF8(line, ptr, len);
    return ptr;
});

EM_JS(void, js_file_write_line, (int handle, const char* line), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }

    const file = Module.fileSystem.files.get(handle);
    const text = UTF8ToString(line);
    file.data += text + '\n';
    file.position = file.data.length;
});

EM_JS(void, js_file_write, (int handle, const char* data), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }

    const file = Module.fileSystem.files.get(handle);
    const text = UTF8ToString(data);
    file.data += text;
    file.position = file.data.length;
});

EM_JS(char*, js_file_read_chars, (int handle, int n), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return 0;
    }

    const file = Module.fileSystem.files.get(handle);
    const chars = file.data.substring(file.position, file.position + n);
    file.position += chars.length;

    if (file.position >= file.data.length) {
        file.eof = true;
    }

    const len = lengthBytesUTF8(chars) + 1;
    const ptr = _malloc(len);
    stringToUTF8(chars, ptr, len);
    return ptr;
});

EM_JS(int, js_file_eof, (int handle), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return 1;
    }
    const file = Module.fileSystem.files.get(handle);
    return file.eof || file.position >= file.data.length ? 1 : 0;
});

EM_JS(int, js_file_position, (int handle), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return 0;
    }
    return Module.fileSystem.files.get(handle).position;
});

EM_JS(int, js_file_length, (int handle), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return 0;
    }
    return Module.fileSystem.files.get(handle).data.length;
});

EM_JS(void, js_file_seek_record, (int handle, int record, int record_length), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }
    const file = Module.fileSystem.files.get(handle);
    file.position = (record - 1) * record_length;
    file.eof = file.position >= file.data.length;
});

EM_JS(void, js_file_read_raw, (int handle, char* buffer, int size), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }

    const file = Module.fileSystem.files.get(handle);
    const data = file.data.substring(file.position, file.position + size);

    for (let i = 0; i < size; i++) {
        if (i < data.length) {
            HEAP8[buffer + i] = data.charCodeAt(i);
        } else {
            HEAP8[buffer + i] = 0;
        }
    }

    file.position += size;
    if (file.position >= file.data.length) {
        file.eof = true;
    }
});

EM_JS(void, js_file_write_raw, (int handle, const char* buffer, int size), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }

    const file = Module.fileSystem.files.get(handle);
    let data = '';
    for (let i = 0; i < size; i++) {
        data += String.fromCharCode(HEAPU8[buffer + i]);
    }

    // If we're in the middle of the file, replace; otherwise append
    if (file.position < file.data.length) {
        file.data = file.data.substring(0, file.position) + data +
                    file.data.substring(file.position + size);
    } else {
        file.data += data;
    }
    file.position += size;
});

EM_JS(void, js_file_flush, (int handle), {
    if (!Module.fileSystem || !Module.fileSystem.files.has(handle)) {
        return;
    }
    const file = Module.fileSystem.files.get(handle);
    Module.fileSystem.virtualFiles.set(file.name, file.data);

    if (typeof Module.onFileSave === 'function') {
        Module.onFileSave(file.name, file.data);
    }
});

EM_JS(int, js_file_exists, (const char* filename), {
    if (!Module.fileSystem) {
        return 0;
    }
    const fname = UTF8ToString(filename);

    // Check custom handler first
    if (typeof Module.onFileExists === 'function') {
        return Module.onFileExists(fname) ? 1 : 0;
    }

    // Check virtual filesystem
    return Module.fileSystem.virtualFiles.has(fname) ? 1 : 0;
});

EM_JS(int, js_file_remove, (const char* filename), {
    if (!Module.fileSystem) {
        return 0;
    }
    const fname = UTF8ToString(filename);

    // Notify if callback exists
    if (typeof Module.onFileDelete === 'function') {
        Module.onFileDelete(fname);
    }

    return Module.fileSystem.virtualFiles.delete(fname) ? 1 : 0;
});

EM_JS(int, js_file_rename, (const char* old_name, const char* new_name), {
    if (!Module.fileSystem) {
        return 0;
    }
    const oldName = UTF8ToString(old_name);
    const newName = UTF8ToString(new_name);

    if (!Module.fileSystem.virtualFiles.has(oldName)) {
        return 0;
    }

    const data = Module.fileSystem.virtualFiles.get(oldName);
    Module.fileSystem.virtualFiles.delete(oldName);
    Module.fileSystem.virtualFiles.set(newName, data);

    // Notify if callback exists
    if (typeof Module.onFileRename === 'function') {
        Module.onFileRename(oldName, newName);
    }

    return 1;
});

// WasmFileHandle implementation

WasmFileHandle::WasmFileHandle(int handle) : handle_(handle) {}

WasmFileHandle::~WasmFileHandle() {
    if (open_) {
        close();
    }
}

bool WasmFileHandle::is_open() const {
    return open_;
}

void WasmFileHandle::close() {
    if (open_) {
        js_file_close(handle_);
        open_ = false;
    }
}

bool WasmFileHandle::read_line(std::string& line) {
    char* result = js_file_read_line(handle_);
    if (result) {
        line = result;
        std::free(result);
        return true;
    }
    return false;
}

void WasmFileHandle::write_line(const std::string& line) {
    js_file_write_line(handle_, line.c_str());
}

void WasmFileHandle::write(const std::string& data) {
    js_file_write(handle_, data.c_str());
}

std::string WasmFileHandle::read_chars(int n) {
    char* result = js_file_read_chars(handle_, n);
    if (result) {
        std::string s(result);
        std::free(result);
        return s;
    }
    return "";
}

bool WasmFileHandle::eof() const {
    return js_file_eof(handle_) != 0;
}

int64_t WasmFileHandle::position() const {
    return js_file_position(handle_);
}

int64_t WasmFileHandle::length() const {
    return js_file_length(handle_);
}

void WasmFileHandle::seek_record(int record, int record_length) {
    js_file_seek_record(handle_, record, record_length);
}

void WasmFileHandle::read_raw(char* buffer, int size) {
    js_file_read_raw(handle_, buffer, size);
}

void WasmFileHandle::write_raw(const char* buffer, int size) {
    js_file_write_raw(handle_, buffer, size);
}

void WasmFileHandle::flush() {
    js_file_flush(handle_);
}

// WasmFileSystem implementation

std::unique_ptr<FileHandle> WasmFileSystem::open(
    const std::string& filename,
    Mode mode,
    int record_length)
{
    int modeInt = static_cast<int>(mode);
    int handle = js_file_open(filename.c_str(), modeInt, record_length);

    if (handle < 0) {
        return nullptr;
    }

    return std::make_unique<WasmFileHandle>(handle);
}

bool WasmFileSystem::exists(const std::string& filename) {
    return js_file_exists(filename.c_str()) != 0;
}

bool WasmFileSystem::remove(const std::string& filename) {
    return js_file_remove(filename.c_str()) != 0;
}

bool WasmFileSystem::rename(const std::string& old_name, const std::string& new_name) {
    return js_file_rename(old_name.c_str(), new_name.c_str()) != 0;
}

} // namespace mbasic
