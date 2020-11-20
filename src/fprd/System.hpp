/// @file System.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <cstdio>
#include <cstdlib>
#include <dbg/Log.hpp>
#include <fprd/util/fdstreambuf.hpp>
#include <istream>
#include <string>

namespace fprd {
using namespace std;

class Executor : protected fdstreambuf, protected istream {
    FILE *f;

  public:
    Executor(string_view command)
        : Executor{command, ::popen(command.data(), "r")} {}

    ~Executor() override { ::pclose(f); }

    using istream::operator bool;

  protected:
    auto get_line() -> string {
        if (istream::operator bool() == false) {
            return "";
        }
        string s;
        std::getline(static_cast<istream &>(*this), s);
        return s;
    }

  private:
    Executor(string_view command, FILE *f)
        : fdstreambuf{::fileno(f)}, istream{static_cast<fdstreambuf *>(this)},
          f{f} {}
};

class ExecutorLine : public Executor {
    int current_line;

  public:
    ExecutorLine(string_view command) : Executor(command), current_line{0} {}

    /// @param line_num Line number starts from 1.
    /// @return string
    string get_line(int line_num) {
        dbg(if (line_num <= current_line) {
            fatal_error("Get line must be called in order.");
        });
        skip_lines(line_num - current_line - 1);
        current_line++;
        return Executor::get_line();
    }

  private:
    void skip_lines(uint lines) {
        current_line += lines;
        for (auto i{0U}; i < lines; i++) {
            static_cast<istream *>(this)->ignore(
                numeric_limits<streamsize>::max(), '\n');
            if (istream::operator bool() == false) {
                break;
            }
        }
    }
};

class ExecutorCSV : public Executor, public vector<string> {
  public:
    ExecutorCSV(string_view command) : Executor(command) {
        const auto str{get_line()};

        const auto delim{','};

        for (size_t end{0}, start;
             (start = str.find_first_not_of(delim, end)) != string::npos;) {
            if (!empty()) {
                start++;
            }
            end = str.find(delim, start);
            push_back(str.substr(start, end - start));
        }
    }
};
}; // namespace fprd