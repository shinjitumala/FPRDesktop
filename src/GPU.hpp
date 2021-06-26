/// @file GPU.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/Config.hpp>
#include <fprd/Threads.hpp>
#include <fprd/Types.hpp>
#include <fprd/Window.hpp>
#include <fprd/draw/ArcBar.hpp>
#include <fprd/draw/Text.hpp>
#include <fprd/draw/animated/AArcBar.hpp>
#include <fprd/draw/animated/AnimatedList.hpp>
#include <fprd/probes/NVML.hpp>
#include <fprd/util/AnimatedValue.hpp>
#include <fprd/util/ranges.hpp>
#include <fprd/util/to_string.hpp>
#include <numbers>
#include <random>
#include <ranges>

namespace fprd {
class GPU {
  public:
    static constexpr auto max_procs{5};
    using Device = nvml::Device<max_procs>;

    using DynamicData = vector<Device::DynamicData>;
    static constexpr auto probe_interval{1s};

    static constexpr auto circle_radious{128};
    static constexpr Area<float> circle_area{circle_radious * 2, circle_radious * 2};
    static constexpr Area<float> proc_line{theme::small_area(circle_area.w)};

  private:
    /// One widget for each GPU device.
    struct Widget {
        static inline const cairo::Image ico_gpu{resources / "icons/Computer/004-video-card.png", theme::green};
        static inline const cairo::Image ico_mem{resources / "icons/Computer/017-processor.png", theme::white};
        static inline const cairo::Image ico_temp{resources / "icons/Nature/049-thermometer.png", theme::red};
        static inline const cairo::Image ico_fan{resources / "icons/Computer/054-cooler.png", theme::blue};

        Widget(Window &w, const Device &d, Position<float> pos, Area<float> area)
            : d{d}, list{w, pos.offset({0, circle_area.h}), proc_line.scale({1, max_procs + 1})} {
            using namespace ::std::numbers;

            const auto center{pos.offset({circle_radious, circle_radious})};
            const Position<float> outer_edge{(circle_radious + 2) / sqrt2, (circle_radious + 2) / sqrt2};
            const auto value_label{theme::medium_area(circle_radious - outer_edge.x)};
            const Position<float> inner_edge{(circle_radious - 18) / sqrt2, (circle_radious - 18) / sqrt2};
            const Area<float> icon_size{32, 32};

            ArcBarBase abb{
                .center = center,
                .radious = circle_radious - 2,
                .border_width = 1,
                .bar_width = 16,
            };

            TextBase tb{
                .font = &theme::normal,
                .area = value_label,
            };

            abb.start = 1 * pi;
            abb.end = 1.5 * pi;
            usage = {{abb, theme::grey, theme::black, theme::green}};
            tb.pos = value_label.bottom_right(center.offset(outer_edge.scale({-1, -1})));
            usage_percent = {tb, theme::white, theme::black};

            abb.start = 1 * pi;
            abb.end = 0.5 * pi;
            memory_usage = {{abb, theme::grey, theme::black, theme::green}};
            tb.pos = value_label.top_right(center.offset(outer_edge.scale({-1, 1})));
            memory_usage_percent = {tb, theme::white, theme::black};

            abb.start = 0 * pi;
            abb.end = -0.5 * pi;
            temp = {{abb, theme::grey, theme::black, theme::green}};
            tb.pos = value_label.bottom_left(center.offset(outer_edge.scale({1, -1})));
            temp_celsius = {tb, theme::white, theme::black};

            abb.start = 0 * pi;
            abb.end = 0.5 * pi;
            fan = {{abb, theme::grey, theme::black, theme::green}};
            tb.pos = center.offset(outer_edge);
            fan_percent = {tb, theme::white, theme::black};

            const auto gpu_name{theme::large_area((circle_radious - 18) * 2)};
            Text<VerticalAlign::center> t{
                {&theme::bold, gpu_name.center(center), gpu_name},
                theme::green,
            };
            draw_text_once(w, t, d.name);

            tb.area = theme::medium_area(inner_edge.x);
            {
                auto tpos{center.offset(inner_edge.scale({-1, -1}))};
                w.draw(ico_gpu, tpos, icon_size);
                tpos = tpos.stack_bottom(icon_size);
                tb.pos = tpos;
                freq = {tb, theme::white, theme::black};
            }
            {
                auto tpos{center.offset(inner_edge.scale({-1, 1}))};
                w.draw(ico_mem, icon_size.bottom_left(tpos), icon_size);
                tpos = tpos.stack_top(icon_size);
                tb.pos = tb.area.bottom_left(tpos);
                mem_usage = {tb, theme::white, theme::black};
            }
            {
                auto tpos{center.offset(inner_edge.scale({1, -1}))};
                w.draw(ico_temp, icon_size.top_right(tpos), icon_size);
                tpos = tpos.stack_bottom(icon_size);
                tb.pos = tb.area.top_right(tpos);
                watts = {tb, theme::white, theme::black};
            }
            w.draw(ico_fan, icon_size.bottom_right(center.offset(inner_edge.scale({1, 1}))), icon_size);
        }
        const Device &d;

        AnimatedArcBar<ArcBarDirection::clock_wise> usage;
        TextCleared<VerticalAlign::right> usage_percent;
        AnimatedValue<short> freqv;
        TextCleared<VerticalAlign::left> freq;

        AnimatedArcBar<ArcBarDirection::counter_clock_wise> memory_usage;
        TextCleared<VerticalAlign::right> memory_usage_percent;
        AnimatedValue<float> memv;
        TextCleared<VerticalAlign::left> mem_usage;

        AnimatedArcBar<ArcBarDirection::counter_clock_wise> temp;
        TextCleared<VerticalAlign::right> temp_celsius;
        AnimatedValue<float> wattsv;
        TextCleared<VerticalAlign::right> watts;

        AnimatedArcBar<ArcBarDirection::clock_wise> fan;
        TextCleared<VerticalAlign::right> fan_percent;

        AnimatedList<Device::Process, max_procs> list;
    };

    Position<int> pos;
    nvml::NVML nvml;
    vector<Device> devices;
    vector<Widget> widgets;

  public:
    GPU(Position<int> pos) : pos{pos}, nvml{}, devices{nvml.get_devices<max_procs>()} {}

    void update_data(const DynamicData &d) {
        for (auto [data, widget] : zip(d, widgets)) {
            widget.usage.update(data.utilization);
            widget.freqv.update(data.clock);

            widget.memory_usage.update(data.utilization_memory);
            widget.memv.update(data.memory);

            widget.temp.update(data.temp);
            widget.wattsv.update(data.power);

            widget.fan.update(data.fan);

            widget.list.update(data.procs);
        }
    }
    void draw(Window &w, bool new_data) {
        for (auto &widget : widgets) {
            widget.usage.draw(w);
            widget.usage_percent.draw(w, ftos<0>(widget.usage.current_percentage()) + "%");
            widget.freq.draw(w, width<4>(to_string(widget.freqv.draw())) + "MHz");

            widget.memory_usage.draw(w);
            widget.memory_usage_percent.draw(w, ftos<0>(widget.memory_usage.current_percentage()) + "%");
            widget.mem_usage.draw(w, width<5>(ftos<3>(widget.memv.draw())) + "/" + ftos<0>(widget.d.memory_total) +
                                         "GB");

            widget.temp.draw(w);
            widget.temp_celsius.draw(w, ftos<0>(widget.temp.current_percentage()) + "℃");
            widget.watts.draw(w, ftos<1>(widget.wattsv.draw()) + "W");

            widget.fan.draw(w);
            widget.fan_percent.draw(w, ftos<0>(widget.fan.current_percentage()) + "%");
            widget.list.draw(w);
        }
    }

    [[nodiscard]] DynamicData get_data() const {
        DynamicData data;
        for (const auto &d : devices) {
            data.push_back(d.probe_data());
        }
        return data;
    }
    [[nodiscard]] Window create_window() {
        Window w{":0.0", pos,
                 Area<float>{circle_area.w * devices.size(), circle_area.h + theme::small_h * (max_procs + 1)}};

        widgets = [&] {
            vector<Widget> temp;
            for (auto [idx, d] : devices | enumerate) {
                const auto rpos{pos.stack_right({circle_area.scale({idx, 1})})};
                temp.push_back(
                    {w, d, rpos,
                     Area<float>{circle_area.w * devices.size(), circle_area.h + proc_line.h * (max_procs + 1)}});
            }
            return temp;
        }();

        return w;
    }
};

class GPUWindow {
    GPU g;
    Threads<GPU> t;

  public:
    GPUWindow(atomic<bool> &run, Position<int> pos) : g{pos}, t{run, g} {}
};
}; // namespace fprd