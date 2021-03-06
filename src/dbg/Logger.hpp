/// @file Logger.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary
/// You are not allowed to use this file
/// without the author's permission.

#pragma once

#include <iostream>
#include <streambuf>

namespace dbg {
using namespace std;
/// A virtual buffer used to insert a callback function with each '\n'
/// character.
/// @tparam Callback
template <class Callback> class LineCallbackBuf : public streambuf {
    using Base = streambuf;

  protected:
    /// The real buffer, we will send all the output here after checking them
    Base *const destination;
    /// The newline callback
    Callback &callback;
    /// Set to true when the next character is the first in a line.
    bool is_newline{true};

  public:
    /// @param dest Final Destination of the output
    /// @param callback Use this to pass dynamically allocated CallBack
    /// functions.
    constexpr LineCallbackBuf(ostream &dest, Callback &callback) : destination{dest.rdbuf()}, callback{callback} {}

    /// Since we did not define a character, this function will be called for
    /// each character.
    /// @param c
    /// @return int
    int overflow(int c) override {
        if (c == traits_type::eof()) {
            return sync();
        }
        if (is_newline) {
            is_newline = false;
            if (callback.call(*destination) == traits_type::eof()) {
                return traits_type::eof();
            }
            return destination->sputc(c);
        }
        if (c == '\n') {
            is_newline = true;
        }
        return destination->sputc(c);
    }
};

/// Helper class for 'LineCallbackBuf'.
/// Combines multiple callback classes to one.
/// @tparam Callbacks
template <class... Callbacks> class CombinedCallback {
    /// Tuple of callback classes.
    tuple<Callbacks &...> callbacks;

  public:
    /// Use this to initialize the callback classes with arguments.
    /// @param callbacks
    constexpr CombinedCallback(Callbacks &...callbacks) : callbacks(callbacks...) {}

    /// Called when LineCallbackBuf
    /// @param buf
    /// @return int
    int call(streambuf &buf) {
        int ret;
        // What have I done...?
        return apply(
            [&](auto &&...cb) {
                // This is done to silence the warning. Will probably be
                // optimized.
                if ((((void(ret = cb.call(buf)), true) && ret == streambuf::traits_type::eof()) || ...)) {
                    cerr << "Output error." << endl;
                    ::exit(1);
                };
                return ret;
            },
            callbacks);
    }
};

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
    inline void dec() { indent.erase(indent.end() - one_indent.length(), indent.end()); }

    /// This is called to insert indent at every line.
    /// @param buf
    /// @return int
    int call(streambuf &buf) { return buf.sputn(indent.c_str(), indent.size()); }
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
}; // namespace dbg