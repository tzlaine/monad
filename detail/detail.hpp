#ifndef DETAIL_HPP_INCLUDED_
#define DETAIL_HPP_INCLUDED_

#include <monad_fwd.hpp>
#include <type_traits>
#include <iterator>


namespace monad { namespace detail {

    template <typename Monad>
    struct state_type
    {
        using type = void;
    };

    template <typename T, typename State>
    struct state_type<monad<T, State>>
    {
        using type = State;
    };

    template <typename Monad>
    using state_type_t = typename state_type<Monad>::type;

    template <std::size_t N,
              typename ReturnMonad,
              typename Fn,
              typename ...Monads>
    struct lift_n_impl;

#define MONAD_LIFT_N_MAX_ARITY 10
#include "detail/lift_n_impl.hpp"

    template <typename Container, typename Iter, typename Tag>
    void reserve_impl (Container&, Iter, Iter, Tag)
    {}

    template <typename Container, typename Iter>
    auto reserve_impl (Container& c,
                       Iter first,
                       Iter last,
                       std::random_access_iterator_tag) ->
        decltype(c.reserve(last - first)) // For SFINAE.
    { c.reserve(last - first); }

    template <typename Container, typename Iter>
    void reserve (Container& c, Iter first, Iter last)
    {
        reserve_impl(
            c,
            first,
            last,
            typename std::iterator_traits<Iter>::iterator_category{}
        );
    }

    template <
        typename Iter,
        typename Monad,
        typename OutSeq,
        typename State,
        typename Fn
    >
    monad<OutSeq, State> sequence_impl (Fn f, Iter first, Iter last)
    {
        monad<OutSeq, State> retval;

        if (first == last)
            return retval;

        detail::reserve(retval.mutable_value(), first, last);

        Monad prev = f(first);
        ++first;
        retval.mutable_value().push_back(prev.value());

        while (first != last) {
            Monad m = f(first);
            ++first;
            retval.mutable_value().push_back(m.value());
            prev = prev >>= [m](typename Monad::value_type) {
                return m;
            };
        }

        retval.mutable_state() = prev.state();

        return retval;
    }

    template <typename Fn, typename Iter>
    struct mapped_value_type
    {
        using type = typename std::result_of<
            Fn(typename Iter::value_type)
        >::type::value_type;
    };

    template <typename Fn, typename Iter>
    using mapped_value_type_t =
        typename mapped_value_type<Fn, Iter>::type;

    template <typename Fn, typename Iter1, typename Iter2>
    struct zip_value_type
    {
        using type = typename std::result_of<
            Fn(typename Iter1::value_type, typename Iter2::value_type)
        >::type;
    };

    template <typename Fn, typename Iter1, typename Iter2>
    using zip_value_type_t =
        typename zip_value_type<Fn, Iter1, Iter2>::type;

    template <typename Iter1, typename Iter2>
    struct zip_iterator
    {
        Iter1 first;
        Iter2 second;

        zip_iterator& operator++ ()
        {
            ++first;
            ++second;
            return *this;
        }

        friend bool operator== (zip_iterator lhs, zip_iterator rhs)
        { return lhs.first == rhs.first; }
        friend bool operator!= (zip_iterator lhs, zip_iterator rhs)
        { return lhs.first != rhs.first; }
    };

} }

namespace std {

    template <typename Iter1, typename Iter2>
    struct iterator_traits<monad::detail::zip_iterator<Iter1, Iter2>>
    {
        using iterator_category = random_access_iterator_tag;
    };

}

#endif
