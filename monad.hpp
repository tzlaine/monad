#ifndef MONAD_HPP_INCLUDED_
#define MONAD_HPP_INCLUDED_

#include <vector>

// TODO: decltype(...state()) -> decltype(...)::state_type


namespace monad {

    template <typename T, typename State>
    struct monad
    {
        using this_type = monad<T, State>;
        using value_type = T;
        using state_type = State;

        monad () :
            value_ (),
            state_ ()
        {}

        monad (value_type value, state_type state) :
            value_ (value),
            state_ (state)
        {}

        value_type value () const
        { return value_; }

        state_type state () const
        { return state_; }

        value_type & mutable_value ()
        { return value_; }

        state_type & mutable_state ()
        { return state_; }

        template <typename Fn>
        this_type bind (Fn f) const;

        value_type value_;
        state_type state_;
    };

    template <typename T>
    struct is_monad : std::false_type
    {};

    template <typename T, typename State>
    struct is_monad<monad<T, State>> : std::true_type
    {};

    template <typename T>
    using is_monad_t = typename is_monad<T>::type;

    // operator==().
    template <typename T, typename State>
    bool operator== (monad<T, State> lhs, monad<T, State> rhs)
    { return lhs.data() == rhs.data() && lhs.state() == rhs.state(); }

    // operator!=().
    template <typename T, typename State>
    bool operator!= (monad<T, State> lhs, monad<T, State> rhs)
    { return !(lhs == rhs); }

    // operator>>=().
    template <typename T, typename State, typename Fn>
    monad<T, State> operator>>= (monad<T, State> m, Fn f)
    { return m.bind(f); }

    // operator<<=().
    template <typename T, typename State, typename Fn>
    monad<T, State> operator<<= (Fn f, monad<T, State> m)
    { return m.bind(f); }

    // operator>>().
    template <typename T, typename State>
    monad<T, State> operator>> (monad<T, State> lhs, monad<T, State> rhs)
    {
        return lhs.bind([rhs](T) {
            return rhs;
        });
    }

    // Unary fmap().
    template <typename T, typename State, typename Fn>
    monad<T, State> fmap (Fn f, monad<T, State> m)
    {
        return m >>= [f](T x) {
            return monad<T, State>{f(x)};
        };
    }

    // Unary liftM().
    template <typename T, typename State, typename Fn>
    monad<T, State> lift (Fn f, monad<T, State> m)
    { return fmap(f, m); }

    namespace detail {

        template <std::size_t N,
                  typename ReturnMonad,
                  typename Fn,
                  typename ...Monads>
        struct fmap_n_impl;

#define MONAD_FMAP_N_MAX_ARITY 10
#include "detail/fmap_n_impl.hpp"

    }

    // N-ary fmap().
    template <typename ReturnMonad, typename Fn, typename ...Monads>
    ReturnMonad fmap_n (Fn f, Monads... monads)
    {
        return detail::fmap_n_impl<
            sizeof...(Monads),
            ReturnMonad,
            Fn,
            Monads...
        >::call(f, monads...);
    }

    // N-ary liftM().
    template <typename ReturnMonad, typename Fn, typename ...Monads>
    ReturnMonad lift_n (Fn f, Monads... monads)
    { return fmap_n<ReturnMonad>(f, monads...); }

    namespace detail {

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

            detail::reserve(retval.mutable_value(), first, last);

            bool done_with_binds = false;
            while (first != last) {
                auto m = f(first);
                ++first;
                retval.mutable_value().push_back(m.value());
                if (!done_with_binds) {
                    bool fail = true;
                    m >>= [m, &fail](typename Monad::value_type) {
                        fail = false;
                        return m;
                    };
                    retval.mutable_state() = m.state();
                    if (fail)
                        done_with_binds = true;
                }
            }

            return retval;
        }

    }

    // sequence().
    template <
        typename Iter,
        typename OutSeq = std::vector<typename Iter::value_type::value_type>,
        typename State = typename Iter::value_type::state_type
    >
    monad<OutSeq, State> sequence (Iter first, Iter last)
    {
        return detail::sequence_impl<
            Iter,
            typename Iter::value_type,
            OutSeq,
            State
        >(
            [](Iter it) {return *it;},
            first,
            last
        );
    }

    template <typename Range>
    auto sequence (Range const & r) ->
        decltype(sequence(std::begin(r), std::end(r)))
    { return sequence(std::begin(r), std::end(r)); }

    namespace detail {

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

    }

    // mapM().  Predicate Fn must have a signature of the form
    // monad<...> (typename Iter::value_type).
    template <
        typename Fn,
        typename Iter,
        typename OutSeq = std::vector<
            detail::mapped_value_type_t<Fn, Iter>
        >
    >
    auto map (Fn f, Iter first, Iter last) ->
        monad<OutSeq, decltype(f(*first).state())>
    {
        return detail::sequence_impl<
            Iter,
            decltype(f(*first)),
            OutSeq,
            decltype(f(*first).state())
        >(
            [f](Iter it) {return f(*it);},
            first,
            last
        );
    }

    template <typename Fn, typename Range>
    auto map (Fn f, Range const & r) ->
        decltype(map(f, std::begin(r), std::end(r)))
    { return map(f, std::begin(r), std::end(r)); }

    // mapAndUnzipM().  Predicate Fn must have a signature of the form
    // monad<std::pair<...>, ...> (typename Iter::value_type). // TODO: Test. // TODO: Range version.
    template <
        typename Fn,
        typename Iter,
        typename FirstOutSeq = std::vector<
            typename detail::mapped_value_type_t<Fn, Iter>::first_type
        >,
        typename SecondOutSeq = std::vector<
            typename detail::mapped_value_type_t<Fn, Iter>::second_type
        >
    >
    auto map_unzip (Fn f, Iter first, Iter last) ->
        monad<std::pair<FirstOutSeq, SecondOutSeq>, decltype(f(*first).state())>
    {
        using first_value_type = typename FirstOutSeq::value_type;
        using second_value_type = typename SecondOutSeq::value_type;
        using value_type = std::pair<first_value_type, second_value_type>;
        using state_type = decltype(f(*first).state());
        using monad_type =
            monad<std::pair<FirstOutSeq, SecondOutSeq>, state_type>;

        if (first == last)
            return monad_type{};

        std::pair<FirstOutSeq, SecondOutSeq> out_seqs;
        auto prev_monad = f(*first++);
        prev_monad = prev_monad >>= [prev_monad, &out_seqs](value_type x) {
            out_seqs.first.push_back(x.first);
            out_seqs.second.push_back(x.second);
            return prev_monad;
        };
        while (first != last) {
            auto current_monad = f(*first++);
            prev_monad = prev_monad >>= [current_monad, &out_seqs](value_type) {
                return current_monad >>=
                [current_monad, &out_seqs](value_type x) {
                    out_seqs.first.push_back(x.first);
                    out_seqs.second.push_back(x.second);
                    return current_monad;
                };
            };
        }

        return monad_type{out_seqs, prev_monad.state()};
    }

    // filterM().  Predicate Fn must have a signature of the form
    // monad<bool, ...> (typename Iter::value_type). // TODO: Test.
    template <
        typename Fn,
        typename Iter,
        typename OutSeq = std::vector<typename Iter::value_type>
    >
    auto filter (Fn f, Iter first, Iter last) ->
        monad<OutSeq, decltype(f(*first).state())>
    {
        using state_type = decltype(f(*first).state());

        if (first == last)
            return monad<OutSeq, state_type>{};

        OutSeq out_seq;
        auto value = *first++;
        auto prev_monad = f(value);
        prev_monad = prev_monad >>= [prev_monad, value, &out_seq](bool b) {
            if (b)
                out_seq.push_back(value);
            return prev_monad;
        };
        while (first != last) {
            value = *first++;
            auto current_monad = f(value);
            prev_monad = prev_monad >>= [current_monad, value, &out_seq](bool) {
                return current_monad >>= [current_monad, value, &out_seq](bool b) {
                    if (b)
                        out_seq.push_back(value);
                    return current_monad;
                };
            };
        }

        return monad<OutSeq, state_type>{out_seq, prev_monad.state()};
    }

    template <typename Fn, typename Range>
    auto filter (Fn f, Range const & r) ->
        decltype(filter(f, std::begin(r), std::end(r)))
    { return filter(f, std::begin(r), std::end(r)); }

    namespace detail {

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

    }

    // zipWithM().  Predicate Fn must have a signature of the form
    // monad<...> (typename Iter1::value_type, typename Iter2::value_type).
    template <
        typename Fn,
        typename Iter1,
        typename Iter2,
        typename OutSeq = std::vector<
            detail::zip_value_type_t<Fn, Iter1, Iter2>
        >
    >
    auto zip (Fn f, Iter1 first1, Iter1 last1, Iter2 first2) ->
        monad<OutSeq, decltype(f(*first1, *first2).state())>
    {
        using value_type = typename OutSeq::value_type;
        using state_type = decltype(f(*first1, *first2).state());

        if (first1 == last1)
            return monad<OutSeq, state_type>{};

        OutSeq out_seq;
        auto prev_monad = f(*first1++, *first2++);
        prev_monad = prev_monad >>= [prev_monad, &out_seq](value_type x) {
            out_seq.push_back(x);
            return prev_monad;
        };
        while (first1 != last1) {
            auto current_monad = f(*first1++, *first2++);
            prev_monad = prev_monad >>= [current_monad, &out_seq](value_type) {
                return current_monad >>=
                [current_monad, &out_seq](value_type x) {
                    out_seq.push_back(x);
                    return current_monad;
                };
            };
        }

        return monad<OutSeq, state_type>{out_seq, prev_monad.state()};
    }

    template <typename Fn, typename Range1, typename Range2>
    auto zip (Fn f, Range1 const & r1, Range2 const & r2) ->
        decltype(zip(f, std::begin(r1), std::end(r1), std::begin(r2)))
    { return zip(f, std::begin(r1), std::end(r1), std::begin(r2)); }

    // foldM().  Predicate Fn must have a signature of the form
    // monad<...> (T, typename Iter::value_type::value_type).
    template <typename Fn, typename T, typename Iter>
    auto fold (Fn f, T initial_value, Iter first, Iter last) ->
        decltype(f(initial_value, *first))
    {
        using monad_type = decltype(f(initial_value, *first));
        using value_type = typename monad_type::value_type;

        if (first == last)
            return monad_type{};

        monad_type retval = f(initial_value, *first++);
        while (first != last) {
            auto y = *first++;
            retval = retval >>= [f, y](value_type x) {
                return f(x, y);
            };
        }
        return retval;
    }

    template <typename Fn, typename T, typename Range>
    auto fold (Fn f, T initial_value, Range const & r) ->
        decltype(fold(f, initial_value, std::begin(r), std::end(r)))
    { return fold(f, initial_value, std::begin(r), std::end(r)); }

}

#define MONAD_BINARY_OP(op, monad_type)                             \
    monad_type operator op (monad_type lhs, monad_type rhs)         \
    {                                                               \
        using value_type = monad_type::value_type;                  \
        return lhs >>= [lhs, rhs](value_type x) {                   \
            return rhs >>= [x](value_type y) {                      \
                return monad_type{x + y};                           \
            };                                                      \
        };                                                          \
    }

#define MONAD_NAMED_BINARY_OP(name, op, monad_type)                 \
    monad_type name (monad_type lhs, monad_type rhs)                \
    {                                                               \
        using value_type = monad_type::value_type;                  \
        return lhs >>= [lhs, rhs](value_type x) {                   \
            return rhs >>= [x](value_type y) {                      \
                return monad_type{x + y};                           \
            };                                                      \
        };                                                          \
    }

#define MONAD_TEMPLATE_BINARY_OP(op, monad_name, num_template_args) \
    template <BOOST_PP_ENUM_PARAMS(num_template_args, typename T)>  \
    monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)>          \
    operator op (                                                   \
        monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)> lhs, \
        monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)> rhs  \
    ) {                                                             \
        using monad_type =                                          \
            monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)>; \
        using value_type = typename monad_type::value_type;         \
        return lhs >>= [lhs, rhs](value_type x) {                   \
            return rhs >>= [x](value_type y) {                      \
                return monad_type{x + y};                           \
            };                                                      \
        };                                                          \
    }

#define MONAD_TEMPLATE_NAMED_BINARY_OP(name, op, monad_name, num_template_args)\
    template <BOOST_PP_ENUM_PARAMS(num_template_args, typename T)>      \
    monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)>              \
    name (                                                              \
        monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)> lhs,     \
        monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)> rhs      \
    ) {                                                                 \
        using monad_type =                                              \
            monad_name<BOOST_PP_ENUM_PARAMS(num_template_args, T)>;     \
        using value_type = typename monad_type::value_type;             \
        return lhs >>= [lhs, rhs](value_type x) {                       \
            return rhs >>= [x](value_type y) {                          \
                return monad_type{x + y};                               \
            };                                                          \
        };                                                              \
    }

#endif
