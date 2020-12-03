/// @file CPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/parts/Bar.hpp>
#include <fprd/parts/Text.hpp>
#include <fprd/query/CPU.hpp>

namespace fprd {
using namespace std;
namespace widget {
using namespace theme;
class CPU {
    inline static const uint thread_bar_h{64};
    inline static const uint center_h{200};
    inline static const uint max_name_len{16};

   public:
    inline static const Area<uint> size{
        300, static_cast<unsigned int>(thread_bar_h * 4 + center_h +
                                       L3_h * (query::CPU::max_procs + 1))};

   private:
    query::CPU& query;

    vector<BarSmooth<true, Color, Color, Color>> threads;
    vector<Text<false, TextAlign::center>> threads_t;
    vector<Text<false, TextAlign::center>> threads_hz;
    vector<float> threads_current_hz;

    vector<Text<true, TextAlign::left>> procs;

   public:
    CPU(FPRWindow& win, query::CPU& query, Position<float> pos)
        : query{query},
          threads_current_hz(query.thread_count),
          procs(query.thread_count) {
        const auto quarter_thread{query.thread_count / 4};
        const Area<float> core_bar_size{(float)size.w / quarter_thread,
                                        thread_bar_h};
        const Area<float> core_bar_text_size{L3_area(core_bar_size.w)};

        BarSmooth<true, Color, Color, Color> b{};
        b.smoothness = 80;
        b.set_colors(grey, black, green);
        Text<false, TextAlign::center> t;
        t.set_font(noto_sans, white);
        Text<false, TextAlign::center> t2;
        t2.set_font(noto_sans, white);

        for (auto i{0}, x{0}, y{0}; i < query.thread_count;
             i++, x++, x %= quarter_thread, y = i / 4) {
            if (y < 2) {
                b.move_to(
                    {(float)core_bar_size.w * x, (float)core_bar_size.h * y},
                    core_bar_size, L3_m, L3_m);
                t.move_to(
                    {(float)core_bar_text_size.w * x,
                     static_cast<float>((float)core_bar_size.h * (y + 0.5F) -
                                        core_bar_text_size.h * 0.5F)},
                    core_bar_text_size, L3_m);
                t2.move_to(
                    {(float)core_bar_text_size.w * x,
                     static_cast<float>((float)core_bar_size.h * (y + 0.5F) +
                                        core_bar_text_size.h * 0.5F)},
                    core_bar_text_size, L3_m);
            } else {
                b.move_to({(float)core_bar_size.w * x,
                           (float)core_bar_size.h * y + center_h},
                          core_bar_size, L3_m, L3_m);
                t.move_to(
                    {(float)core_bar_text_size.w * x,
                     static_cast<float>((float)core_bar_size.h * (y + 0.5F) -
                                        core_bar_text_size.h * 0.5F) +
                         center_h},
                    core_bar_text_size, L3_m);
                t2.move_to(
                    {(float)core_bar_text_size.w * x,
                     static_cast<float>((float)core_bar_size.h * (y + 0.5F) +
                                        core_bar_text_size.h * 0.5F) +
                         center_h},
                    core_bar_text_size, L3_m);
            }
            b.target = 50;
            threads.push_back(b);
            threads_t.push_back(t);
            threads_hz.push_back(t2);
        }

        t.set_font(noto_sans_bold, red);
        t.move_to(pos + Position<float>{0, (float)thread_bar_h * 2.0F +
                                               center_h * 0.5F - L2_h * 0.5F},
                  {L2_area(size.w)}, L2_m);
        t.update(win, query.name);

        t.set_font(noto_sans_bold, white);
        ostringstream oss;
        oss << setfill(' ') << setw(6) << right << "PID";
        oss << " ";
        oss << setfill(' ') << setw(1) << right << "T";
        oss << " ";
        oss << setfill(' ') << setw(max_name_len) << left << "Name";
        oss << " ";
        oss << setfill(' ') << setw(8) << right << "Usage";
        oss << " ";
        oss << setfill(' ') << setw(7) << right << "Memory";

        Text<true, TextAlign::left> t3;
        t3.set_font(noto_sans_bold, white);
        t3.move_to(pos + Position<float>{0, thread_bar_h * 4.0F + center_h},
                   L3_area(size.w), L3_m);
        t3.update(win, oss.str());

        t3.set_font(noto_sans, white);
        for (auto i{0U}; i < procs.size(); i++) {
            t3.move_to(pos + Position<float>{0, thread_bar_h * 4.0F + center_h +
                                                    L3_h * (float)(i + 1)},
                       L3_area(size.w), L3_m);
            procs[i] = t3;
        }
    }

    void start_update() { query.start_update(); }

    void draw(FPRWindow& w) {
        if (query.check_update()) {
            for (auto i{0U}; i < query.thread_count; i++) {
                threads[i].update_target(query.data.threads[i].usage * 100);
            }
            for (auto i{0U}; i < procs.size(); i++) {
                if (i < query.data.procs.size()) {
                    const auto& p{query.data.procs[i]};
                    ostringstream oss;
                    oss << setfill(' ') << setw(6) << right << p.pid;
                    oss << " ";
                    oss << setfill(' ') << setw(1) << right << p.mode;
                    oss << " ";
                    oss << setfill(' ') << setw(max_name_len) << left << [&]() {
                        if (p.name.size() < max_name_len) {
                            return p.name;
                        }
                        return p.name.substr(0, max_name_len - 3) + "...";
                    }();
                    oss << " ";
                    oss << setfill(' ') << setw(8) << right
                        << (ftos<2>(p.usage * 100) + "%");
                    oss << " ";
                    oss << setfill(' ') << setw(7) << right
                        << (ftos<3>(p.mem) + "GB");
                    procs[i].update(w, oss.str());
                } else {
                    procs[i].update(w, "");
                }
            }
        }
        ushort i{0};
        for_each(threads.begin(), threads.end(), [&](auto& t) {
            t.update(w);
            threads_t[i].update(w, to_string(t.current, 2) + "%");

            threads_current_hz[i] +=
                (query.data.threads[i].freq - threads_current_hz[i]) / 80;
            threads_hz[i].update(w, ftos<2>(threads_current_hz[i]) + "GHz");
            i++;
        });
    }
};
};  // namespace widget
};  // namespace fprd