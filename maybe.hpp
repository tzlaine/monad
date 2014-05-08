#include <monad.hpp>

#include <iostream>


struct nothing_t {};
const nothing_t nothing = {};

namespace monad {

    template <typename T>
    struct monad<T, bool>
    {
        using this_type = monad<T, bool>;
        using value_type = T;
        using state_type = bool;

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
            if (!state_)
                return *this;
            else
                return f(value_);
        }

        template <typename State_>
        join_result_t<this_type, State_> join() const
        { return !state_ ? value_type{nothing} : value_; }

        value_type & mutable_value ()
        { return value_; }

        state_type & mutable_state ()
        { return state_; }

        value_type value_;
        state_type state_;
    };

}

template <typename T>
using maybe = monad::monad<T, bool>;

template <typename T>
std::ostream& operator<< (std::ostream& os, maybe<T> m)
{
    if (!m.state_)
        os << "Nothing";
    else
        os << "Just " << m.value_;
    return os;
}
