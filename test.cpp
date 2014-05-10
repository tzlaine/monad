#include "maybe/maybe.hpp"
#include "maybe/io.hpp"
#include "list/list.hpp"
#include "list/io.hpp"
#include "declare_operators.hpp"

#include <iostream>

#define BOOST_TEST_MODULE Monad

#include <cstdio>
#include <boost/test/included/unit_test.hpp>


BOOST_AUTO_TEST_CASE(maybe_regularity)
{
    // default constructible
    monad::maybe<int> default_;

    monad::maybe<int> _1 = {1};
    monad::maybe<int> nothing = monad::nothing;

    // copy constructible
    monad::maybe<int> _1_copy{_1};
    monad::maybe<int> nothing_copy{nothing};

    // copy assignable
    monad::maybe<int> assigned_from_1 = _1;
    monad::maybe<int> assigned_from_nothing = nothing;

    // equality comparable
    BOOST_CHECK(default_ == nothing);

    BOOST_CHECK(_1_copy == _1);
    BOOST_CHECK(nothing_copy == nothing);
    BOOST_CHECK(assigned_from_1 == _1);
    BOOST_CHECK(assigned_from_nothing == nothing);

    BOOST_CHECK(_1 != nothing);

    BOOST_CHECK(!(_1_copy != _1));
    BOOST_CHECK(!(nothing_copy != nothing));
    BOOST_CHECK(!(assigned_from_1 != _1));
    BOOST_CHECK(!(assigned_from_nothing != nothing));

    monad::maybe<int> nothing_with_1{1, {false}};
    monad::maybe<int> nothing_with_2{2, {false}};
    BOOST_CHECK(nothing_with_1 == nothing_with_2);

    // mutation does not affect copies
    assigned_from_1 = nothing;
    BOOST_CHECK(_1 != nothing);
}


BOOST_AUTO_TEST_CASE(list_regularity)
{
    // default constructible
    monad::list<int> default_;

    monad::list<int> empty = {};
    monad::list<int> _3 = {1, 2, 3};

    // copy constructible
    monad::list<int> _3_copy{_3};
    monad::list<int> empty_copy{empty};

    // copy assignable
    monad::list<int> assigned_from_3 = _3;
    monad::list<int> assigned_from_empty = empty;

    // equality comparable
    BOOST_CHECK(default_ == empty);

    BOOST_CHECK(_3_copy == _3);
    BOOST_CHECK(empty_copy == empty);
    BOOST_CHECK(assigned_from_3 == _3);
    BOOST_CHECK(assigned_from_empty == empty);

    BOOST_CHECK(_3 != empty);

    BOOST_CHECK(!(_3_copy != _3));
    BOOST_CHECK(!(empty_copy != empty));
    BOOST_CHECK(!(assigned_from_3 != _3));
    BOOST_CHECK(!(assigned_from_empty != empty));

    // mutation does not affect copies
    assigned_from_3 = empty;
    BOOST_CHECK(_3 != empty);
}


// TODO: Test separately.
// MONAD_TEMPLATE_BINARY_OP(+, monad::maybe, 1);

template <typename T>
T add3 (T l, T m, T r)
{ return l + m + r; }

template <typename T>
T add_2 (T t)
{ return t + 2; }

namespace monad {

    template <typename T, typename U>
    std::ostream& operator<< (std::ostream& os,
                              maybe<std::pair<std::vector<T>, std::vector<U>>> m)
    {
        if (!m.state().nonempty_) {
            os << "Nothing";
        } else {
            os << "Just ([ ";
            for (auto x : m.value().first) {
                os << x << ' ';
            }
            os << "], [ ";
            for (auto x : m.value().second) {
                os << x << ' ';
            }
            os << "])";
        }
        return os;
    }

}


BOOST_AUTO_TEST_CASE(maybe)
{
    monad::maybe<int> m_nothing_i = monad::nothing;
    monad::maybe<int> m_0_i = 0;
    monad::maybe<int> m_3_i = 3;
    monad::maybe<double> m_nothing_d = monad::nothing;
    monad::maybe<double> m_0_d = 0.0;
    monad::maybe<double> m_3_d = 3.0;

    auto plus = [](monad::maybe<int> lhs, monad::maybe<int> rhs) {
        using value_type = monad::maybe<int>::value_type;
        return lhs >>= [rhs](value_type x) {
            return rhs >>= [x](value_type y) {
                return monad::maybe<int>{x + y};
            };
        };
    };

    auto plus_double = [](monad::maybe<int> lhs, monad::maybe<int> rhs) {
        using value_type = monad::maybe<int>::value_type;
        return lhs >>= [rhs](value_type x) {
            return rhs >>= [x](value_type y) {
                return monad::maybe<double>{1.0 * x + y};
            };
        };
    };

    // operator>>=

    BOOST_CHECK_EQUAL(plus(m_nothing_i, m_3_i), monad::nothing);
    BOOST_CHECK_EQUAL(plus(m_3_i, m_nothing_i), monad::nothing);
    BOOST_CHECK_EQUAL(plus(m_0_i, m_3_i), m_3_i);
    BOOST_CHECK_EQUAL(plus(m_3_i, m_0_i), m_3_i);

    BOOST_CHECK_EQUAL(plus_double(m_nothing_i, m_3_i), monad::nothing);
    BOOST_CHECK_EQUAL(plus_double(m_3_i, m_nothing_i), monad::nothing);
    BOOST_CHECK_EQUAL(plus_double(m_0_i, m_3_i), m_3_d);
    BOOST_CHECK_EQUAL(plus_double(m_3_i, m_0_i), m_3_d);


    // operator>>

    BOOST_CHECK_EQUAL(m_nothing_i >> m_3_i, monad::nothing);
    BOOST_CHECK_EQUAL(m_3_i >> m_nothing_i, monad::nothing);
    BOOST_CHECK_EQUAL(m_3_i >> m_0_i, m_0_i);
    BOOST_CHECK_EQUAL(m_0_i >> m_3_i, m_3_i);

    BOOST_CHECK_EQUAL(m_nothing_i >> m_3_d, m_nothing_d);
    BOOST_CHECK_EQUAL(m_nothing_d >> m_3_i, m_nothing_i);
    BOOST_CHECK_EQUAL(m_3_i >> m_nothing_d, m_nothing_d);
    BOOST_CHECK_EQUAL(m_3_d >> m_nothing_i, m_nothing_i);
    BOOST_CHECK_EQUAL(m_3_i >> m_0_d, m_0_d);
    BOOST_CHECK_EQUAL(m_3_d >> m_0_i, m_0_i);
    BOOST_CHECK_EQUAL(m_0_i >> m_3_d, m_3_d);
    BOOST_CHECK_EQUAL(m_0_d >> m_3_i, m_3_i);


    // fmap

    BOOST_CHECK_EQUAL((fmap(add_2<int>, m_nothing_i)), monad::nothing);
    BOOST_CHECK_EQUAL((fmap(add_2<int>, m_3_i)), monad::maybe<int>{5});
    BOOST_CHECK_EQUAL((fmap(add_2<int>, fmap(add_2<int>, m_3_i))), monad::maybe<int>{7});
    BOOST_CHECK_EQUAL((fmap(add_2<int>, fmap(add_2<int>, m_nothing_i))), monad::nothing);


    // join

    monad::maybe<monad::maybe<int>> inner_bad = {{monad::nothing}};
    monad::maybe<monad::maybe<int>> outer_bad = {monad::nothing};
    monad::maybe<monad::maybe<int>> both_good = {{3}};

    BOOST_CHECK_EQUAL(join(inner_bad), monad::nothing);
    BOOST_CHECK_EQUAL(join(outer_bad), monad::nothing);
    BOOST_CHECK_EQUAL(join(both_good), m_3_i);


    // unary lift

    BOOST_CHECK_EQUAL((lift(add_2<int>, m_nothing_i)), monad::nothing);
    BOOST_CHECK_EQUAL((lift(add_2<int>, m_3_i)), monad::maybe<int>{5});
    BOOST_CHECK_EQUAL((lift(add_2<int>, lift(add_2<int>, m_3_i))), monad::maybe<int>{7});
    BOOST_CHECK_EQUAL((lift(add_2<int>, lift(add_2<int>, m_nothing_i))), monad::nothing);


    // 2-ary lift

    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_nothing_i, m_3_i)),
                      monad::nothing);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_3_i, m_nothing_i)),
                      monad::nothing);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_0_i, m_3_i)),
                      m_3_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(std::plus<int>{}, m_3_i, m_0_i)),
                      m_3_i);


    // 3-ary lift

    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(add3<int>, m_nothing_i, m_3_i, m_3_i)),
                      monad::nothing);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_nothing_i, m_3_i)),
                      monad::nothing);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_3_i, m_nothing_i)),
                      monad::nothing);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::maybe<int>>(add3<int>, m_3_i, m_3_i, m_3_i)),
                      monad::maybe<int>{9});


    // sequence

    std::vector<monad::maybe<int>> no_maybes;
    std::vector<monad::maybe<int>> bad_maybes_1 = {monad::nothing, 0, 3};
    std::vector<monad::maybe<int>> bad_maybes_2 = {0, monad::nothing, 3};
    std::vector<monad::maybe<int>> bad_maybes_3 = {0, 3, monad::nothing};
    std::vector<monad::maybe<int>> good_maybes = {-1, 0, 3};
    monad::maybe<monad::list<int>> good_sequence{{-1, 0, 3}};

    BOOST_CHECK_EQUAL((sequence(no_maybes)), monad::nothing);
    BOOST_CHECK_EQUAL((sequence(bad_maybes_1)), monad::nothing);
    BOOST_CHECK_EQUAL((sequence(bad_maybes_2)), monad::nothing);
    BOOST_CHECK_EQUAL((sequence(bad_maybes_3)), monad::nothing);
    BOOST_CHECK_EQUAL((sequence(good_maybes)), good_sequence);


    std::vector<int> empty_set;

    std::vector<int> set_123 = {1, 2, 3};
    std::vector<int> set_213 = {2, 1, 3};
    std::vector<int> set_231 = {2, 3, 1};
    std::vector<int> set_024 = {0, 2, 4};
    std::vector<int> set_204 = {2, 0, 4};
    std::vector<int> set_240 = {2, 4, 0};


    // map

    auto map_nonzero = [](int x) {
        return x ? monad::maybe<int>{x} : monad::nothing;
    };
    auto map_even = [](int x) {
        return x % 2 == 0 ? monad::maybe<double>{1.0 * x} : monad::nothing;
    };

    monad::maybe<monad::list<int>> _123_sequence{{1, 2, 3}};
    monad::maybe<monad::list<int>> _213_sequence{{2, 1, 3}};
    monad::maybe<monad::list<int>> _231_sequence{{2, 3, 1}};
    monad::maybe<monad::list<int>> _024_sequence{{0, 2, 4}};
    monad::maybe<monad::list<int>> _204_sequence{{2, 0, 4}};
    monad::maybe<monad::list<int>> _240_sequence{{2, 4, 0}};
    monad::maybe<monad::list<double>> _024_sequence_double{{0.0, 2.0, 4.0}};
    monad::maybe<monad::list<double>> _204_sequence_double{{2.0, 0.0, 4.0}};
    monad::maybe<monad::list<double>> _240_sequence_double{{2.0, 4.0, 0.0}};

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, empty_set)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_123)), _123_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_213)), _213_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_231)), _231_sequence);

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_024)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_204)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_240)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map(map_even, set_123)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_213)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_231)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map(map_even, set_024)), _024_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_204)), _204_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_240)), _240_sequence_double);


#if 0
    // map_unzip

    auto map_unzip_nonzero = [](int x) {
        monad::maybe<std::pair<int, double>> retval =
            x ?
            monad::maybe<std::pair<int, double>>{{x, x + 0.5}} :
            monad::nothing;
        return retval;
    };
    auto map_unzip_even = [](int x) {
        monad::maybe<std::pair<int, double>> retval =
            x % 2 == 0 ?
            monad::maybe<std::pair<int, double>>{{x, x + 0.5}} :
            monad::nothing;
        return retval;
    };

    using unzipped_pair = std::pair<std::vector<int>, std::vector<double>>;
    monad::maybe<unzipped_pair> _123_unzipped_sequence{unzipped_pair{{1, 2, 3}, {1.5, 2.5, 3.5}}};
    monad::maybe<unzipped_pair> _213_unzipped_sequence{unzipped_pair{{2, 1, 3}, {2.5, 1.5, 3.5}}};
    monad::maybe<unzipped_pair> _231_unzipped_sequence{unzipped_pair{{2, 3, 1}, {2.5, 3.5, 1.5}}};
    monad::maybe<unzipped_pair> _024_unzipped_sequence{unzipped_pair{{0, 2, 4}, {0.5, 2.5, 4.5}}};
    monad::maybe<unzipped_pair> _204_unzipped_sequence{unzipped_pair{{2, 0, 4}, {2.5, 0.5, 4.5}}};
    monad::maybe<unzipped_pair> _240_unzipped_sequence{unzipped_pair{{2, 4, 0}, {2.5, 4.5, 0.5}}};

    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, empty_set)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_123)), _123_unzipped_sequence);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_213)), _213_unzipped_sequence);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_231)), _231_unzipped_sequence);

    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_024)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_204)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_nonzero, set_240)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_123)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_213)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_231)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_024)), _024_unzipped_sequence);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_204)), _204_unzipped_sequence);
    BOOST_CHECK_EQUAL((monad::map_unzip(map_unzip_even, set_240)), _240_unzipped_sequence);
#endif


    // fold

    auto fold_product = [](double lhs, int rhs) {
        return monad::maybe<double>{lhs * rhs};
    };
    auto fold_quotient = [](double lhs, int rhs) {
        return rhs ? monad::maybe<double>{lhs / rhs} : monad::nothing;
    };

    auto fold_product_result = monad::fold(fold_product, 1.0, set_123);
    auto fold_quotient_result_0 = monad::fold(fold_quotient, 1000.0, set_123);
    auto fold_quotient_result_1 = monad::fold(fold_quotient, 1000.0, set_024);
    auto fold_quotient_result_2 = monad::fold(fold_quotient, 1000.0, set_204);
    auto fold_quotient_result_3 = monad::fold(fold_quotient, 1000.0, set_240);

    const double epsilon = 1.0e-5;

    BOOST_CHECK_EQUAL(monad::fold(fold_product, 1.0, empty_set), monad::nothing);

    BOOST_CHECK(fold_product_result != monad::nothing);
    BOOST_CHECK_CLOSE(fold_product_result.value(), 6.0, epsilon);
    BOOST_CHECK(fold_quotient_result_0 != monad::nothing);
    BOOST_CHECK_CLOSE(fold_quotient_result_0.value(), 1000.0 / 6.0, epsilon);

    BOOST_CHECK_EQUAL(fold_quotient_result_1, monad::nothing);
    BOOST_CHECK_EQUAL(fold_quotient_result_2, monad::nothing);
    BOOST_CHECK_EQUAL(fold_quotient_result_3, monad::nothing);


    // filter

    auto filter_odd = [](int x) {
        return monad::maybe<bool>{x % 2 == 1};
    };
    auto filter_flag_zero = [](int x) {
        return x ? monad::maybe<bool>{true} : monad::nothing;
    };

    monad::maybe<monad::list<int>> _13_sequence{{1, 3}};
    monad::maybe<monad::list<int>> _31_sequence{{3, 1}};

    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, empty_set)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_123)), _13_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_213)), _13_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_231)), _31_sequence);

    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_024)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_204)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_240)), monad::nothing);


    // zip

    std::vector<float> set_neg_111_float = {-1.0f, -1.0f, -1.0f};
    std::vector<float> set_024_float = {0.0f, 2.0f, 4.0f};

    auto zip_sum_nonzero = [](int lhs, float rhs) {
        float sum = lhs + rhs;
        return sum ? monad::maybe<double>{1.0 * sum} : monad::nothing;
    };

    monad::maybe<monad::list<double>> _147_sequence_double{{1.0, 4.0, 7.0}};

    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, empty_set, set_024_float)), monad::nothing);

    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_123, set_024_float)), _147_sequence_double);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_123, set_neg_111_float)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_213, set_neg_111_float)), monad::nothing);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_231, set_neg_111_float)), monad::nothing);
}


BOOST_AUTO_TEST_CASE(list)
{
    monad::list<int> m_empty_i;
    monad::list<int> m_1_i{8};
    monad::list<int> m_3_i{3, 4, 5};
    monad::list<double> m_empty_d;
    monad::list<double> m_1_d{8.0};
    monad::list<double> m_3_d{3.0, 4.0, 5.0};

    // operator>>=

    auto pos_and_neg = [](int x) {
        return monad::list<int>{x, -x};
    };
    auto half_pos_and_neg_double = [](int x) {
        return monad::list<double>{x / 2.0, -x / 2.0};
    };

    BOOST_CHECK_EQUAL((m_empty_i >>= pos_and_neg), monad::list<int>{});
    BOOST_CHECK_EQUAL((m_1_i >>= pos_and_neg), (monad::list<int>{8, -8}));
    BOOST_CHECK_EQUAL((m_3_i >>= pos_and_neg), (monad::list<int>{3, -3, 4, -4, 5, -5}));

    BOOST_CHECK_EQUAL((m_empty_i >>= half_pos_and_neg_double),
                       monad::list<double>{});
    BOOST_CHECK_EQUAL((m_1_i >>= half_pos_and_neg_double),
                      (monad::list<double>{4.0, -4.0}));
    BOOST_CHECK_EQUAL((m_3_i >>= half_pos_and_neg_double),
                      (monad::list<double>{1.5, -1.5, 2.0, -2.0, 2.5, -2.5}));


    // operator>>

    BOOST_CHECK_EQUAL((m_empty_i >> m_1_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_empty_i >> m_3_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_empty_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_3_i), m_3_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_empty_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_3_i >> m_1_i), (monad::list<int>{8, 8, 8}));

    BOOST_CHECK_EQUAL((m_empty_i >> m_1_d), m_empty_d);
    BOOST_CHECK_EQUAL((m_empty_d >> m_1_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_empty_i >> m_3_d), m_empty_d);
    BOOST_CHECK_EQUAL((m_empty_d >> m_3_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_empty_d), m_empty_d);
    BOOST_CHECK_EQUAL((m_1_d >> m_empty_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_3_d), m_3_d);
    BOOST_CHECK_EQUAL((m_1_d >> m_3_i), m_3_i);
    BOOST_CHECK_EQUAL((m_1_i >> m_empty_d), m_empty_d);
    BOOST_CHECK_EQUAL((m_1_d >> m_empty_i), m_empty_i);
    BOOST_CHECK_EQUAL((m_3_i >> m_1_d), (monad::list<double>{8.0, 8.0, 8.0}));
    BOOST_CHECK_EQUAL((m_3_d >> m_1_i), (monad::list<int>{8, 8, 8}));


    // fmap

    using nested_list_int = monad::list<monad::list<int>>;
    using nested_list_double = monad::list<monad::list<double>>;

    BOOST_CHECK_EQUAL((fmap(pos_and_neg, m_empty_i)),
                      nested_list_int{});
    BOOST_CHECK_EQUAL((fmap(pos_and_neg, m_1_i)),
                      (nested_list_int{{8, -8}}));
    BOOST_CHECK_EQUAL((fmap(pos_and_neg, m_3_i)),
                      (nested_list_int{{3, -3}, {4, -4}, {5, -5}}));

    BOOST_CHECK_EQUAL((fmap(half_pos_and_neg_double, m_empty_i)),
                       nested_list_double{});
    BOOST_CHECK_EQUAL((fmap(half_pos_and_neg_double, m_1_i)),
                      (nested_list_double{{4.0, -4.0}}));
    BOOST_CHECK_EQUAL((fmap(half_pos_and_neg_double, m_3_i)),
                      (nested_list_double{{1.5, -1.5}, {2, -2}, {2.5, -2.5}}));


    // join

    nested_list_int outer_empty = {};
    nested_list_int inner_empty = {{}};
    nested_list_int nested_1 = {{}, {}};
    nested_list_int nested_2 = {{1}, {2}};
    nested_list_int nested_3 = {{1, 2}};
    nested_list_int nested_4 = {{1, 2}, {3}, {}};

    BOOST_CHECK_EQUAL(join(outer_empty), m_empty_i);
    BOOST_CHECK_EQUAL(join(inner_empty), m_empty_i);
    BOOST_CHECK_EQUAL(join(nested_1), m_empty_i);
    BOOST_CHECK_EQUAL(join(nested_2), (monad::list<int>{1, 2}));
    BOOST_CHECK_EQUAL(join(nested_3), (monad::list<int>{1, 2}));
    BOOST_CHECK_EQUAL(join(nested_4), (monad::list<int>{1, 2, 3}));


    // unary lift

    BOOST_CHECK_EQUAL((lift(add_2<int>, m_empty_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((lift(add_2<int>, m_3_i)),
                      (monad::list<int>{5, 6, 7}));
    BOOST_CHECK_EQUAL((lift(add_2<int>, lift(add_2<int>, m_3_i))),
                      (monad::list<int>{7, 8, 9}));
    BOOST_CHECK_EQUAL((lift(add_2<int>, lift(add_2<int>, m_empty_i))),
                      m_empty_i);


    // 2-ary lift

    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(std::plus<int>{}, m_empty_i, m_3_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(std::plus<int>{}, m_3_i, m_empty_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(std::plus<int>{}, m_1_i, m_3_i)),
                      (monad::list<int>{11, 12, 13}));
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(std::plus<int>{}, m_3_i, m_1_i)),
                      (monad::list<int>{11, 12, 13}));


    // 3-ary lift

    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(add3<int>, m_empty_i, m_1_i, m_3_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(add3<int>, m_1_i, m_empty_i, m_3_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(add3<int>, m_1_i, m_3_i, m_empty_i)),
                      m_empty_i);
    BOOST_CHECK_EQUAL((monad::lift_n<monad::list<int>>(add3<int>, m_1_i, m_1_i, m_3_i)),
                      (monad::list<int>{19, 20, 21}));
    BOOST_CHECK_EQUAL(
        (monad::lift_n<monad::list<int>>(add3<int>, m_1_i, m_3_i, m_3_i)),
        (monad::list<int>{8 + 3 + 3, 8 + 3 + 4, 8 + 3 + 5,
                          8 + 4 + 3, 8 + 4 + 4, 8 + 4 + 5,
                          8 + 5 + 3, 8 + 5 + 4, 8 + 5 + 5})
    );


    // sequence

    std::vector<monad::list<int>> no_lists;
    std::vector<monad::list<int>> single_list = {{1, 2}};
    std::vector<monad::list<int>> multiple_lists = {{1}, {2}};
    monad::list<monad::list<int>> single_list_result = {{1}, {2}};
    monad::list<monad::list<int>> multiple_lists_result = {{1, 2}};

    std::vector<monad::maybe<int>> maybes_list_1 = {1, 2};
    std::vector<monad::maybe<int>> maybes_list_2 = {1, monad::nothing};
    monad::maybe<monad::list<int>> maybes_list_1_result = {{1, 2}};

    BOOST_CHECK_EQUAL((sequence(single_list)), single_list_result);
    BOOST_CHECK_EQUAL((sequence(multiple_lists)), multiple_lists_result);

    BOOST_CHECK_EQUAL((sequence(maybes_list_1)), maybes_list_1_result);
    BOOST_CHECK_EQUAL((sequence(maybes_list_2)), monad::nothing);


    std::vector<int> empty_set;

    std::vector<int> set_123 = {1, 2, 3};
    std::vector<int> set_213 = {2, 1, 3};
    std::vector<int> set_231 = {2, 3, 1};
    std::vector<int> set_024 = {0, 2, 4};
    std::vector<int> set_204 = {2, 0, 4};
    std::vector<int> set_240 = {2, 4, 0};


    // map

    auto map_nonzero = [](int x) {
        return x ? monad::list<int>{x} : monad::list<int>{};
    };
    auto map_even = [](int x) {
        return x % 2 == 0 ? monad::list<double>{1.0 * x} : monad::list<double>{};
    };

    monad::list<monad::list<int>> empty_sequence{};
    monad::list<monad::list<double>> empty_sequence_double{};

    monad::list<monad::list<int>> _123_sequence{{1, 2, 3}};
    monad::list<monad::list<int>> _213_sequence{{2, 1, 3}};
    monad::list<monad::list<int>> _231_sequence{{2, 3, 1}};
    monad::list<monad::list<int>> _024_sequence{{0, 2, 4}};
    monad::list<monad::list<int>> _204_sequence{{2, 0, 4}};
    monad::list<monad::list<int>> _240_sequence{{2, 4, 0}};
    monad::list<monad::list<double>> _024_sequence_double{{0.0, 2.0, 4.0}};
    monad::list<monad::list<double>> _204_sequence_double{{2.0, 0.0, 4.0}};
    monad::list<monad::list<double>> _240_sequence_double{{2.0, 4.0, 0.0}};

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, empty_set)), empty_sequence);

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_123)), _123_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_213)), _213_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_231)), _231_sequence);

    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_024)), empty_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_204)), empty_sequence);
    BOOST_CHECK_EQUAL((monad::map(map_nonzero, set_240)), empty_sequence);

    BOOST_CHECK_EQUAL((monad::map(map_even, set_123)), empty_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_213)), empty_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_231)), empty_sequence_double);

    BOOST_CHECK_EQUAL((monad::map(map_even, set_024)), _024_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_204)), _204_sequence_double);
    BOOST_CHECK_EQUAL((monad::map(map_even, set_240)), _240_sequence_double);


    // filter

    auto filter_odd = [](int x) {
        return monad::list<bool>{x % 2 == 1};
    };
    auto filter_flag_zero = [](int x) {
        return x ? monad::list<bool>{true} : monad::list<bool>{};
    };

    monad::list<monad::list<int>> _13_sequence{{1, 3}};
    monad::list<monad::list<int>> _31_sequence{{3, 1}};

    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, empty_set)), empty_sequence);

    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_123)), _13_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_213)), _13_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_odd, set_231)), _31_sequence);

    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_024)), empty_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_204)), empty_sequence);
    BOOST_CHECK_EQUAL((monad::filter(filter_flag_zero, set_240)), empty_sequence);


    // zip

    std::vector<float> set_neg_111_float = {-1.0f, -1.0f, -1.0f};
    std::vector<float> set_024_float = {0.0f, 2.0f, 4.0f};

    auto zip_sum_nonzero = [](int lhs, float rhs) {
        float sum = lhs + rhs;
        return sum ? monad::list<double>{1.0 * sum} : monad::list<double>{};
    };

    monad::list<monad::list<double>> _147_sequence_double{{1.0, 4.0, 7.0}};

    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, empty_set, set_024_float)),
                      empty_sequence_double);

    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_123, set_024_float)),
                      _147_sequence_double);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_123, set_neg_111_float)),
                      empty_sequence_double);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_213, set_neg_111_float)),
                      empty_sequence_double);
    BOOST_CHECK_EQUAL((monad::zip(zip_sum_nonzero, set_231, set_neg_111_float)),
                      empty_sequence_double);
}
