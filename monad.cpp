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
        using this_type = monad<T, bool>;
        using value_type = T;
        using state_type = bool;

        monad () :
            value_ {},
            state_ {}
        {}

        monad (value_type value, state_type state) :
            value_ (value),
            state_ (state)
        {}

        monad (T t) :
            value_ {t},
            state_ {true}
        {}

        monad (empty_t) :
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

template <typename T>
std::ostream& operator<< (std::ostream& os, maybe<std::vector<T>> m)
{
    if (!m.state_) {
        os << "[empty]";
    } else {
        os << "[ ";
        for (auto x : m.value()) {
            os << x << ' ';
        }
        os << ']';
    }
    return os;
}

int main()
{
    maybe<int> m_empty_i(empty);
    maybe<int> m_0_i(0);
    maybe<int> m_3_i(3);

    maybe<double> m_empty_d(empty);
    maybe<double> m_1_d(1.0);
    maybe<double> m_4_d(4.0);


    // operator>>=

    std::cout << m_empty_i << " + " << m_3_i << " = "
              << (m_empty_i + m_3_i) << "\n";

    std::cout << m_3_i << " + " << m_empty_i << " = "
              << (m_3_i + m_empty_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " = "
              << (m_0_i + m_3_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " + " << m_0_i << " = "
              << (m_0_i + m_3_i + m_0_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " + " << m_empty_i << " = "
              << (m_0_i + m_3_i + m_empty_i) << "\n";


    // operator>>
    std::cout << m_empty_i << " >> " << m_3_i << " = "
              << (m_empty_i >> m_3_i) << "\n";

    std::cout << m_3_i << " >> " << m_empty_i << " = "
              << (m_3_i >> m_empty_i) << "\n";

    std::cout << m_0_i << " >> " << m_3_i << " = "
              << (m_0_i >> m_3_i) << "\n";


    // unary fmap

    std::cout << "fmap(add_2<int>, " << m_empty_i << ") = "
              << fmap(add_2<int>, m_empty_i) << "\n";

    std::cout << "fmap(add_2<int>, " << m_3_i << ") = "
              << fmap(add_2<int>, m_3_i) << "\n";

    std::cout << "fmap(add_2<int>, fmap(add_2<int>, " << m_3_i << ")) = "
              << fmap(add_2<int>, fmap(add_2<int>, m_3_i)) << "\n";

    std::cout << "lift(add_2<int>, fmap(add_2<int>, " << m_empty_i << ")) = "
              << lift(add_2<int>, fmap(add_2<int>, m_empty_i)) << "\n";


    // 2-ary fmap

    std::cout << "fmap_n(add<int>, " << m_3_i << ", " << m_3_i << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_3_i, m_3_i) << "\n";

    std::cout << "fmap_n(add<int>, " << m_empty_i << ", " << m_3_i << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_empty_i, m_3_i) << "\n";

    std::cout << "fmap_n(add<int>, " << m_3_i << ", " << m_empty_i << ")) = "
              << monad::fmap_n<maybe<int>>(std::plus<int>{}, m_3_i, m_empty_i) << "\n";


    // 3-ary fmap

    std::cout << "fmap_n(add<int>, " << m_3_i << ", " << m_3_i << ", " << m_3_i << ")) = "
              << monad::fmap_n<maybe<int>>(add3<int>, m_3_i, m_3_i, m_3_i) << "\n";

    std::cout << "fmap_n(add<int>, " << m_3_i << ", " << m_empty_i << ", " << m_3_i << ")) = "
              << monad::fmap_n<maybe<int>>(add3<int>, m_3_i, m_empty_i, m_3_i) << "\n";

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_3_i << ", " << m_empty_i << ")) = "
              << monad::lift_n<maybe<int>>(add3<int>, m_3_i, m_3_i, m_empty_i) << "\n";


    // sequence

    std::vector<maybe<int>> bad_maybes = {empty, 0, 3};
    std::vector<maybe<int>> good_maybes = {-1, 0, 3};

    std::cout << "sequence(bad_maybes=[ " << bad_maybes[0] << " " << bad_maybes[1] << " " << bad_maybes[2] << " ]) = "
              << monad::sequence(bad_maybes) << "\n";
    std::cout << "sequence(bad_maybes=[ " << bad_maybes[0] << " " << bad_maybes[1] << " " << bad_maybes[2] << " ]) = "
              << monad::sequence(bad_maybes.begin(), bad_maybes.end()) << "\n";
    std::cout << "sequence(good_maybes=[ " << good_maybes[0] << " " << good_maybes[1] << " " << good_maybes[2] << " ]) = "
              << monad::sequence(good_maybes.begin(), good_maybes.end()) << "\n";
    std::cout << "sequence(good_maybes=[ " << good_maybes[0] << " " << good_maybes[1] << " " << good_maybes[2] << " ]) = "
              << monad::sequence(good_maybes) << "\n";

    std::cout << "ok.";

    return 0;
}
