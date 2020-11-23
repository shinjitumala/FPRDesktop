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
#include <fprd/util/to_string.hpp>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

namespace fprd {
using namespace std;
namespace widget {
using namespace theme;
class Nvidia {
   public:
    inline static const float w{256};
    inline static const float h{w + L3_h * (1 + nvml::Device::max_procs)};

   private:
    inline static const float b_w{w - L3_h * 2};

    nvml::Device &d;

    inline static const Image gpu{
        resources / "icons" / "Computer" / "004-video-card.png", green};
    inline static const Image mem_i{
        resources / "icons" / "Computer" / "017-processor.png", white};
    inline static const Image temp_i{
        resources / "icons" / "Nature" / "049-thermometer.png", red};
    inline static const Image power_i{
        resources / "icons" / "Nature" / "032-lightning.png", yellow};
    inline static const Image fan_i{
        resources / "icons" / "Computer" / "054-cooler.png", blue};

    ArcBar<true> util;
    Text<true, TextAlign::left> util_t;
    Text<true, TextAlign::left> clock;
    ushort current_clock{};

    ArcBar<false> mem;
    Text<true, TextAlign::left> mem_t;
    Text<true, TextAlign::left> mem_t2;

    ArcBar<false> temp;
    Text<true, TextAlign::right> temp_t;
    ArcBar<true> fan;
    Text<true, TextAlign::right> fan_t;

    Text<true, TextAlign::right> power;

    array<Text<true, TextAlign::left>, nvml::Device::max_procs> procs;
    inline static const size_t max_name_len{20};

    float current_power{};
    float current_memory{};

   public:
    Nvidia(FPRWindow &win, nvml::Device &d, Position<float> pos) : d{d} {
        {
            Text<false, TextAlign::center> t;
            t.move_to(pos + Position<float>{0, w / 2 - L2_h / 2}, L2_area(w),
                      L2_m);
            t.set_font(noto_sans_bold, green, black);
            t.update(win, d.name.substr(8, 10));
        }
        const auto center{pos + Position<float>{w / 2, w / 2}};
        const auto quarter{w / 4 + 10};
        {
            Text<true, TextAlign::left> l;
            const auto area{L2_area(quarter)};
            l.set_font(noto_sans, white, black);
            l.move_to(center + Position<float>{-quarter, -quarter}, area, L3_m);
            util_t = l;

            l.move_to((center + Position<float>{-quarter, quarter}).bl(area),
                      area, L3_m);
            mem_t = l;

            Text<true, TextAlign::right> r;
            r.set_font(noto_sans, white, black);
            r.move_to((center + Position<float>{quarter, -quarter}).tr(area),
                      area, L3_m);
            temp_t = r;
            r.move_to((center + Position<float>{quarter, quarter}).br(area),
                      area, L3_m);
            fan_t = r;
        }
        {
            Text<true, TextAlign::left> t;
            const auto area{L3_area(100)};
            t.set_font(noto_sans, white, black);
            t.move_to(
                (center + Position<float>{-quarter, quarter - L2_h}).bl(area),
                area, L3_m);
            mem_t2 = t;

            const auto area2{L3_area(quarter)};
            t.move_to((center + Position<float>{-quarter, -quarter + L2_h}),
                      area2, L3_m);
            clock = t;
        }
        {
            Text<true, TextAlign::right> t;
            const auto area{L3_area(100)};
            t.set_font(noto_sans, white, black);

            t.move_to(
                (center + Position<float>{quarter, -quarter + L2_h}).tr(area),
                area, L3_m);
            power = t;
        }
        Size<float> icon{Size<float>{36, 36}};

        win.draw_image(gpu, pos, icon.pad(L2_m));
        win.draw_image(mem_i, (pos + Position<float>{0, w}).bl(icon),
                       icon.pad(L2_m));
        win.draw_image(fan_i, (pos + Position<float>{w, w}).br(icon),
                       icon.pad(L2_m));
        win.draw_image(temp_i, (pos + Position<float>{w, 0}).tr(icon),
                       icon.pad(L2_m));

        util.center = center;
        util.radious = w / 2;
        util.start = 3.14 * 1;
        util.end = 3.14 * 1.5;
        util.border_width = 3;
        util.bar_width = 20;
        util.bg = black;
        util.fg = green;
        util.border = grey;

        mem = util;
        mem.start = 3.14 * 1;
        mem.end = 3.14 * 0.5;

        temp = util;
        temp.start = 3.14 * 0;
        temp.end = 3.14 * -0.5;

        fan = util;
        fan.start = 3.14 * 0;
        fan.end = 3.14 * 0.5;

        // Processes
        {
            Text<true, TextAlign::left> t;

            ostringstream oss;
            oss << setfill(' ') << setw(5) << right << "PID";
            oss << " ";
            oss << setfill(' ') << setw(1) << right << "T";
            oss << " ";
            oss << setfill(' ') << setw(max_name_len) << left << "Name";
            oss << " ";
            oss << setfill(' ') << setw(7) << right << "Memory";

            t.set_font(noto_sans_bold, Color{white});
            t.move_to(pos + Position<float>{0, w}, L3_area(w), L3_m);
            t.update(win, oss.str());

            t.set_font(noto_sans, Color{white});
            for (auto i :
                 ranges::iota_view{0, static_cast<int>(procs.size())}) {
                t.move_to(pos + Position<float>{0, w + L3_h * (float)(i + 1)},
                          L3_area(w), L3_m);
                procs[i] = t;
            }
        }
    }

    void start_update() { d.start_update(); };

    void draw(FPRWindow &w) {
        const auto &data{d.data};
        if (d.check_update()) {
            util.target = (float)data.utilization / 100;
            mem.target = (float)data.utilization_memory / 100;
            temp.target = (float)data.temp / 100;
            fan.target = (float)data.fan / 100;

            for (auto i{0U}; i < procs.size(); i++) {
                const auto s{[&]() -> string {
                    if (i < data.procs.size()) {
                        const auto &p{data.procs.at(i)};

                        ostringstream oss;
                        oss << setfill(' ') << setw(5) << right << p.t.pid;
                        oss << " ";
                        oss << setfill(' ') << setw(1) << right << "G";
                        oss << " ";
                        oss << setfill(' ') << setw(max_name_len) << left
                            << [&]() {
                                   if (p.name.size() < max_name_len) {
                                       return p.name;
                                   }
                                   return p.name.substr(0, max_name_len - 3) +
                                          "...";
                               }();
                        oss << " ";
                        oss << setfill(' ') << setw(7) << right
                            << (to_string((float)p.t.usedGpuMemory * 1e-6, 0) + "MB");
                        return oss.str();
                    }
                    return "";
                }()};
                procs[i].update(w, s);
            }
        }

        util.draw(w);
        mem.draw(w);
        temp.draw(w);
        fan.draw(w);
        current_power += (data.power - current_power) / 80;
        current_clock += (data.clock - current_clock) / 80;
        current_memory += ((float)data.memory - current_memory) / 80;

        util_t.update(w, to_string(util.current * 100, 0) + "%");
        mem_t2.update(
            w, to_string((float)current_memory * d.memory_total / 100, 3) +
                   "/" + to_string(d.memory_total, 0) + "GB");

        mem_t.update(w, to_string(mem.current * 100, 0) + "%");
        temp_t.update(w, to_string(temp.current * 100, 0) + "℃");
        fan_t.update(w, to_string(fan.current * 100, 0) + "%");
        power.update(w, to_string(current_power, 2) + "W");
        clock.update(w, std::to_string(current_clock) + "MHz");
    }
};
}  // namespace widget
}  // namespace fprd