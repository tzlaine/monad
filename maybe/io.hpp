#ifndef MAYBE_IO_HPP_INCLUDED_
#define MAYBE_IO_HPP_INCLUDED_

#include <maybe/maybe.hpp>

#include <iostream>


namespace monad {

    template <typename T>
    std::ostream& operator<< (std::ostream& os, maybe<T> m)
    {
        if (!m.state().nonempty_)
            os << "Nothing";
        else
            os << "Just " << m.value();
        return os;
    }

    template <typename T>
    std::ostream& operator<< (std::ostream& os, maybe<std::vector<T>> m)
    {
        if (!m.state().nonempty_) {
            os << "Nothing";
        } else {
            os << "Just [ ";
            for (auto && v : m.value()) {
                os << v << " ";
            }
            os << "]";
        }
        return os;
    }

    template <typename T, typename U>
    std::ostream& operator<< (
        std::ostream& os,
        maybe<std::pair<std::vector<T>, std::vector<U>>> m
    ) {
        if (!m.state().nonempty_) {
            os << "Nothing";
        } else {
            os << "Just [ ";
            for (auto && v : m.value().first) {
                os << v << " ";
            }
            os << "] [ ";
            for (auto && v : m.value().second) {
                os << v << " ";
            }
            os << "]";
        }
        return os;
    }

    inline std::ostream& operator<< (std::ostream& os, nothing_t)
    { return os << "Nothing"; }

}

#endif
