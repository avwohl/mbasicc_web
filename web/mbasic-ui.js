// MBASIC 5.21 WebAssembly - User Interface
// Provides terminal interaction and file management for the BASIC interpreter

// Module will be loaded from mbasic.js
let Module = null;

// UI Elements
const output = document.getElementById('output');
const input = document.getElementById('input');
const prompt = document.getElementById('prompt');
const editor = document.getElementById('editor');
const btnRun = document.getElementById('btn-run');
const btnStop = document.getElementById('btn-stop');
const btnLoad = document.getElementById('btn-load');
const btnSave = document.getElementById('btn-save');
const btnUpload = document.getElementById('btn-upload');
const btnDownload = document.getElementById('btn-download');
const fileUpload = document.getElementById('file-upload');
const fileList = document.getElementById('file-list');

// State
let isRunning = false;
let inputResolve = null;
let inputBuffer = [];
let commandHistory = [];
let historyIndex = -1;
let virtualFiles = new Map();

// Print text to the terminal
function print(text) {
    const span = document.createElement('span');
    span.textContent = text;
    output.appendChild(span);
    output.scrollTop = output.scrollHeight;
}

// Print error text
function printError(text) {
    const span = document.createElement('span');
    span.className = 'error';
    span.textContent = text;
    output.appendChild(span);
    output.scrollTop = output.scrollHeight;
}

// Print system message
function printSystem(text) {
    const span = document.createElement('span');
    span.className = 'system';
    span.textContent = text;
    output.appendChild(span);
    output.scrollTop = output.scrollHeight;
}

// Clear the terminal
function clearScreen() {
    output.innerHTML = '';
}

// Get input from user (async, used by WASM via ASYNCIFY)
async function getInput() {
    return new Promise((resolve) => {
        inputResolve = resolve;
        input.focus();
    });
}

// Check for buffered key (non-blocking)
function checkInkey() {
    if (inputBuffer.length > 0) {
        return inputBuffer.shift();
    }
    return null;
}

// Handle input submission
function handleInput(text) {
    if (inputResolve) {
        print(text + '\n');
        const resolve = inputResolve;
        inputResolve = null;
        resolve(text);
    }
}

// Process a direct command (interactive mode)
function processCommand(cmd) {
    const trimmed = cmd.trim().toUpperCase();

    // Handle special commands
    if (trimmed === 'NEW') {
        if (Module) {
            Module.clearProgram();
        }
        editor.value = '';
        print('Ok\n');
        return;
    }

    if (trimmed === 'LIST') {
        if (Module) {
            const listing = Module.listProgram();
            if (listing) {
                print(listing + '\n');
            }
        }
        print('Ok\n');
        return;
    }

    if (trimmed === 'RUN') {
        runProgram();
        return;
    }

    if (trimmed === 'CLS') {
        clearScreen();
        print('Ok\n');
        return;
    }

    if (trimmed === 'FILES') {
        listFiles();
        return;
    }

    if (trimmed.startsWith('LOAD ')) {
        const filename = cmd.substring(5).trim().replace(/"/g, '');
        loadFile(filename);
        return;
    }

    if (trimmed.startsWith('SAVE ')) {
        const filename = cmd.substring(5).trim().replace(/"/g, '');
        saveFile(filename);
        return;
    }

    // Try to execute as immediate command or add line
    if (/^\d+/.test(trimmed)) {
        // Line with number - add to editor
        addLineToEditor(cmd);
    } else {
        // Try as immediate command
        executeImmediate(cmd);
    }
}

// Add a numbered line to the editor
function addLineToEditor(line) {
    const match = line.match(/^(\d+)/);
    if (!match) return;

    const lineNum = parseInt(match[1]);
    const lines = editor.value.split('\n').filter(l => l.trim());
    const lineContent = line.trim();

    // Find and replace or insert
    let found = false;
    const newLines = lines.map(l => {
        const m = l.match(/^(\d+)/);
        if (m && parseInt(m[1]) === lineNum) {
            found = true;
            // If just line number, delete the line
            if (lineContent === lineNum.toString()) {
                return null;
            }
            return lineContent;
        }
        return l;
    }).filter(l => l !== null);

    if (!found && lineContent !== lineNum.toString()) {
        newLines.push(lineContent);
    }

    // Sort by line number
    newLines.sort((a, b) => {
        const numA = parseInt(a.match(/^(\d+)/)?.[1] || '0');
        const numB = parseInt(b.match(/^(\d+)/)?.[1] || '0');
        return numA - numB;
    });

    editor.value = newLines.join('\n');
    print('Ok\n');
}

// Execute an immediate command
function executeImmediate(cmd) {
    if (!Module) {
        printError('WASM module not loaded\n');
        return;
    }

    // For now, just show a message for unsupported immediate commands
    printSystem('Immediate mode not yet supported. Use editor and RUN.\n');
    print('Ok\n');
}

// Run the program
function runProgram() {
    if (!Module) {
        printError('WASM module not loaded\n');
        return;
    }

    const source = editor.value.trim();
    if (!source) {
        printError('No program to run\n');
        print('Ok\n');
        return;
    }

    print('\n');

    if (Module.loadProgram(source)) {
        isRunning = true;
        btnRun.disabled = true;
        btnStop.disabled = false;
        prompt.textContent = '';

        Module.runProgram();

        isRunning = false;
        btnRun.disabled = false;
        btnStop.disabled = true;
        prompt.textContent = 'Ok';
        print('\nOk\n');
    } else {
        printError(Module.getLastError() + '\n');
        print('Ok\n');
    }
}

// Stop the running program
function stopProgram() {
    if (Module) {
        Module.stopProgram();
    }
    isRunning = false;
    btnRun.disabled = false;
    btnStop.disabled = true;
    prompt.textContent = 'Ok';
    print('\nBreak\nOk\n');
}

// List virtual files
function listFiles() {
    if (virtualFiles.size === 0) {
        print('No files\n');
    } else {
        for (const [name, data] of virtualFiles) {
            print(`${name.padEnd(20)} ${data.length} bytes\n`);
        }
    }
    print('Ok\n');
}

// Load a file from virtual filesystem
function loadFile(filename) {
    const content = virtualFiles.get(filename) ||
                    virtualFiles.get(filename.toUpperCase());
    if (content) {
        editor.value = content;
        print(`Loaded ${filename}\nOk\n`);
    } else {
        printError(`File not found: ${filename}\n`);
        print('Ok\n');
    }
}

// Save to virtual filesystem
function saveFile(filename) {
    virtualFiles.set(filename, editor.value);
    updateFileList();
    print(`Saved ${filename}\nOk\n`);
}

// Update the file list UI
function updateFileList() {
    fileList.innerHTML = '';
    for (const [name, _] of virtualFiles) {
        const item = document.createElement('div');
        item.className = 'file-item';

        const nameSpan = document.createElement('span');
        nameSpan.textContent = name;
        nameSpan.onclick = () => {
            editor.value = virtualFiles.get(name) || '';
        };

        const deleteSpan = document.createElement('span');
        deleteSpan.className = 'delete';
        deleteSpan.textContent = 'x';
        deleteSpan.onclick = (e) => {
            e.stopPropagation();
            virtualFiles.delete(name);
            updateFileList();
        };

        item.appendChild(nameSpan);
        item.appendChild(deleteSpan);
        fileList.appendChild(item);
    }
}

// Setup event handlers
function setupEventHandlers() {
    // Terminal input
    input.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            const text = input.value;
            input.value = '';

            if (inputResolve) {
                // Program is waiting for input
                handleInput(text);
            } else {
                // Interactive command
                print(text + '\n');
                if (text.trim()) {
                    commandHistory.push(text);
                    historyIndex = commandHistory.length;
                    processCommand(text);
                } else {
                    print('Ok\n');
                }
            }
        } else if (e.key === 'ArrowUp') {
            // History navigation
            if (historyIndex > 0) {
                historyIndex--;
                input.value = commandHistory[historyIndex];
            }
            e.preventDefault();
        } else if (e.key === 'ArrowDown') {
            if (historyIndex < commandHistory.length - 1) {
                historyIndex++;
                input.value = commandHistory[historyIndex];
            } else {
                historyIndex = commandHistory.length;
                input.value = '';
            }
            e.preventDefault();
        } else if (e.key === 'Escape') {
            // Buffer the key for INKEY$
            inputBuffer.push(String.fromCharCode(27));
        }
    });

    // Capture keys for INKEY$ even when focused elsewhere
    document.addEventListener('keydown', (e) => {
        if (isRunning && e.target !== input) {
            inputBuffer.push(e.key.length === 1 ? e.key : '');
        }
    });

    // Button handlers
    btnRun.addEventListener('click', runProgram);
    btnStop.addEventListener('click', stopProgram);

    btnLoad.addEventListener('click', () => {
        const filename = window.prompt('Enter filename to load:');
        if (filename) {
            loadFile(filename);
        }
    });

    btnSave.addEventListener('click', () => {
        const filename = window.prompt('Enter filename to save:', 'PROGRAM.BAS');
        if (filename) {
            saveFile(filename);
        }
    });

    btnUpload.addEventListener('click', () => {
        fileUpload.click();
    });

    fileUpload.addEventListener('change', async (e) => {
        for (const file of e.target.files) {
            const content = await file.text();
            virtualFiles.set(file.name, content);
        }
        updateFileList();
        fileUpload.value = '';
    });

    btnDownload.addEventListener('click', () => {
        const filename = window.prompt('Enter filename to download:');
        if (filename) {
            const content = virtualFiles.get(filename);
            if (content) {
                const blob = new Blob([content], { type: 'text/plain' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = filename;
                a.click();
                URL.revokeObjectURL(url);
            } else {
                printError(`File not found: ${filename}\n`);
            }
        }
    });

    // Focus terminal on click
    document.getElementById('terminal').addEventListener('click', () => {
        input.focus();
    });
}

// Initialize the application
async function init() {
    printSystem('Loading MBASIC WebAssembly module...\n');

    try {
        // Load the WASM module
        const createModule = (await import('./mbasic.js')).default;

        Module = await createModule({
            // I/O callbacks
            onPrint: (text) => {
                print(text);
            },

            onInput: () => {
                return getInput();
            },

            onInkey: () => {
                return checkInkey();
            },

            onClearScreen: () => {
                clearScreen();
            },

            // File system callbacks
            onFileOpen: (filename, mode, recordLength) => {
                if (mode === 'input') {
                    return virtualFiles.get(filename) || null;
                } else {
                    return virtualFiles.get(filename) || '';
                }
            },

            onFileSave: (filename, data) => {
                virtualFiles.set(filename, data);
                updateFileList();
            },

            onFileExists: (filename) => {
                return virtualFiles.has(filename);
            },

            onFileDelete: (filename) => {
                virtualFiles.delete(filename);
                updateFileList();
            },

            onFileRename: (oldName, newName) => {
                const data = virtualFiles.get(oldName);
                if (data !== undefined) {
                    virtualFiles.delete(oldName);
                    virtualFiles.set(newName, data);
                    updateFileList();
                }
            }
        });

        clearScreen();
        print('MBASIC Version 5.21\n');
        print('Copyright (C) Microsoft 1977-1983\n');
        print('C++ Implementation by mbasicc project\n');
        print('WebAssembly port\n\n');
        print('Ok\n');

        setupEventHandlers();
        input.focus();

    } catch (error) {
        printError(`Failed to load WASM module: ${error.message}\n`);
        console.error(error);
    }
}

// Start the application
init();
