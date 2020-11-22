/// @file fdstreambuf.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <unistd.h>

#include <array>
#include <streambuf>

namespace fprd {
using namespace std;

/// This class is used to turn a filedescriptor to a C++ ifstream.
class fdstreambuf : public streambuf {
    /// Alias for convenience
    using traits_type = streambuf::traits_type;

    /// Buffer size for both input and output.
    static const int bufsize{1024};
    /// Output buffer
    array<char, bufsize> output_buf;
    /// Input buffer
    array<char, bufsize + 16 - sizeof(int)> input_buf;

   protected:
    /// Filedescriptor this is mapped to.
    int file_descriptor;

   public:
    /// @param file_descriptor The file_descriptor this will be representing.
    /// WARNING: fdstreambuf will not take ownership of the 'file_descriptor'.
    /// You still need to close it manually.
    fdstreambuf(int file_descriptor) : file_descriptor{file_descriptor} {
        setg(input_buf.data(), input_buf.data(), input_buf.data());
        setp(output_buf.data(), output_buf.data() + bufsize - 1);
    };

   protected:
    int overflow(int c) override {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(c);
            pbump(1);
        }
        if (sync() == -1) {
            return traits_type::eof();
        }

        return traits_type::not_eof(c);
    };
    int underflow() override {
        if (gptr() == egptr()) {
            auto pback(std::min(gptr() - eback(), ptrdiff_t(16 - sizeof(int))));
            copy(egptr() - pback, egptr(), eback());
            auto read_size(::read(file_descriptor, eback() + pback, bufsize));
            setg(eback(), eback() + pback,
                 eback() + pback + max(0L, read_size));
        }

        if (gptr() == egptr()) {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*gptr());
    };
    int sync() override {
        while (pbase() != pptr()) {
            auto data_size{pptr() - pbase()};
            auto sent_size{
                ::write(file_descriptor, output_buf.data(), data_size)};

            if (0 < sent_size) {
                copy(pbase() + sent_size, pptr(), pbase());
                setp(pbase(), epptr());
                pbump(data_size - sent_size);
            }
        }

        if (pptr() == epptr()) {
            return -1;
        }
        return 0;
    };
};
};  // namespace fprd