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
#include <fstream>
#include <istream>
#include <string>

namespace fprd {
using namespace std;

string get_name(pid_t pid) {
    using namespace std::filesystem;
    const path file{"/proc/" + to_string(pid) + "/stat"};
    if (!exists(file)) {
        return "<E: Missing file>";
    }
    ifstream ifs{file};
    ifs.ignore(numeric_limits<streamsize>::max(), '(');
    return [&]() {
        string name;
        for (char c; ifs >> c, c != ')';) {
            name += c;
        }
        return name;
    }();
}
};  // namespace fprd