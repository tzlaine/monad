#ifndef MONAD_HPP_INCLUDED_
#define MONAD_HPP_INCLUDED_

#include <vector>


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

        template <typename Fn>
        this_type bind (Fn f) const;

        value_type value_;
        state_type state_;
    };

    template <typename T, typename State, typename Fn>
    monad<T, State> operator>>= (monad<T, State> m, Fn f)
    { return m.bind(f); }

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

        template <std::size_t N, typename ReturnMonad, typename Fn, typename ...Monads>
        struct fmap_n_impl;

#define MONAD_FMAP_N_MAX_ARITY 10
#include "detail/fmap_n_impl.hpp"

    }

    // N-ary fmap().
    template <typename ReturnMonad, typename Fn, typename ...Monads>
    ReturnMonad fmap_n (Fn f, Monads... monads)
    {
        return detail::fmap_n_impl<sizeof...(Monads), ReturnMonad, Fn, Monads...>::call(
            f,
            monads...
        );
    }

    // N-ary liftM().
    template <typename ReturnMonad, typename Fn, typename ...Monads>
    ReturnMonad lift_n (Fn f, Monads... monads)
    { return fmap_n<ReturnMonad>(f, monads...); }

    // sequence().
    template <typename InIter,
              typename OutSeq = std::vector<typename InIter::value_type::value_type>,
              typename State = typename InIter::value_type::state_type>
    monad<OutSeq, State> sequence (InIter first, InIter last)
    {
        if (first == last)
            return monad<OutSeq, State>{};

        OutSeq out_seq;
        using value_type = typename InIter::value_type::value_type;
        auto prev_monad = *first++;
        prev_monad = prev_monad >>= [prev_monad, &out_seq](value_type x) {
            out_seq.push_back(x);
            return prev_monad;
        };
        while (first != last) {
            auto current_monad = *first++;
            prev_monad = prev_monad >>= [current_monad, &out_seq](value_type) {
                return current_monad >>= [current_monad, &out_seq](value_type x) {
                    out_seq.push_back(x);
                    return current_monad;
                };
            };
        }

        return monad<OutSeq, State>{out_seq, prev_monad.state()};
    }

    // filterM().  Predicate Fn must have a signature of the form
    // monad<bool, ...> (typename InIter::value_type).
    template <typename Fn, typename InIter, typename OutIter>
    Fn filter (Fn f, InIter first, InIter last, OutIter out)
    {
        while (first != last) {
            auto value = *first++;
            f(value) >>= [value, &out](bool b) {
                if (b)
                    *out++ = value;
            };
        }
    }

    // foldM().  Predicate Fn must have a signature of the form
    // monad<T, ...> (T, T).
    template <typename Fn, typename Iter>
    typename Iter::value_type
    fold (Fn f,
          typename Iter::value_type::value_type initial_value,
          Iter first,
          Iter last)
    {
        using monad_type = typename Iter::value_type;
        using value_type = typename monad_type::value_type;
        monad_type retval{initial_value};
        while (first != last) {
            retval = retval >>= [f, &first](value_type x) {
                return monad_type{f(x, *first++)};
            };
        }
        return retval;
    }

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

#define MONAD_TEMPLATE_NAMED_BINARY_OP(name, op, monad_name, num_template_args) \
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
