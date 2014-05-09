#ifndef DECLARE_OPERATORS_HPP_INCLUDED_
#define DECLARE_OPERATORS_HPP_INCLUDED_


#define MONAD_BINARY_OP(op, monad_type)                             \
    monad_type operator op (monad_type lhs, monad_type rhs)         \
    {                                                               \
        using value_type = monad_type::value_type;                  \
        return lhs >>= [rhs](value_type x) {                        \
            return rhs >>= [x](value_type y) {                      \
                return monad_type{x + y};                           \
            };                                                      \
        };                                                          \
    }

#define MONAD_NAMED_BINARY_OP(name, op, monad_type)                 \
    monad_type name (monad_type lhs, monad_type rhs)                \
    {                                                               \
        using value_type = monad_type::value_type;                  \
        return lhs >>= [ rhs](value_type x) {                       \
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
        return lhs >>= [rhs](value_type x) {                        \
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
        return lhs >>= [rhs](value_type x) {                            \
            return rhs >>= [x](value_type y) {                          \
                return monad_type{x + y};                               \
            };                                                          \
        };                                                              \
    }

#endif
