#ifndef MONAD_HPP_INCLUDED_
#define MONAD_HPP_INCLUDED_

#include <vector>


namespace monad {

    template <typename T>
    struct is_monad;

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

        typename std::enable_if<is_monad<value_type>::value>::type join ();

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

    // operator>>=().  Fn must have a signature of the form
    // monad<...> (T).
    // (>>=) :: m a -> (a -> m b) -> m b
    template <typename T, typename State, typename Fn>
    monad<T, State> operator>>= (monad<T, State> m, Fn f)
    { return m.bind(f); }

    // operator<<=().  Fn must have a signature of the form
    // monad<...> (T).
    // (=<<) :: Monad m => (a -> m b) -> m a -> m b
    template <typename T, typename State, typename Fn>
    monad<T, State> operator<<= (Fn f, monad<T, State> m)
    { return m.bind(f); }

    // operator>>().  Though the Haskell version operates on two monadic
    // values, limitations of the C++ type system require the operands to be
    // the same.
    // (>>) :: m a -> m b -> m b
    template <typename T, typename State>
    monad<T, State> operator>> (monad<T, State> lhs, monad<T, State> rhs)
    {
        return lhs.bind([rhs](T) {
            return rhs;
        });
    }

    // join().
    // join :: (Monad m) => m (m a) -> m a
    template <typename T, typename State>
    auto join (monad<monad<T, State>, State> m) -> decltype(m.join())
    { return m.join(); }

    // Unary fmap().
    // fmap :: Functor f => (a -> b) -> f a -> f b
    template <typename T, typename State, typename Fn>
    monad<T, State> fmap (Fn f, monad<T, State> m)
    {
        return m >>= [f](T x) {
            return monad<T, State>{f(x)};
        };
    }

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
    // fmap :: Functor f => (a -> b) -> f a -> f b
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

    }

    // sequence().
    // sequence :: Monad m => [m a] -> m [a]
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
        >([](Iter it) {return *it;}, first, last);
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

        template <typename Monad>
        struct state_type
        {
            using type = typename Monad::state_type;
        };

        template <typename Monad>
        using state_type_t = typename state_type<Monad>::type;

    }

    // mapM().  Fn must have a signature of the form
    // monad<...> (typename Iter::value_type).
    // mapM :: Monad m => (a -> m b) -> [a] -> m [b]
    template <
        typename Fn,
        typename Iter,
        typename OutSeq = std::vector<
            detail::mapped_value_type_t<Fn, Iter>
        >
    >
    auto map (Fn f, Iter first, Iter last) ->
        monad<OutSeq, detail::state_type_t<decltype(f(*first))>>
    {
        using monad_type = decltype(f(*first));
        using state_type = detail::state_type_t<monad_type>;
        return detail::sequence_impl<Iter, monad_type, OutSeq, state_type>(
            [f](Iter it) {return f(*it);},
            first,
            last
        );
    }

    template <typename Fn, typename Range>
    auto map (Fn f, Range const & r) ->
        decltype(map(f, std::begin(r), std::end(r)))
    { return map(f, std::begin(r), std::end(r)); }

    // mapAndUnzipM().  Fn must have a signature of the form
    // monad<std::pair<...>, ...> (typename Iter::value_type).
    // mapAndUnzipM :: (Monad m) => (a -> m (b,c)) -> [a] -> m ([b], [c])
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
        monad<
            std::pair<FirstOutSeq, SecondOutSeq>,
            detail::state_type_t<decltype(f(*first))>
        >
    {
        using monad_type = decltype(f(*first));
        using state_type = detail::state_type_t<monad_type>;

        monad<std::pair<FirstOutSeq, SecondOutSeq>, state_type> retval;

        if (first == last)
            return retval;

        detail::reserve(retval.mutable_value(), first, last);

        monad_type prev = f(*first);
        ++first;
        retval.mutable_value().first.push_back(prev.value().first);
        retval.mutable_value().second.push_back(prev.value().second);

        while (first != last) {
            monad_type m = f(*first);
            ++first;
            retval.mutable_value().first.push_back(m.value().first);
            retval.mutable_value().second.push_back(m.value().second);
            prev = prev >>= [m](typename monad_type::value_type) {
                return m;
            };
        }

        retval.mutable_state() = prev.state();

        return retval;
    }

    template <typename Fn, typename Range>
    auto map_unzip (Fn f, Range const & r) ->
        decltype(map_unzip(f, std::begin(r), std::end(r)))
    { return map_unzip(f, std::begin(r), std::end(r)); }

    // filterM().  Predicate Fn must have a signature of the form
    // monad<bool, ...> (typename Iter::value_type).
    // filterM :: Monad m => (a -> m Bool) -> [a] -> m [a]
    template <
        typename Fn,
        typename Iter,
        typename OutSeq = std::vector<typename Iter::value_type>
    >
    auto filter (Fn f, Iter first, Iter last) ->
        monad<OutSeq, detail::state_type_t<decltype(f(*first))>>
    {
        using monad_type = decltype(f(*first));
        using state_type = detail::state_type_t<monad_type>;

        monad<OutSeq, state_type> retval;

        if (first == last)
            return retval;

        detail::reserve(retval.mutable_value(), first, last);

        auto prev_value = *first;
        monad_type prev = f(prev_value);
        ++first;

        while (first != last) {
            auto value = *first;
            monad_type m = f(value);
            ++first;
            prev = prev >>= [m, prev_value, &retval](bool b) {
                if (b)
                    retval.mutable_value().push_back(prev_value);
                return m;
            };
            prev_value = value;
        }

        prev >>= [prev, prev_value, &retval](bool b) {
            if (b)
                retval.mutable_value().push_back(prev_value);
            return prev;
        };

        retval.mutable_state() = prev.state();

        return retval;
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

    }

}

namespace std {

    template <typename Iter1, typename Iter2>
    struct iterator_traits<monad::detail::zip_iterator<Iter1, Iter2>>
    {
        using iterator_category = random_access_iterator_tag;
    };

}

namespace monad {

    // zipWithM().  Fn must have a signature of the form
    // monad<...> (typename Iter1::value_type, typename Iter2::value_type).
    // zipWithM :: (Monad m) => (a -> b -> m c) -> [a] -> [b] -> m [c]
    template <
        typename Fn,
        typename Iter1,
        typename Iter2,
        typename OutSeq = std::vector<
            detail::zip_value_type_t<Fn, Iter1, Iter2>
        >
    >
    auto zip (Fn f, Iter1 first1, Iter1 last1, Iter2 first2) ->
        monad<OutSeq, detail::state_type_t<decltype(f(*first1, *first2))>>
    {
        using monad_type = decltype(f(*first1, *first2));
        using state_type = detail::state_type_t<monad_type>;
        using zip_iter = detail::zip_iterator<Iter1, Iter2>;
        zip_iter first{first1, first2};
        zip_iter last{last1, first2};
        return detail::sequence_impl<zip_iter, monad_type, OutSeq, state_type>(
            [f](zip_iter it) {return f(*it.first, *it.second);},
            first,
            last
        );
    }

    template <typename Fn, typename Range1, typename Range2>
    auto zip (Fn f, Range1 const & r1, Range2 const & r2) ->
        decltype(zip(f, std::begin(r1), std::end(r1), std::begin(r2)))
    { return zip(f, std::begin(r1), std::end(r1), std::begin(r2)); }

    // foldM().  Fn must have a signature of the form
    // monad<T, ...> (T, typename Iter::value_type::value_type).
    // foldM :: (Monad m) => (a -> b -> m a) -> a -> [b] -> m a
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
