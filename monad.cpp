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


    std::vector<int> set_1 = {1, 2, 3};
    std::vector<int> set_2 = {0, 2, 4};
    std::vector<int> set_3 = {-1, -1, -1};


    // map

    auto map_nonzero = [](int x) {
        maybe<int> retval = x ? maybe<int>{x} : maybe<int>{empty};
        return retval;
    };
    auto map_odd = [](int x) {
        maybe<int> retval = x % 2 ? maybe<int>{x} : maybe<int>{empty};
        return retval;
    };

    std::cout << "map(map_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map(map_nonzero, set_1.begin(), set_1.end()) << "\n";
    std::cout << "map(map_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map(map_nonzero, set_1) << "\n";
    std::cout << "map(map_nonzero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::map(map_nonzero, set_2.begin(), set_2.end()) << "\n";
    std::cout << "map(map_nonzero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::map(map_nonzero, set_2) << "\n";
    std::cout << "map(map_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map(map_odd, set_1.begin(), set_1.end()) << "\n";
    std::cout << "map(map_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map(map_odd, set_1) << "\n";

    // fold

    auto fold_product = [](int lhs, int rhs) {
        return maybe<int>{lhs * rhs};
    };
    auto fold_quotient = [](int lhs, int rhs) {
        maybe<int> retval = rhs ? maybe<int>{lhs / rhs} : maybe<int>{empty};
        return retval;
    };

    std::cout << "fold(fold_product, 1, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_product, 1, set_1.begin(), set_1.end()) << "\n";
    std::cout << "fold(fold_product, 1, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_product, 1, set_1) << "\n";
    std::cout << "fold(fold_quotient, 1000.0, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_quotient, 1000.0, set_1.begin(), set_1.end()) << "\n";
    std::cout << "fold(fold_quotient, 1000, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_quotient, 1000, set_1) << "\n";
    std::cout << "fold(fold_quotient, 1000, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::fold(fold_quotient, 1000, set_2.begin(), set_2.end()) << "\n";
    std::cout << "fold(fold_quotient, 1000, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::fold(fold_quotient, 1000, set_2) << "\n";


    // filter

    auto filter_odd = [](int x) {
        return maybe<bool>{x % 2 == 1};
    };
    auto filter_flag_zero = [](int x) {
        maybe<bool> retval = x ? maybe<bool>{true} : maybe<bool>{empty};
        return retval;
    };

    std::cout << "filter(filter_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::filter(filter_odd, set_1.begin(), set_1.end()) << "\n";
    std::cout << "filter(filter_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::filter(filter_odd, set_1) << "\n";
    std::cout << "filter(filter_flag_zero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_2.begin(), set_2.end()) << "\n";
    std::cout << "filter(filter_flag_zero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_2) << "\n";

    // zip

    auto zip_sum_nonzero = [](int lhs, int rhs) {
        int sum = lhs + rhs;
        maybe<int> retval = sum ? maybe<int>{sum} : maybe<int>{empty};
        return retval;
    };

    std::cout << "zip(zip_sum_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ], set_2=[ "
              << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::zip(zip_sum_nonzero, set_1.begin(), set_1.end(), set_2.begin()) << "\n";
    std::cout << "zip(zip_sum_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ], set_2=[ "
              << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::zip(zip_sum_nonzero, set_1, set_2) << "\n";
    std::cout << "zip(zip_sum_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ], set_3=[ "
              << set_3[0] << " " << set_3[1] << " " << set_3[2] << " ]) = "
              << monad::zip(zip_sum_nonzero, set_1.begin(), set_1.end(), set_3.begin()) << "\n";
    std::cout << "zip(zip_sum_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ], set_3=[ "
              << set_3[0] << " " << set_3[1] << " " << set_3[2] << " ]) = "
              << monad::zip(zip_sum_nonzero, set_1, set_3) << "\n";


    std::cout << "ok.";

    return 0;
}
