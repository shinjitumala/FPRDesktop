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
#include <fprd/parts/History.hpp>
#include <fprd/parts/Text.hpp>
#include <fprd/query/CPU.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/values/animated.hpp>

namespace fprd {
using namespace std;
namespace widget {
using namespace theme;
class CPU {
    inline static const auto thread_bar_h{L3_h * 1.5F};
    inline static const auto center_h{thread_bar_h * 4 + L2_h + L3_h * 2};
    inline static const auto max_name_len{16};
    inline static const auto thread_count{query::CPU::thread_count};

   public:
    inline static const Area<float> size{
        350, thread_bar_h * 4 + center_h + L3_h*(query::CPU::max_procs + 1)};

   private:
    query::CPU& query;
    query::CPU::DynamicData data;

    array<Bar<Orientation::horizontal, Direction::positive>, thread_count>
        thread_usage_bars;
    array<AnimatedValue<float>, thread_count> thread_usages;
    array<Text<VerticalAlign::center>, thread_count> thread_freq_texts;
    array<AnimatedValue<float>, thread_count> thread_freqs;

    array<DynamicText<VerticalAlign::left>, query::CPU::max_procs> procs;

    History<float> usage;
    History<float> memory;

   public:
    CPU(FPRWindow& win, query::CPU& query, const Position<float> pos)
        : query{query} {
        const auto quarter_thread{query::CPU::thread_count / 4};
        const Area<float> core_bar_size{(float)size.w / quarter_thread,
                                        thread_bar_h};

        Bar<Orientation::horizontal, Direction::positive> b{
            .border_width = 2,
            .frame = grey,
            .empty = black,
            .filled = green,
        };

        /// Initialize thread frequency indicators.
        Text<VerticalAlign::center> t;
        t.font = &noto_sans;
        t.fg = white;
        t.area = L3_area(core_bar_size.w);
        for (auto i{0}, x{0}, y{0}; i < query::CPU::thread_count;
             i++, x++, x %= quarter_thread, y = i / 4) {
            b.pos = pos.stack(core_bar_size.scale({x, y})).pad(L3_m);
            if (2 <= y) {
                b.pos = b.pos.offset({0, center_h});
            }
            thread_usage_bars[i] = b;

            t.pos = t.area.vertical_center(b.area.vertical_center(b.pos));
            thread_freq_texts[i] = t;
        }

        /// Draw the CPU name
        t.font = &noto_sans_bold;
        t.fg = red;
        t.area = L3_area(size.w);
        t.pos = t.area.vertical_center(
            pos.offset(Position<double>{0, thread_bar_h * 2 + center_h * 0.5}));
        t.update(win, query.name);

        /// Draw process list header
        t.fg = white;
        t.font = &noto_sans_bold;
        t.pos = pos.offset({0, thread_bar_h * 4.0F + center_h});
        t.area = L3_area(size.w);
        const auto header{[]() {
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
            return oss.str();
        }()};
        t.update(win, header);

        /// Draw process list item.
        DynamicText<VerticalAlign::left> dt{{t}};
        dt.font = &noto_sans;
        dt.fg = white;
        dt.bg = black;
        for (auto& proc : procs) {
            dt.pos = dt.pos.stack_bottom(L3_area(size.w));
            proc = dt;
        }

        usage.area = Area<float>{size.w, thread_bar_h * 2}.pad(L3_m);
        usage.pos = pos.offset({0, thread_bar_h * 2}).pad(L3_m);
        usage.fg = green;
        usage.bg = black;
        usage.b = grey;
        usage.border_width = 2;

        memory = usage;
        memory.fg = blue;
        memory.pos = pos.offset({0, center_h}).pad(L3_m);
    }

    void draw(FPRWindow& w) {
        if (auto [updated, new_data]{query.get_dynamic_data()}; updated) {
            // Only run when there is new data.
            data = new_data;  // Copy the new data.

            // Update history
            usage.update(w, data.avg.usage);
            memory.update(w, 0);

            for (auto [b, v, td] :
                 zip(thread_usage_bars, thread_usages, data.threads)) {
                b.draw(w, v);
            }
            for (auto [p, pd] : zip(procs, data.procs)) {
                if (!pd.name.empty()) {
                    ostringstream oss;
                    oss << setfill(' ') << setw(6) << right << pd.pid;
                    oss << " ";
                    oss << setfill(' ') << setw(1) << right << pd.mode;
                    oss << " ";
                    oss << setfill(' ') << setw(max_name_len) << left <<
                        [&pd = pd]() {
                            if (pd.name.size() < max_name_len) {
                                return pd.name;
                            }
                            return pd.name.substr(0, max_name_len - 3) + "...";
                        }();
                    oss << " ";
                    oss << setfill(' ') << setw(8) << right
                        << (ftos<2>(pd.usage * 100) + "%");
                    oss << " ";
                    oss << setfill(' ') << setw(7) << right
                        << (to_string(pd.mem) + "MB");
                    p.update(w, oss.str());
                } else {
                    p.update(w, "");
                }
            }
        }
        for (auto [t, thz, td, chz] : zip(thread_usage_bars, threads_hz,
                                          data.threads, threads_current_hz)) {
            t.update(w);
            chz = slow_update(chz, td.freq);
            thz.update(w, ftos<0>(chz) + "MHz");
        }
    }
};
};  // namespace widget
};  // namespace fprd