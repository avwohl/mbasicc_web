// MBASIC WebAssembly - Embind Bindings
// Exposes the MBASIC interpreter to JavaScript

#include <emscripten/bind.h>
#include <emscripten.h>
#include <mbasic/lexer.hpp>
#include <mbasic/parser.hpp>
#include <mbasic/runtime.hpp>
#include <mbasic/interpreter.hpp>
#include <mbasic/error.hpp>
#include "wasm_io.hpp"
#include <memory>
#include <string>
#include <sstream>

using namespace emscripten;

namespace {

// Global state for the interpreter session
class MBasicSession {
public:
    MBasicSession() : io_(std::make_unique<mbasic::WasmIO>()) {}

    // Load a program from source code
    bool loadProgram(const std::string& source) {
        try {
            program_ = std::make_unique<mbasic::Program>(mbasic::parse(source));
            program_source_ = source;
            runtime_ = std::make_unique<mbasic::Runtime>();
            runtime_->load(*program_);
            interpreter_ = std::make_unique<mbasic::Interpreter>(*runtime_, io_.get());
            loaded_ = true;
            return true;
        } catch (const mbasic::ParseError& e) {
            last_error_ = "Parse error at line " + std::to_string(e.line) +
                          ", col " + std::to_string(e.column) + ": " + e.what();
            return false;
        } catch (const mbasic::LexerError& e) {
            last_error_ = "Lexer error at line " + std::to_string(e.line) +
                          ", col " + std::to_string(e.column) + ": " + e.what();
            return false;
        } catch (const std::exception& e) {
            last_error_ = std::string("Error: ") + e.what();
            return false;
        }
    }

    // Run the loaded program
    void run() {
        if (!loaded_ || !interpreter_) {
            return;
        }

        try {
            interpreter_->run();
        } catch (const mbasic::RuntimeError& e) {
            last_error_ = "Runtime error at line " + std::to_string(e.line) +
                          ": " + e.what();
            io_->print("\n" + last_error_ + "\n");
        } catch (const std::exception& e) {
            last_error_ = std::string("Error: ") + e.what();
            io_->print("\n" + last_error_ + "\n");
        }
    }

    // Execute a single tick (for cooperative multitasking)
    bool tick() {
        if (!loaded_ || !interpreter_) {
            return false;
        }

        try {
            return interpreter_->tick();
        } catch (const mbasic::RuntimeError& e) {
            last_error_ = "Runtime error at line " + std::to_string(e.line) +
                          ": " + e.what();
            io_->print("\n" + last_error_ + "\n");
            return false;
        } catch (const std::exception& e) {
            last_error_ = std::string("Error: ") + e.what();
            io_->print("\n" + last_error_ + "\n");
            return false;
        }
    }

    // Stop execution
    void stop() {
        if (interpreter_) {
            interpreter_->stop();
        }
    }

    // Pause execution
    void pause() {
        if (interpreter_) {
            interpreter_->pause();
        }
    }

    // Resume execution
    void resume() {
        if (interpreter_) {
            interpreter_->resume();
        }
    }

    // Provide input (for INPUT statement)
    void provideInput(const std::string& input) {
        if (interpreter_) {
            interpreter_->provide_input(input);
        }
    }

    // Reset execution (keep program)
    void reset() {
        if (runtime_) {
            runtime_->reset();
        }
    }

    // Clear everything
    void clear() {
        loaded_ = false;
        interpreter_.reset();
        runtime_.reset();
        program_.reset();
        program_source_.clear();
        last_error_.clear();
    }

    // Get the last error message
    std::string getLastError() const {
        return last_error_;
    }

    // Check if a program is loaded
    bool isLoaded() const {
        return loaded_;
    }

    // Check if execution is running
    bool isRunning() const {
        if (!runtime_) return false;
        return runtime_->pc.reason == mbasic::StopReason::RUNNING;
    }

    // Get current program counter line
    int getCurrentLine() const {
        if (!runtime_) return 0;
        return runtime_->pc.line;
    }

    // List the program
    std::string listProgram() const {
        return program_source_;
    }

    // Set terminal width
    void setWidth(int width) {
        io_->set_width(width);
    }

private:
    std::unique_ptr<mbasic::WasmIO> io_;
    std::unique_ptr<mbasic::Program> program_;
    std::unique_ptr<mbasic::Runtime> runtime_;
    std::unique_ptr<mbasic::Interpreter> interpreter_;
    std::string program_source_;
    std::string last_error_;
    bool loaded_ = false;
};

// Global session instance
MBasicSession g_session;

} // anonymous namespace

// Embind bindings
EMSCRIPTEN_BINDINGS(mbasic) {

    // Expose the session class
    class_<MBasicSession>("MBasicSession")
        .constructor<>()
        .function("loadProgram", &MBasicSession::loadProgram)
        .function("run", &MBasicSession::run)
        .function("tick", &MBasicSession::tick)
        .function("stop", &MBasicSession::stop)
        .function("pause", &MBasicSession::pause)
        .function("resume", &MBasicSession::resume)
        .function("provideInput", &MBasicSession::provideInput)
        .function("reset", &MBasicSession::reset)
        .function("clear", &MBasicSession::clear)
        .function("getLastError", &MBasicSession::getLastError)
        .function("isLoaded", &MBasicSession::isLoaded)
        .function("isRunning", &MBasicSession::isRunning)
        .function("getCurrentLine", &MBasicSession::getCurrentLine)
        .function("listProgram", &MBasicSession::listProgram)
        .function("setWidth", &MBasicSession::setWidth)
        ;

    // Global functions for simple API
    function("loadProgram", +[](const std::string& source) -> bool {
        return g_session.loadProgram(source);
    });

    function("runProgram", +[]() {
        g_session.run();
    });

    function("tickProgram", +[]() -> bool {
        return g_session.tick();
    });

    function("stopProgram", +[]() {
        g_session.stop();
    });

    function("resetProgram", +[]() {
        g_session.reset();
    });

    function("clearProgram", +[]() {
        g_session.clear();
    });

    function("getLastError", +[]() -> std::string {
        return g_session.getLastError();
    });

    function("isProgramLoaded", +[]() -> bool {
        return g_session.isLoaded();
    });

    function("isProgramRunning", +[]() -> bool {
        return g_session.isRunning();
    });

    function("provideInput", +[](const std::string& input) {
        g_session.provideInput(input);
    });

    function("listProgram", +[]() -> std::string {
        return g_session.listProgram();
    });

    function("setTerminalWidth", +[](int width) {
        g_session.setWidth(width);
    });
}
