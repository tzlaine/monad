#ifndef LIST_IO_HPP_INCLUDED_
#define LIST_IO_HPP_INCLUDED_

#include <list/list.hpp>

#include <iostream>


namespace monad {

    template <typename T>
    std::ostream& operator<< (std::ostream& os, list<T> m)
    {
        os << "[ ";
        for (auto x : m.value()) {
            os << x << ' ';
        }
        os << ']';
        return os;
    }

    template <typename T, typename U>
    std::ostream& operator<< (std::ostream& os, std::pair<list<T>, list<U>> m)
    {
        os << "([ ";
        for (auto x : m.first.value()) {
            os << x << ' ';
        }
        os << "], [";
        for (auto x : m.second.value()) {
            os << x << ' ';
        }
        os << "])";
        return os;
    }

}

#endif
