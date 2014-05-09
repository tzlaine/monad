#if !BOOST_PP_IS_ITERATING

    template <typename Monad>
    struct monad_t_param;

    template <typename T, typename State>
    struct monad_t_param<monad<T, State>>
    {
        using type = T;
    };

    template <typename Monad>
    using monad_t_param_t = typename monad_t_param<Monad>::type;

#  include <boost/preprocessor/repetition/enum.hpp>
#  include <boost/preprocessor/repetition/enum_params.hpp>
#  include <boost/preprocessor/repetition/enum_binary_params.hpp>
#  include <boost/preprocessor/repetition/repeat.hpp>
#  include <boost/preprocessor/iteration/iterate.hpp>

#  define BOOST_PP_FILENAME_1 "detail/lift_n_impl.hpp"
#  define BOOST_PP_ITERATION_LIMITS (1, MONAD_LIFT_N_MAX_ARITY)
#  include BOOST_PP_ITERATE()

#else

#  define N BOOST_PP_ITERATION()
#  define OPEN(_1, N, _3) return m##N >>= [=](monad_t_param_t<M##N> _##N) {
#  define CLOSE(_1, _2, _3) };

    template <typename ReturnMonad,
              typename Fn,
              BOOST_PP_ENUM_PARAMS(N, typename M)>
    struct lift_n_impl<N,
                       ReturnMonad,
                       Fn,
                       BOOST_PP_ENUM_PARAMS(N, M)>
    {
        // Without all the noise below, call() looks something like:
        // return m0 >>= [=](typename M0::value_type _0) {
        //    return m1 >>= [=](typename M1::value_type _1) {
        //        ...
        //            return ReturnMonad{f(_0, _1, ...))};
        //        ...
        //    };
        // };
        static ReturnMonad call (Fn f,
                                 BOOST_PP_ENUM_BINARY_PARAMS(N, M, m))
        {
            BOOST_PP_REPEAT(N, OPEN, _)
                return ReturnMonad{f(BOOST_PP_ENUM_PARAMS(N, _))};
            BOOST_PP_REPEAT(N, CLOSE, _);
        }
    };

#  undef OPEN
#  undef CLOSE
#  undef N

#endif
