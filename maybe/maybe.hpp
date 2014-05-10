#ifndef MAYBE_MAYBE_HPP_INCLUDED_
#define MAYBE_MAYBE_HPP_INCLUDED_

#include <monad.hpp>


namespace monad {

    namespace detail {

        struct maybe_state
        {
            bool nonempty_;
        };

        bool operator== (maybe_state lhs, maybe_state rhs)
        { return lhs.nonempty_ == rhs.nonempty_; }

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
            state_ {false}
        {}

        monad (value_type value, state_type state) :
            value_ {value},
            state_ (state)
        {}

        monad (value_type t) :
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

        template <typename Fn>
        this_type fmap (Fn f)
        {
            return *this >>= [f](value_type x) {
                return this_type{f(x)};
            };
        }

        value_type join() const
        { return !state_.nonempty_ ? nothing : value_; }

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

    template <typename T>
    bool operator== (maybe<T> lhs, maybe<T> rhs)
    {
        return
            lhs.state() == rhs.state() &&
            (!lhs.state().nonempty_ || lhs.value() == rhs.value());
    }

    template <typename T>
    bool operator== (maybe<T> lhs, nothing_t)
    { return !lhs.state().nonempty_; }

    template <typename T>
    bool operator== (nothing_t n, maybe<T> m)
    { return m == n; }

    template <typename T>
    bool operator!= (maybe<T> m, nothing_t n)
    { return !(m == n); }

    template <typename T>
    bool operator!= (nothing_t n, maybe<T> m)
    { return !(m == n); }

}

#endif
