#include "monad.hpp"

#include <boost/preprocessor/repetition/enum_params.hpp>

#include <iostream>
#include <functional>


struct empty_t {};
const empty_t empty = {};

namespace monad {

    template <typename T>
    struct monad<T, bool>
    {
        typedef monad<T, bool> this_type;
        typedef T value_type;
        typedef bool state_type;

        monad () :
            value_ {},
            state_ {}
        {}

        explicit monad (T t) :
            value_ {t},
            state_ {true}
        {}

        explicit monad (empty_t) :
            value_ {},
            state_ {false}
        {}

        monad (const monad& rhs) = default;
        monad& operator= (const monad& rhs) = default;

        value_type value () const
        { return value_; }

        state_type state () const
        { return state_; }

        template <typename Fn>
        this_type bind (Fn f) const
        {
            if (!state_)
                return *this;
            else
                return f(value_);
        }

        value_type value_;
        state_type state_;
    };

}

template <typename T>
using maybe = monad::monad<T, bool>;

MONAD_TEMPLATE_BINARY_OP(+, maybe, 1);

template <typename T>
T add3 (T l, T m, T r)
{ return l + m + r; }

template <typename T>
T add_2 (T t)
{ return t + 2; }

template <typename T>
std::ostream& operator<< (std::ostream& os, maybe<T> m)
{
    if (!m.state_)
        os << "[empty]";
    else
        os << m.value_;
    return os;
}

int main()
{
    maybe<int> m_empty(empty);
    maybe<int> m_3(3);
    maybe<int> m_0(0);


    // operator>>=

    std::cout << m_empty << " + " << m_3 << " = "
              << (m_empty + m_3) << "\n";

    std::cout << m_3 << " + " << m_empty << " = "
              << (m_3 + m_empty) << "\n";

    std::cout << m_0 << " + " << m_3 << " = "
              << (m_0 + m_3) << "\n";

    std::cout << m_0 << " + " << m_3 << " + " << m_0 << " = "
              << (m_0 + m_3 + m_0) << "\n";

    std::cout << m_0 << " + " << m_3 << " + " << m_empty << " = "
              << (m_0 + m_3 + m_empty) << "\n";


    // unary fmap

    std::cout << "fmap(add_2<int>, " << m_empty << ") = "
              << fmap(add_2<int>, m_empty) << "\n";

    std::cout << "fmap(add_2<int>, " << m_3 << ") = "
              << fmap(add_2<int>, m_3) << "\n";

    std::cout << "fmap(add_2<int>, fmap(add_2<int>, " << m_3 << ")) = "
              << fmap(add_2<int>, fmap(add_2<int>, m_3)) << "\n";

    std::cout << "lift(add_2<int>, fmap(add_2<int>, " << m_empty << ")) = "
              << lift(add_2<int>, fmap(add_2<int>, m_empty)) << "\n";


    // 2-ary fmap

    std::cout << "fmap_n(add<int>, " << m_3 << ", " << m_3 << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_3, m_3) << "\n";

    std::cout << "fmap_n(add<int>, " << m_empty << ", " << m_3 << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_empty, m_3) << "\n";

    std::cout << "fmap_n(add<int>, " << m_3 << ", " << m_empty << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_3, m_empty) << "\n";


    // 3-ary fmap

    std::cout << "fmap_n(add<int>, " << m_3 << ", " << m_3 << ", " << m_3 << ")) = "
              << monad::fmap_n<maybe<int>>(add3<int>, m_3, m_3, m_3) << "\n";

    std::cout << "fmap_n(add<int>, " << m_3 << ", " << m_empty << ", " << m_3 << ")) = "
              << monad::fmap_n<maybe<int>>(add3<int>, m_3, m_empty, m_3) << "\n";

    std::cout << "lift_n(add<int>, " << m_3 << ", " << m_3 << ", " << m_empty << ")) = "
              << monad::lift_n<maybe<int>>(add3<int>, m_3, m_3, m_empty) << "\n";


    std::cout << "ok.";

    return 0;
}
