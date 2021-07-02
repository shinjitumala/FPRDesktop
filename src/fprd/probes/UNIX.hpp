/// @file System.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <dbg/Log.hpp>
#include <fprd/util/format.hpp>
#include <fstream>
#include <istream>
#include <string>

namespace fprd {
using namespace std;

template <class Storage> auto get_name(pid_t pid, Storage &dst) -> void {
    static array<char, 32> buf;

    snprintf(buf, "/proc/%d/comm", pid);
    ifstream is{buf.data()};
    copy(istream_iterator<char>(is), istream_iterator<char>(), back_inserter(dst));
}
}; // namespace fprd