/// @file AnimatedList.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <fprd/draw/Text.hpp>
#include <fprd/util/ranges.hpp>
#include <functional>

namespace fprd {

/// The concept that an Item in a list must satisfy.
/// @tparam T
template <class T>
concept list_item = requires(const T& t) {
    { T::header() } -> same_as<string>;
    { t == t } -> convertible_to<bool>;
}
&&Printable<T>;

/// Animated list.
/// It shows a list of items that are sorted.
/// For example, it can be used to show processes sorted by memory usage.
/// @tparam Item
/// @tparam max_items
template <list_item Item, size_t max_items>
class AnimatedList {
    using Data = vector<Item>;

    /// Used to create the animation for the next second.
    struct Diff {
        struct Movement {
            size_t new_idx;
            size_t prev_idx;
        };

        vector<size_t> appeared;
        vector<size_t> disappeared;
        vector<Movement> moved;
    };

    /// A animated text in the list.
    struct ItemText {
        Text<VerticalAlign::left> drawer;  // Drawn text object.
        string text;                       // The shown text.
        float vertical_motion;             // The vertical movement per frame.
        float fading;  // Negative means deleted in the next update.
    };

    /// Template text for list items.
    const Text<VerticalAlign::left> item_template;

    /// Position of this list.
    Position<float> pos;
    /// Area for this list.
    Area<float> area;
    /// All the currently animated texts.
    vector<ItemText> items;
    /// Recored to generate the animation.
    Data prev;

   public:
    /// Constructor. The window is needed to draw the header.
    /// @param w
    /// @param pos
    /// @param area
    AnimatedList(Window& w, Position<float> pos, Area<float> area)
        : item_template{[area]() -> Text<VerticalAlign::left> {
              const auto line_area{area.scale({1, 1.0F / (max_items + 1)})};
              return {{&theme::normal, {}, line_area}, theme::white};
          }()},
          pos{pos},
          area{area},
          prev{} {
        // Header
        auto t{item_template};
        t.font = &theme::bold;
        t.pos = pos;
        draw_text_once(w, t, Item::header());
    }

    /// No copying.
    AnimatedList(const AnimatedList&) = delete;
    /// Moving is allowed, however.
    AnimatedList(AnimatedList&&) noexcept = default;

    /// Update the target percentage.
    /// Currently hard-coded such that this function must be called every second
    /// or it breaks.
    /// TODO: Make the update interval adjustable via template parameters.
    /// @param new_data
    void update(const Data& new_data) {
        if (new_data.size() > max_items) {
            fatal_error("New data larger than expected: "
                        << new_data.size() << " (Expected: " << max_items
                        << ")");
        }

        // Remove everything.
        items.clear();

        // Re-create the animation from the diff.
        const auto d{diff(new_data, prev)};
        for (auto i : d.appeared) {
            items.push_back(
                create_text_item(new_data.at(i), max_items, i, 1.0F / fps));
        }
        for (auto i : d.disappeared) {
            items.push_back(
                create_text_item(prev.at(i), i, max_items, -1.0F / fps));
        }
        for (auto [e, s] : d.moved) {
            items.push_back(create_text_item(new_data.at(e), s, e, 0));
        }

        prev = new_data;
    }

    /// Call this every frame.
    /// @param w
    void draw(Window& w) {
        w.rectangle(pos.stack_bottom(item_template.area),
                    item_template.area.scale({1, max_items}));
        w.set_source(theme::black);
        w.fill();

        for (auto& i : items) {
            i.drawer.pos.y += i.vertical_motion;
            i.drawer.fg.a += i.fading;
            i.drawer.draw(w, i.text);
        }
    }

   private:
    /// Utility function for computing the current Diff.
    /// @param new_data
    /// @param prev
    /// @return auto
    static auto diff(const Data& new_data, const Data& prev) {
        const auto appeared{[&] {
            vector<size_t> temp;
            for (auto [idx, nd] : new_data | enumerate) {
                if (find(prev.begin(), prev.end(), nd) == prev.end()) {
                    temp.push_back(idx);
                }
            }
            return temp;
        }()};
        const auto disappeared{[&] {
            vector<size_t> temp;
            for (auto [idx, pd] : prev | enumerate) {
                if (find(new_data.begin(), new_data.end(), pd) ==
                    new_data.end()) {
                    temp.push_back(idx);
                }
            }
            return temp;
        }()};
        const auto movement{[&] {
            vector<typename Diff::Movement> temp;
            for (auto [idx, nd] : new_data | enumerate) {
                if (auto itr{find(prev.begin(), prev.end(), nd)};
                    itr != prev.end()) {
                    temp.push_back(
                        {idx, static_cast<size_t>(itr - prev.begin())});
                }
            }
            return temp;
        }()};
        return Diff{appeared, disappeared, movement};
    }

    /// Utility function for generating animated text.
    /// @param a
    /// @param start_pos
    /// @param end_pos
    /// @param fade
    /// @return ItemText
    ItemText create_text_item(const Item& a, size_t start_pos, size_t end_pos,
                              float fade) {
        auto copy{item_template};
        copy.pos = pos.offset(
            {0, item_template.area.h * static_cast<float>(start_pos + 1)});

        ostringstream oss;
        oss << a;

        if (fade > 0) {
            copy.fg.a = 0;
        }

        return {copy, oss.str(),
                (static_cast<float>(end_pos) - static_cast<float>(start_pos)) *
                    item_template.area.h / fps,
                fade};
    }
};
};  // namespace fprd