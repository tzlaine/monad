#ifndef LIST_LIST_HPP_INCLUDED_
#define LIST_LIST_HPP_INCLUDED_

#include <monad.hpp>

#include <algorithm>


namespace monad {

    namespace detail {

        struct list_state {};

        bool operator== (list_state lhs, list_state rhs)
        { return true; }

    }

    template <typename T>
    class monad<T, detail::list_state>
    {
    public:
        using this_type = monad<T, detail::list_state>;
        using value_type = std::vector<T>;
        using state_type = detail::list_state;

    private:
        value_type value_;
        state_type state_;

    public:
        monad () :
            value_ {},
            state_ {}
        {}

        monad (value_type value, state_type) :
            value_ {value}
        {}

        monad (T t) :
            value_ {1, t}
        {}

        monad (std::vector<T> v) :
            value_ {v}
        {}

        monad (std::initializer_list<T> l) :
            value_ {l}
        {}

        monad (const monad& rhs) = default;
        monad& operator= (const monad& rhs) = default;

        value_type value () const
        { return value_; }

        state_type state () const
        { return state_; }

        /** TODO Fn must return a list monad. */
        template <typename Fn>
        auto bind (Fn f) const ->
            typename std::remove_cv<decltype(f(value_[0]))>::type
        {
            using result_type =
                typename std::remove_cv<decltype(f(value_[0]))>::type;
            result_type retval;
            retval.mutable_value().reserve(value_.size());
            std::for_each(
                value_.begin(),
                value_.end(),
                [f, &retval](T x) {
                    result_type f_x = f(x);
                    retval.mutable_value().insert(
                        retval.mutable_value().end(),
                        f_x.mutable_value().begin(),
                        f_x.mutable_value().end()
                    );
                }
            );
            return retval;
        }

        template <typename Fn>
        auto fmap (Fn f) const ->
            monad<
                typename std::remove_cv<decltype(f(value_[0]))>::type,
                state_type
            >
        {
            monad<
                typename std::remove_cv<decltype(f(value_[0]))>::type,
                state_type
            > retval;
            retval.mutable_value().reserve(value_.size());
            std::for_each(
                value_.begin(),
                value_.end(),
                [f, &retval](T x) {retval.mutable_value().push_back(f(x));}
            );
            return retval;
        }

        typename value_type::value_type join() const
        {
            return *this >>= [](T x) {
                return x;
            };
        }

        value_type & mutable_value ()
        { return value_; }

        state_type & mutable_state ()
        { return state_; }
    };

    template <typename T>
    using list = monad<T, detail::list_state>;

}

#endif
