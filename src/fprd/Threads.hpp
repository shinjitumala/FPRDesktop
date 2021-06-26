/// @file Threads.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary.
/// You may not use or share this file without the permission of the author.

#pragma once

#include <chrono>
#include <dbg/Log.hpp>
#include <fprd/Config.hpp>
#include <fprd/Types.hpp>
#include <fprd/Window.hpp>
#include <fprd/util/time.hpp>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace fprd {
using namespace ::std;

/// The definition of a drawable type in fprd.
/// @tparam D
template <class D>
concept drawable = requires(D &d, Window &w, const typename D::DynamicData &data) {
    { d.update_data(data) } -> same_as<void>;
    { d.draw(w, declval<bool>()) } -> same_as<void>;
    { d.get_data() } -> same_as<typename D::DynamicData>;
    { d.create_window() } -> same_as<Window>;
}
&&is_same_v<decltype(D::probe_interval), const seconds>;

template <drawable D> class Threads {
    using DynamicData = typename D::DynamicData;

    mutex m;         /// Mutex for our buffer.
    DynamicData buf; /// Data buffer.

    /// The thread for fetching new data.
    thread data;
    /// The thread for draw calls to X11.
    thread draw;

  public:
    Threads(atomic<bool> &running, D &d)
        : m{}, data{[&running, &mtx = this->m, &buf = this->buf, interval = D::probe_interval, &d] {
              while (running) {
                  const auto tp{now() + interval};
                  const auto data{d.get_data()};
                  {
                      lock_guard lg{mtx};
                      buf = data;
                  }
                  this_thread::sleep_until(tp);
              }
          }},
          draw{[&running, &d, &buf = this->buf, &mtx = this->m] {
              auto w{d.create_window()};

              /// Frame counter.
              unsigned char frame_counter{0};

              while (running) {
                  const auto tp{now() + draw_interval};

                  const auto has_new_data{frame_counter == 0};
                  if (has_new_data) {
                      /// Attempt to obtain new data.
                      lock_guard lg{mtx};
                      d.update_data(buf);
                  }

                  w.frame_counter = frame_counter;
                  d.draw(w, has_new_data);
                  w.flush();

                  if (now() >= tp) {
                      cerr << "Frame is late by " << duration_cast<milliseconds>(now() - tp).count() << "ms!"
                           << endl;
                  }
                  this_thread::sleep_until(tp);
                  frame_counter++;
                  frame_counter %= fps * duration_cast<seconds>(D::probe_interval).count();
              }
          }} {}

    ~Threads() {
        draw.join();
        data.join();
    }
};
}; // namespace fprd