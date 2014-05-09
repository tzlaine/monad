#ifndef LIST_LIST_HPP_INCLUDED_
#define LIST_LIST_HPP_INCLUDED_

#include <monad.hpp>

#include <algorithm>


namespace monad {

    namespace detail {

        struct list_state {};

    }

    template <typename T>
    class monad<T, detail::list_state>
    {
    public:
        using this_type = monad<T, detail::list_state>;
        using value_type = std::vector<T>;
        using state_type = detail::list_state;

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

        monad (const monad& rhs) = default;
        monad& operator= (const monad& rhs) = default;

        value_type value () const
        { return value_; }

        state_type state () const
        { return state_; }

        /** TODO Fn must return a list monad. */
        template <typename Fn>
        auto bind (Fn f) const -> decltype(f(this->value_[0]))
        {
            using result_type = decltype(f(value_[0]));
            result_type retval;
            retval.mutable_value().reserve(value_.size());
            std::for_each(
                value_.begin(),
                value_.end(),
                [f, &retval](T x) {
                    result_type f_x = f(x);
                    retval.mutable_value().insert(
                        f_x.value().begin(),
                        f_x.value().end()
                    );
                }
            );
        }

        template <typename Fn>
        auto fmap (Fn f) const ->
            monad<decltype(f(this->value_[0])), state_type>
        {
            monad<decltype(f(value_[0])), state_type> retval;
            retval.mutable_value().reserve(value_.size());
            std::for_each(
                value_.begin(),
                value_.end(),
                [f, &retval](T x) {retval.mutable_value().push_back(f(x));}
            );
        }

        template <typename State_>
        join_result_t<this_type, State_> join() const
        { return value_; }

        value_type & mutable_value ()
        { return value_; }

        state_type & mutable_state ()
        { return state_; }

    private:
        value_type value_;
        state_type state_;
    };

    template <typename T>
    using list = monad<T, detail::list_state>;

}

#endif
