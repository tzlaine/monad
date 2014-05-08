#ifndef MAYBE_MAYBE_HPP_INCLUDED_
#define MAYBE_MAYBE_HPP_INCLUDED_

#include <monad.hpp>


namespace monad {

    namespace detail {

        struct maybe_state
        {
            bool nonempty_;
        };

    }

    struct nothing_t {};
    const nothing_t nothing = {};

    template <typename T>
    class monad<T, detail::maybe_state>
    {
    public:
        using this_type = monad<T, detail::maybe_state>;
        using value_type = T;
        using state_type = detail::maybe_state;

        monad () :
            value_ {},
            state_ {}
        {}

        monad (value_type value, state_type state) :
            value_ {value},
            state_ {state}
        {}

        monad (T t) :
            value_ {t},
            state_ {true}
        {}

        monad (nothing_t) :
            value_ {},
            state_ {false}
        {}

        monad (const monad& rhs) = default;
        monad& operator= (const monad& rhs) = default;

        value_type value () const
        { return value_; }

        state_type state () const
        { return state_; }

        template <typename Fn>
        this_type bind (Fn f) const
        {
            if (!state_.nonempty_)
                return *this;
            else
                return f(value_);
        }

        template <typename State_>
        join_result_t<this_type, State_> join() const
        { return !state_.nonempty_ ? value_type{nothing} : value_; }

        value_type & mutable_value ()
        { return value_; }

        state_type & mutable_state ()
        { return state_; }

    private:
        value_type value_;
        state_type state_;
    };

    template <typename T>
    using maybe = monad<T, detail::maybe_state>;

}

#endif
