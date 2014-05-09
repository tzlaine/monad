#include "maybe/maybe.hpp"
#include "maybe/io.hpp"
#include "declare_operators.hpp"

#include <iostream>


MONAD_TEMPLATE_BINARY_OP(+, monad::maybe, 1);

template <typename T>
T add3 (T l, T m, T r)
{ return l + m + r; }

template <typename T>
T add_2 (T t)
{ return t + 2; }

template <typename T>
std::ostream& operator<< (std::ostream& os, monad::maybe<std::vector<T>> m)
{
    if (!m.state().nonempty_) {
        os << "Nothing";
    } else {
        os << "Just [ ";
        for (auto x : m.value()) {
            os << x << ' ';
        }
        os << ']';
    }
    return os;
}

template <typename T, typename U>
std::ostream& operator<< (std::ostream& os,
                          monad::maybe<std::pair<std::vector<T>, std::vector<U>>> m)
{
    if (!m.state().nonempty_) {
        os << "Nothing";
    } else {
        os << "Just ([ ";
        for (auto x : m.value().first) {
            os << x << ' ';
        }
        os << "],[ ";
        for (auto x : m.value().second) {
            os << x << ' ';
        }
        os << "])";
    }
    return os;
}

int main()
{
    monad::maybe<int> m_nothing_i(monad::nothing);
    monad::maybe<int> m_0_i(0);
    monad::maybe<int> m_3_i(3);


    // operator>>=

    std::cout << m_nothing_i << " + " << m_3_i << " = "
              << (m_nothing_i + m_3_i) << "\n";

    std::cout << m_3_i << " + " << m_nothing_i << " = "
              << (m_3_i + m_nothing_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " = "
              << (m_0_i + m_3_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " + " << m_0_i << " = "
              << (m_0_i + m_3_i + m_0_i) << "\n";

    std::cout << m_0_i << " + " << m_3_i << " + " << m_nothing_i << " = "
              << (m_0_i + m_3_i + m_nothing_i) << "\n";


    // operator>>
    std::cout << m_nothing_i << " >> " << m_3_i << " = "
              << (m_nothing_i >> m_3_i) << "\n";

    std::cout << m_3_i << " >> " << m_nothing_i << " = "
              << (m_3_i >> m_nothing_i) << "\n";

    std::cout << m_0_i << " >> " << m_3_i << " = "
              << (m_0_i >> m_3_i) << "\n";


    // unary fmap

    std::cout << "fmap(add_2<int>, " << m_nothing_i << ") = "
              << fmap(add_2<int>, m_nothing_i) << "\n";

    std::cout << "fmap(add_2<int>, " << m_3_i << ") = "
              << fmap(add_2<int>, m_3_i) << "\n";

    std::cout << "fmap(add_2<int>, fmap(add_2<int>, " << m_3_i << ")) = "
              << fmap(add_2<int>, fmap(add_2<int>, m_3_i)) << "\n";

    std::cout << "fmap(add_2<int>, fmap(add_2<int>, " << m_nothing_i << ")) = "
              << fmap(add_2<int>, fmap(add_2<int>, m_nothing_i)) << "\n";


    // unary join

    monad::maybe<monad::maybe<int>> inner_bad = {{monad::nothing}};
    monad::maybe<monad::maybe<int>> outer_bad = {monad::nothing};
    monad::maybe<monad::maybe<int>> both_good = {{1}};

    std::cout << "join(inner_bad=" << inner_bad << ") = "
              << join(inner_bad) << "\n";
    std::cout << "join(outer_bad=" << outer_bad << ") = "
              << join(outer_bad) << "\n";
    std::cout << "join(both_good=" << both_good << ") = "
              << join(both_good) << "\n";


    // unary lift

    std::cout << "lift(add_2<int>, " << m_nothing_i << ") = "
              << lift(add_2<int>, m_nothing_i) << "\n";

    std::cout << "lift(add_2<int>, " << m_3_i << ") = "
              << lift(add_2<int>, m_3_i) << "\n";

    std::cout << "lift(add_2<int>, lift(add_2<int>, " << m_3_i << ")) = "
              << lift(add_2<int>, lift(add_2<int>, m_3_i)) << "\n";

    std::cout << "lift(add_2<int>, lift(add_2<int>, " << m_nothing_i << ")) = "
              << lift(add_2<int>, lift(add_2<int>, m_nothing_i)) << "\n";


    // 2-ary lift

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_3_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_3_i, m_3_i) << "\n";

    std::cout << "lift_n(add<int>, " << m_nothing_i << ", " << m_3_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_nothing_i, m_3_i) << "\n";

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_nothing_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_3_i, m_nothing_i) << "\n";


    // 3-ary lift

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_3_i << ", " << m_3_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_3_i, m_3_i) << "\n";

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_nothing_i << ", " << m_3_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_nothing_i, m_3_i) << "\n";

    std::cout << "lift_n(add<int>, " << m_3_i << ", " << m_3_i << ", " << m_nothing_i << ")) = "
              << monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_3_i, m_nothing_i) << "\n";


    // sequence

    std::vector<monad::maybe<int>> bad_maybes_1 = {monad::nothing, 0, 3};
    std::vector<monad::maybe<int>> bad_maybes_2 = {0, monad::nothing, 3};
    std::vector<monad::maybe<int>> bad_maybes_3 = {0, 3, monad::nothing};
    std::vector<monad::maybe<int>> good_maybes = {-1, 0, 3};

    std::cout << "sequence(bad_maybes_1=[ " << bad_maybes_1[0] << " " << bad_maybes_1[1] << " " << bad_maybes_1[2] << " ]) = "
              << monad::sequence(bad_maybes_1.begin(), bad_maybes_1.end()) << "\n";
    std::cout << "sequence(bad_maybes_1=[ " << bad_maybes_1[0] << " " << bad_maybes_1[1] << " " << bad_maybes_1[2] << " ]) = "
              << monad::sequence(bad_maybes_1) << "\n";
    std::cout << "sequence(bad_maybes_2=[ " << bad_maybes_2[0] << " " << bad_maybes_2[1] << " " << bad_maybes_2[2] << " ]) = "
              << monad::sequence(bad_maybes_2.begin(), bad_maybes_2.end()) << "\n";
    std::cout << "sequence(bad_maybes_2=[ " << bad_maybes_2[0] << " " << bad_maybes_2[1] << " " << bad_maybes_2[2] << " ]) = "
              << monad::sequence(bad_maybes_2) << "\n";
    std::cout << "sequence(bad_maybes_3=[ " << bad_maybes_3[0] << " " << bad_maybes_3[1] << " " << bad_maybes_3[2] << " ]) = "
              << monad::sequence(bad_maybes_3.begin(), bad_maybes_3.end()) << "\n";
    std::cout << "sequence(bad_maybes_3=[ " << bad_maybes_3[0] << " " << bad_maybes_3[1] << " " << bad_maybes_3[2] << " ]) = "
              << monad::sequence(bad_maybes_3) << "\n";
    std::cout << "sequence(good_maybes=[ " << good_maybes[0] << " " << good_maybes[1] << " " << good_maybes[2] << " ]) = "
              << monad::sequence(good_maybes.begin(), good_maybes.end()) << "\n";
    std::cout << "sequence(good_maybes=[ " << good_maybes[0] << " " << good_maybes[1] << " " << good_maybes[2] << " ]) = "
              << monad::sequence(good_maybes) << "\n";


    std::vector<int> set_1 = {1, 2, 3};
    std::vector<int> set_2 = {0, 2, 4};
    std::vector<int> set_3 = {-1, -1, -1};
    std::vector<int> set_4 = {2, 0, 4};
    std::vector<int> set_5 = {2, 4, 0};


    // map

    auto map_nonzero = [](int x) {
        monad::maybe<int> retval = x ? monad::maybe<int>{x} : monad::maybe<int>{monad::nothing};
        return retval;
    };
    auto map_odd = [](int x) {
        monad::maybe<int> retval = x % 2 ? monad::maybe<int>{x} : monad::maybe<int>{monad::nothing};
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


    // map

    auto map_unzip_nonzero = [](int x) {
        monad::maybe<std::pair<int, double>> retval =
            x ?
            monad::maybe<std::pair<int, double>>{{x, x + 0.5}} :
            monad::maybe<std::pair<int, double>>{monad::nothing};
        return retval;
    };
    auto map_unzip_odd = [](int x) {
        monad::maybe<std::pair<int, double>> retval =
            x % 2 ?
            monad::maybe<std::pair<int, double>>{{x, x + 0.5}} :
            monad::maybe<std::pair<int, double>>{monad::nothing};
        return retval;
    };

    std::cout << "map_unzip(map_unzip_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map_unzip(map_unzip_nonzero, set_1.begin(), set_1.end()) << "\n";
    std::cout << "map_unzip(map_unzip_nonzero, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map_unzip(map_unzip_nonzero, set_1) << "\n";
    std::cout << "map_unzip(map_unzip_nonzero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::map_unzip(map_unzip_nonzero, set_2.begin(), set_2.end()) << "\n";
    std::cout << "map_unzip(map_unzip_nonzero, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::map_unzip(map_unzip_nonzero, set_2) << "\n";
    std::cout << "map_unzip(map_unzip_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map_unzip(map_unzip_odd, set_1.begin(), set_1.end()) << "\n";
    std::cout << "map_unzip(map_unzip_odd, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::map_unzip(map_unzip_odd, set_1) << "\n";


    // fold

    auto fold_product = [](double lhs, int rhs) {
        return monad::maybe<double>{lhs * rhs};
    };
    auto fold_quotient = [](double lhs, int rhs) {
        monad::maybe<double> retval = rhs ? monad::maybe<double>{lhs / rhs} : monad::maybe<double>{monad::nothing};
        return retval;
    };

    std::cout << "fold(fold_product, 1.0, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_product, 1.0, set_1.begin(), set_1.end()) << "\n";
    std::cout << "fold(fold_product, 1.0, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_product, 1.0, set_1) << "\n";
    std::cout << "fold(fold_quotient, 1000.0, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_quotient, 1000.0, set_1.begin(), set_1.end()) << "\n";
    std::cout << "fold(fold_quotient, 1000.0, set_1=[ " << set_1[0] << " " << set_1[1] << " " << set_1[2] << " ]) = "
              << monad::fold(fold_quotient, 1000.0, set_1) << "\n";
    std::cout << "fold(fold_quotient, 1000.0, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::fold(fold_quotient, 1000.0, set_2.begin(), set_2.end()) << "\n";
    std::cout << "fold(fold_quotient, 1000.0, set_2=[ " << set_2[0] << " " << set_2[1] << " " << set_2[2] << " ]) = "
              << monad::fold(fold_quotient, 1000.0, set_2) << "\n";


    // filter

    auto filter_odd = [](int x) {
        return monad::maybe<bool>{x % 2 == 1};
    };
    auto filter_flag_zero = [](int x) {
        monad::maybe<bool> retval = x ? monad::maybe<bool>{true} : monad::maybe<bool>{monad::nothing};
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
    std::cout << "filter(filter_flag_zero, set_4=[ " << set_4[0] << " " << set_4[1] << " " << set_4[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_4.begin(), set_4.end()) << "\n";
    std::cout << "filter(filter_flag_zero, set_4=[ " << set_4[0] << " " << set_4[1] << " " << set_4[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_4) << "\n";
    std::cout << "filter(filter_flag_zero, set_5=[ " << set_5[0] << " " << set_5[1] << " " << set_5[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_5.begin(), set_5.end()) << "\n";
    std::cout << "filter(filter_flag_zero, set_5=[ " << set_5[0] << " " << set_5[1] << " " << set_5[2] << " ]) = "
              << monad::filter(filter_flag_zero, set_5) << "\n";


    // zip

    auto zip_sum_nonzero = [](int lhs, int rhs) {
        int sum = lhs + rhs;
        monad::maybe<int> retval = sum ? monad::maybe<int>{sum} : monad::maybe<int>{monad::nothing};
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
