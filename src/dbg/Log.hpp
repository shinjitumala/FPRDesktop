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
#include <experimental/source_location>
#include <fprd/util/ostream.hpp>
#include <iostream>
#include <ostream>

namespace dbg {
using namespace std;

#ifndef NDEBUG
/// Only expaned in debug builds.
#define dbg(...) __VA_ARGS__
#define dbg_out(...)                                                     \
    ::dbg::out << "[" << ::std::experimental::source_location::current() \
               << "] " << __VA_ARGS__ << ::std::endl
#else
#define dbg(...)
#define dbg_out(...)
#endif

/// Fatal error
#define fatal_error(msg)                                                  \
    ::dbg::err << "[FATAL ERROR]["                                        \
               << ::std::experimental::source_location::current() << "] " \
               << msg << ::std::endl;                                     \
    ::abort();                                                            \
    ::exit(1)

/// Get the current time.
/// @return auto
auto now() {
    using namespace ::std::chrono;
    return time_point<high_resolution_clock>::clock::now();
}

/// Gets time difference in milliseconds.
/// @param tp
/// @return auto
auto diff(chrono::time_point<chrono::high_resolution_clock> &tp) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(now() - tp).count();
}
};  // namespace dbg