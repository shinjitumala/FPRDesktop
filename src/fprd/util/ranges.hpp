/// @file ranges.hpp
/// @author FPR (funny.pig.run __ATMARK__ gmail.com)
///
/// @copyright Copyright (c) 2020
///
/// License: Proprietary
/// You are not allowed to use this file
/// without the author's permission.

#pragma once

#include <compare>
#include <fprd/Pattern.hpp>
#include <iterator>
#include <limits>
#include <utility>

namespace fprd {

using namespace std;

struct Enumerate {};
static constexpr Enumerate enumerate{};

template <class C>
struct Enumeration {
    C &c;

    using BaseIterator =
        conditional_t<is_const_v<C>, typename C::const_iterator,
                      typename C::iterator>;
    struct iterator {
       private:
        using Base = BaseIterator;
        Base itr;

       public:
        using value_type = pair<size_t, decltype(*itr) &>;
        using difference_type = int;
        using pointer = value_type;
        using reference = value_type;

       private:
        size_t index;

       public:
        iterator(Base itr, size_t index) : itr{itr}, index{index} {}

        auto operator++() {
            index++;
            itr++;
            return *this;
        }

        auto operator--() {
            index--;
            itr--;
            return *this;
        }

        reference operator*() { return make_pair(index, ref(*itr)); }
        pointer operator->() { return make_pair(index, Base::operator->()); };
        bool operator!=(const iterator &rhs) const { return itr != rhs.itr; }
        // BUG?
        // strong_ordering operator<=>(const iterator &rhs) const {
        //     return itr <=> rhs.itr;
        // }
    };

    auto begin() { return iterator{c.begin(), 0}; }
    auto end() { return iterator{c.end(), numeric_limits<size_t>::max()}; }
};

template <class C>
auto operator|(C &c, Enumerate /* unused */) {
    return Enumeration<C>{c};
}

/// WARNING: The left range is assumed to be shorter or equal to the right one.
/// @tparam Cs
template <class... Cs>
struct Zipped {
    tuple<Cs &...> containers;

    struct iterator {
        template <class C>
        using BaseIterator =
            conditional_t<is_const_v<C>, typename C::const_iterator,
                          typename C::iterator>;

        tuple<BaseIterator<Cs>...> itrs;

        using iterator_category = bidirectional_iterator_tag;
        using value_type = tuple<decltype(*declval<BaseIterator<Cs>>()) &...>;
        using difference_type = int;
        using pointer = value_type *;
        using reference = value_type;

        iterator(decltype(itrs) itrs) : itrs{itrs} {}

        auto operator++() {
            apply([](auto &...args) { (args++, ...); }, itrs);
            return *this;
        }

        auto operator--() {
            apply([](auto &...args) { (args--, ...); }, itrs);
            return *this;
        }

        reference operator*() {
            return apply([](auto... args) { return make_tuple(ref(*args)...); },
                         itrs);
        }
        bool operator!=(const iterator &rhs) const {
            return get<0>(itrs) != get<0>(rhs.itrs);
        };
        // BUG?
        // auto operator<=>(const iterator &rhs) const { return li <=> rhs.li; }
    };

    iterator begin() {
        return {apply([](auto&... args) { return make_tuple(args.begin()...); },
                      containers)};
    }
    iterator end() {
        return {apply([](auto&... args) { return make_tuple(args.end()...); },
                      containers)};
    }
};

template <class... Cs>
auto zip(Cs &...args) {
    return Zipped<Cs...>{make_tuple(ref(args)...)};
}
};  // namespace fprd