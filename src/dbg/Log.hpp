/// @file Log.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary
/// You are not allowed to use this file
/// without the author's permission.

#pragma once

#include <chrono>
#include <dbg/Logger.hpp>
#include <iostream>
#include <ostream>

namespace dbg {
using namespace std;

/// Use this on 'BufNewLine' to insert indent at every line
class Indent {
    /// String for one indent.
    const string one_indent{"    "};
    /// String for the current indent.
    string indent;

  public:
    /// Increase indent by one.
    inline void inc() { indent += one_indent; }
    /// Decrease indent by one.
    inline void dec() {
        indent.erase(indent.end() - one_indent.length(), indent.end());
    }

    /// This is called to insert indent at every line.
    /// @param buf
    /// @return int
    int call(streambuf &buf) {
        return buf.sputn(indent.c_str(), indent.size());
    }
};

/// The buffer that we use for logging.
using LoggerBuf = LineCallbackBuf<Indent>;

/// Indent tracker
static Indent indent{};
/// Our custom log buffer
static LoggerBuf buf{cout, indent};
/// Our custom log buffer for errors.
static LoggerBuf errbuf{cerr, indent};
/// Log stream
static ostream out{&buf};
/// Error log stream
static ostream err{&errbuf};

/// Indent guard. Works like a lock guard!
struct IndentGuard {
    /// Increases the indent.
    IndentGuard() { indent.inc(); }

    /// Decreases the indent.
    ~IndentGuard() { indent.dec(); }

    /// Should not be copyable.
    IndentGuard(const IndentGuard &) = delete;

    /// Movable
    IndentGuard(IndentGuard &&) = default;
};

#ifndef NDEBUG
/// Only expaned in debug builds.
#define dbg(...) __VA_ARGS__
#define dbg_out(...) ::dbg::out << __VA_ARGS__ << ::std::endl
#else
#define dbg(...)
#define dbg_out(...)
#endif

/// Fatal error
#define fatal_error(msg)                                                       \
    ::dbg::err << "[FATAL ERROR]["                                      \
                      << ::std::experimental::source_location::current()       \
                      << "] " << msg << ::std::endl;                           \
    ::abort();                                                                 \
    ::exit(1)
}; // namespace dbg