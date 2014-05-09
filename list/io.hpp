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

}

#endif
