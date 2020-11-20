/// @file Nvidia.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/Pattern.hpp>
#include <fprd/Theme.hpp>
#include <fprd/Window.hpp>
#include <fprd/parts/Bar.hpp>
#include <fprd/parts/Text.hpp>
#include <fprd/query/Nvidia.hpp>
#include <fprd/util/to_string.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace fprd {
using namespace std;
namespace widget {
using namespace theme;
template <uint gpu_id> class Nvidia {
  public:
    inline static const float w{330};
    inline static const float h{L2_h + L3_h * 9};

  private:
    inline static const float b_w{w - L3_h * 2};

    query::Nvidia<gpu_id> query;
    Position pos;

    inline static const Image gpu{
        resources / "icons" / "Computer" / "004-video-card.png", green};
    inline static const Image temp_i{
        resources / "icons" / "Nature" / "049-thermometer.png", red};
    inline static const Image power_i{
        resources / "icons" / "Nature" / "032-lightning.png", yellow};
    inline static const Image fan_i{
        resources / "icons" / "Computer" / "054-cooler.png", blue};

    BarSmooth<Color, Color, PatternLinear> util;
    Text<false, TextAlign::center, Color> util_t;

    BarSmooth<Color, Color, PatternLinear> mem;
    Text<false, TextAlign::center, Color> mem_t;

    Text<true, TextAlign::left> temp;
    Text<true, TextAlign::left> power;
    Text<true, TextAlign::left> fan;
    array<Text<true, TextAlign::left>, query::NvidiaProcesses<gpu_id>::max>
        procs;

  public:
    Nvidia(FPRWindow &w, query::Nvidia<gpu_id> &&query, Position pos)
        : query{move(query)}, pos{pos},
          util{Bar{pos + Position{L3_h * 2, L2_h},
                   {b_w, L3_h},
                   L3_m,
                   L3_m,
                   Color{grey},
                   Color{black},
                   PatternLinear{pos + Position{L3_h * 2, L2_h},
                                 {b_w, 0},
                                 {{0, {green}}, {0.5, {yellow}}, {1, {red}}}}},
               100},
          util_t{pos + Position{L3_h * 2, L2_h}, L3_area(b_w), L3_m * 3,
                 noto_sans},
          mem{Bar{pos + Position{L3_h * 2, L2_h + L3_h},
                  {b_w, L3_h},
                  L3_m,
                  L3_m,
                  Color{grey},
                  Color{black},
                  PatternLinear{pos + Position{L3_h * 2, L2_h + L3_h},
                                {b_w, 0},
                                {{0, {green}}, {0.5, {yellow}}, {1, {red}}}}},
              100},
          mem_t{pos + Position{L3_h * 2, L2_h + L3_h}, L3_area(b_w), L3_m * 3,
                noto_sans},
          temp{pos + Position{L3_h * 3, L2_h + L3_h * 2},
               L3_area((Nvidia::w - L3_h * 2) / 3 - L3_h), L3_m * 2, noto_sans},
          power{pos + Position{L3_h * 3 + (Nvidia::w - L3_h * 2) / 3,
                               L2_h + L3_h * 2},
                L3_area((Nvidia::w - L3_h * 2) / 3 - L3_h), L3_m * 2,
                noto_sans},
          fan{pos + Position{L3_h * 3 + (Nvidia::w - L3_h * 2) / 3 * 2,
                             L2_h + L3_h * 2},
              L3_area((Nvidia::w - L3_h * 2) / 3 - L3_h), L3_m * 2, noto_sans},
          procs{
              Text<true, TextAlign::left>{pos + Position{L3_h, L2_h + L3_h * 4},
                                          L3_area(Nvidia::w - L3_h), L3_m * 2,
                                          noto_sans_bold, Color{white}},
              {pos + Position{L3_h, L2_h + L3_h * 5}, L3_area(Nvidia::w - L3_h),
               L3_m * 2, noto_sans_bold, Color{white}},
              {pos + Position{L3_h, L2_h + L3_h * 6}, L3_area(Nvidia::w - L3_h),
               L3_m * 2, noto_sans_bold, Color{white}},
              {pos + Position{L3_h, L2_h + L3_h * 7}, L3_area(Nvidia::w - L3_h),
               L3_m * 2, noto_sans_bold, Color{white}},
              {pos + Position{L3_h, L2_h + L3_h * 8}, L3_area(Nvidia::w - L3_h),
               L3_m * 2, noto_sans_bold, Color{white}}}

    {
        Text<false, TextAlign::left>{pos + Position{L2_h, 0},
                                     L2_area(Nvidia::w), L2_m * 2,
                                     noto_sans_bold, Color{green}}
            .update(w, string{this->query.name});
        w.draw_image(gpu, pos - L2_m, L2_area(L2_h) - L2_m);
        Text<false, TextAlign::center>{pos + Position{L3_h, L2_h},
                                       L3_area(L3_h), L3_m * 2, noto_sans_bold,
                                       Color{white}}
            .update(w, "U");
        Text<false, TextAlign::center>{pos + Position{L3_h, L2_h + L3_h},
                                       L3_area(L3_h), L3_m * 2, noto_sans_bold,
                                       Color{white}}
            .update(w, "M");
        auto last_pos{pos + Position{L3_h, L2_h + L3_h * 2}};
        w.draw_image(temp_i, last_pos + Position{L3_h, 0} - L3_m,
                     L3_area(L3_h) - L3_m);
        w.draw_image(power_i,
                     last_pos + Position{L3_h + (Nvidia::w - L3_h * 2) / 3, 0} -
                         L3_m,
                     L3_area(L3_h) - L3_m);
        w.draw_image(fan_i,
                     last_pos +
                         Position{L3_h + (Nvidia::w - L3_h * 2) / 3 * 2, 0} -
                         L3_m,
                     L3_area(L3_h) - L3_m);

        /// Processes
        ostringstream oss;
        oss << setfill(' ') << setw(5) << right << "PID";
        oss << " ";
        oss << setfill(' ') << setw(4) << right << "Type";
        oss << " ";
        oss << setfill(' ')
            << setw(query::NvidiaProcesses<gpu_id>::max_name_len) << left
            << "Name";
        oss << " ";
        oss << setfill(' ') << setw(9) << right << "Memory";
        Text<false, TextAlign::left>{pos + Position{L3_h, L2_h + L3_h * 3},
                                     L3_area(Nvidia::w - L3_h), L3_m * 2,
                                     noto_sans_bold, Color{white}}
            .update(w, oss.str());
    }

    void update_data() {
        query.update();
        util.update_target(query.utilization);
        mem.update_target(query.memory);
    };

    void draw(FPRWindow &w) {
        util.update(w);
        util_t.update(w, to_string(util.current, 0) + "% (" +
                             std::to_string(query.clock_gpu) + " MHz)");
        mem.update(w);
        mem_t.update(w,
                     to_string(mem.current, 0) + "% (" +
                         to_string(query.memory_total * mem.current / 100, 2) +
                         "/" + to_string(query.memory_total, 2) + " GiB, " +
                         std::to_string(query.clock_mem) + " MHz)");
        "(Fan: " + std::to_string(query.fan) + "%)";

        temp.update(w, " " + std::to_string(query.temp) + " ℃");
        power.update(w, " " + to_string(query.power, 1) + " W");
        fan.update(w, " " + std::to_string(query.fan) + " %");

        for (auto i{0}; i < query::NvidiaProcesses<gpu_id>::max; i++) {
            const auto &p{query.procs.processes[i]};
            if (!p.is_valid()) {
                continue;
            }
            ostringstream oss;
            oss << setfill(' ') << setw(5) << right << p.id;
            oss << " ";
            oss << setfill(' ') << setw(4) << right << p.type;
            oss << " ";
            oss << setfill(' ') << setw(query::NvidiaProcesses<gpu_id>::max_name_len) << left << string{p.name.data()};
            oss << " ";
            oss << setfill(' ') << setw(9) << right
                << (std::to_string(p.memory) + " MiB");
            auto &t{procs[i]};
            t.update(w, oss.str());
        }
    }
};
} // namespace widget
} // namespace fprd