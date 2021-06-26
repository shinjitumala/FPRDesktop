/// @file Nvidia.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/NVML.hpp>
#include <fprd/Pattern.hpp>
#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/parts/ArcBar.hpp>
#include <fprd/parts/Text.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/util/to_string.hpp>
#include <iomanip>
#include <numbers>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

namespace fprd {
using namespace std;
using namespace ::std::numbers;
namespace widget {
using namespace theme;
class Nvidia {
  public:
    static constexpr Area<float> size{312, 312 + L3_h *(1 + nvml::Device::max_procs)};

  private:
    nvml::Device &d;
    nvml::Device::DynamicData data;

    inline static const Image gpu{resources / "icons" / "Computer" / "004-video-card.png", green};
    inline static const Image mem_i{resources / "icons" / "Computer" / "017-processor.png", white};
    inline static const Image temp_i{resources / "icons" / "Nature" / "049-thermometer.png", red};
    inline static const Image power_i{resources / "icons" / "Nature" / "032-lightning.png", yellow};
    inline static const Image fan_i{resources / "icons" / "Computer" / "054-cooler.png", blue};

    ArcBar<true> util;
    Text<VerticalAlign::right> util_t;
    DynamicText<VerticalAlign::left> clock;
    ushort current_clock{};

    ArcBar<false> mem;
    Text<VerticalAlign::right> mem_t;
    DynamicText<VerticalAlign::left> mem_t2;

    ArcBar<false> temp;
    Text<VerticalAlign::right> temp_t;
    ArcBar<true> fan;
    Text<VerticalAlign::right> fan_t;

    DynamicText<VerticalAlign::right> power;

    array<DynamicText<VerticalAlign::left>, nvml::Device::max_procs> procs;
    inline static const size_t max_name_len{20};

    float current_power{};
    float current_memory{};

  public:
    Nvidia(FPRWindow &w, nvml::Device &d, Position<float> pos) : d{d} {
        constexpr auto arc_bar_width{50};
        constexpr Margin<float> arc_bar_m{L3_m};
        TextBase text_base{
            .font = &noto_sans,
            .pos = pos,
        };
        Text<VerticalAlign::center> t{text_base};
        t.area = {size.w - arc_bar_width * 2, 50};
        t.pos = t.area.center(pos.offset({size.w / 2, size.w / 2}));
        t.fg = green;
        t.update(w, d.name.substr(8));

        // ArcBars and their text indicators
        ArcBarBase arc{
            .center = pos.offset({size.w / 2, size.w / 2}),
            .radious = size.w / 2,
            .border_width = 3,
            .bar_width = arc_bar_width,
            .border = grey,
            .bg = black,
            .fg = green,
            .current = 0,
            .target = 0,
        };

        t.fg = white;
        const auto arc_text_w{arc_bar_width - 2 * arc.border_width};
        t.area = Area<float>(arc_text_w, 24).pad(arc_bar_m);

        t.pos = t.area.bottom_right(pos.offset({size.w * 0.25F, size.w * 0.25F}));
        util_t = t;

        t.pos = t.area.top_right(pos.offset({size.w * 0.25F, size.w * 0.75F}));
        mem_t = t;

        t.pos = t.area.bottom_left(pos.offset({size.w * 0.75F, size.w * 0.25F}));
        temp_t = t;

        t.pos = pos.offset({size.w * 0.75F, size.w * 0.75F});
        fan_t = t;

        arc.start = pi * 1;
        arc.end = pi * 1.5;
        util = {arc};

        arc.start = pi * 1;
        arc.end = pi * 0.5;
        mem = {arc};

        arc.start = 0;
        arc.end = -pi * 0.5;
        temp = {arc};

        arc.start = 0;
        arc.end = pi * 0.5;
        fan = {arc};

        // Icons
        const Area<float> icon_area{size.w * 0.125, size.w * 0.125};
        w.draw_image(gpu, icon_area.bottom_right(pos.offset({size.w * 0.375, size.w * 0.375})), icon_area);
        w.draw_image(mem_i, icon_area.top_right(pos.offset({size.w * 0.375, size.w * 0.625})), icon_area);
        w.draw_image(temp_i, icon_area.bottom_left(pos.offset({size.w * 0.625, size.w * 0.375})), icon_area);
        w.draw_image(fan_i, pos.offset({size.w * 0.625, size.w * 0.625}), icon_area);

        // Other data
        DynamicText<VerticalAlign::left> dt;
        dt.font = &noto_sans;
        dt.fg = white;
        dt.bg = black;
        dt.area = L3_area(size.w * 0.25);

        dt.pos = pos.offset({size.w * 0.25, size.w * 0.375});
        clock = dt;

        dt.pos = dt.area.bottom_left(pos.offset({size.w * 0.25, size.w * 0.625}));
        mem_t2 = dt;

        dt.pos = dt.area.top_right(pos.offset({size.w * 0.75, size.w * 0.375}));
        power = dt;

        // Processes
        DynamicText<VerticalAlign::left> dtl{};
        dtl.font = &noto_sans_bold;
        dtl.pos = pos.offset({0, size.w});
        dtl.area = L3_area(size.w);
        dtl.fg = white;
        dtl.bg = black;

        dtl.update(w, []() {
            ostringstream oss;
            oss << setfill(' ') << setw(5) << right << "PID";
            oss << " ";
            oss << setfill(' ') << setw(1) << right << "T";
            oss << " ";
            oss << setfill(' ') << setw(max_name_len) << left << "Name";
            oss << " ";
            oss << setfill(' ') << setw(7) << right << "Memory";
            return oss.str();
        }());

        dtl.font = &noto_sans;
        for (auto &proc : procs) {
            dtl.pos = dtl.pos.offset({0, L3_h});
            proc = dtl;
        }
    }

    void draw(FPRWindow &w) {
        if (auto [updated, new_data]{d.get_dynamic_data()}; updated) {
            // Only run when there is new data.
            data = new_data; // Copy the new data.

            util.target = data.utilization;
            mem.target = data.utilization_memory;
            temp.target = data.temp;
            fan.target = data.fan;

            for (const auto [data, proc] : zip(data.procs, procs)) {
                const auto s{[&p = data]() {
                    ostringstream oss;
                    oss << setfill(' ') << setw(5) << right << p.t.pid;
                    oss << " ";
                    oss << setfill(' ') << setw(1) << right << "G";
                    oss << " ";
                    oss << setfill(' ') << setw(max_name_len) << left << [&]() {
                        if (p.name.size() < max_name_len) {
                            return p.name;
                        }
                        return p.name.substr(0, max_name_len - 3) + "...";
                    }();
                    oss << " ";
                    oss << setfill(' ') << setw(7) << right << (ftos<0>((float)p.t.usedGpuMemory * 1e-6) + "MB");
                    return oss.str();
                }()};
                proc.update(w, s.c_str());
            }
            using namespace ::std::ranges::views;
            const auto procs_size{procs.size()};
            const auto all_procs_size{data.procs.size()};
            if (all_procs_size < procs_size) {
                for (const auto &proc : procs | views::reverse | take(procs_size - all_procs_size)) {
                    proc.update(w, "");
                }
            }
        }

        util.draw(w);
        mem.draw(w);
        temp.draw(w);
        fan.draw(w);

        current_power = slow_update(current_power, data.power);
        current_clock = slow_update(current_clock, data.clock);
        current_memory = slow_update(current_memory, data.memory);

        util_t.update(w, ftos<0>(util.current) + "%");
        mem_t.update(w, ftos<0>(mem.current) + "%");
        temp_t.update(w, ftos<0>(temp.current) + "℃");
        fan_t.update(w, ftos<0>(fan.current) + "%");

        mem_t2.update(w, ftos<2>(current_memory) + "/" + ftos<0>(d.memory_total) + "GB");

        power.update(w, ftos<2>(current_power) + "W");
        clock.update(w, width<7>(to_string(current_clock) + "MHz"));
    }
};
} // namespace widget
} // namespace fprd