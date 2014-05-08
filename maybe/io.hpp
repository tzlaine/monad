#ifndef MAYBE_IO_HPP_INCLUDED_
#define MAYBE_IO_HPP_INCLUDED_

#include <maybe/maybe.hpp>

#include <iosfwd>


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

}

#endif
