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
    struct iterator : BaseIterator {
       private:
        using Base = BaseIterator;

       public:
        using iterator_category = typename Base::iterator_category;
        using value_type = pair<size_t, typename Base::value_type>;
        using difference_type = typename Base::difference_type;
        using pointer = pair<size_t, typename Base::pointer>;
        using reference = pair<size_t, typename Base::reference>;

       private:
        size_t index;

       public:
        iterator(Base itr, size_t index) : Base{itr}, index{index} {}

        auto operator++() {
            index++;
            Base::operator++();
            return *this;
        }

        auto operator--() {
            index--;
            Base::operator--();
            return *this;
        }

        reference operator*() {
            return make_pair(index, ref(Base::operator*()));
        }
        pointer operator->() { return make_pair(index, Base::operator->()); };
        strong_ordering operator<=>(const iterator &rhs) const {
            return *static_cast<const Base *>(this) <=>
                   static_cast<const Base &>(rhs);
        }
    };

    auto begin() { return iterator{c.begin(), 0}; }
    auto end() { return iterator{c.end(), numeric_limits<size_t>::max()}; }
};

template <class C>
auto operator|(C &c, Enumerate /* unused */) {
    return Enumeration<C>{c};
}

/// WARNING: The left range is assumed to be shorter or equal to the right one.
/// @tparam LC
/// @tparam RC
template <class LC, class RC>
struct Zipped {
    LC &lc;
    RC &rc;

    using LBaseIterator =
        conditional_t<is_const_v<LC>, typename LC::const_iterator,
                      typename LC::iterator>;
    using RBaseIterator =
        conditional_t<is_const_v<RC>, typename RC::const_iterator,
                      typename RC::iterator>;

    struct iterator {
        LBaseIterator li;
        RBaseIterator ri;

        using LBase = LBaseIterator;
        using RBase = RBaseIterator;

        using iterator_category = bidirectional_iterator_tag;
        using value_type = pair<decltype(*li) &, decltype(*ri) &>;
        using difference_type = int;
        using pointer = value_type *;
        using reference = value_type;

        iterator(LBase li, RBase ri) : li{li}, ri{ri} {}

        auto operator++() {
            li++;
            ri++;
            return *this;
        }

        auto operator--() {
            li--;
            ri--;
            return *this;
        }

        reference operator*() { return make_pair(ref(*li), ref(*ri)); }
        bool operator!=(const iterator &rhs) const { return li != rhs.li; };
        // BUG?
        // auto operator<=>(const iterator &rhs) const { return li <=> rhs.li; }
    };

    iterator begin() { return {lc.begin(), rc.begin()}; }
    iterator end() { return {lc.end(), rc.end()}; }
};

template <class LC, class RC>
auto zip(LC &lc, RC &rc) {
    return Zipped<LC, RC>{lc, rc};
}
};  // namespace fprd