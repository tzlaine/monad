#ifndef MONAD_HPP_INCLUDED_
#define MONAD_HPP_INCLUDED_

#include <detail/detail.hpp>

#include <vector>


namespace monad {

    template <typename T, typename State>
    class monad
    {
    public:
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

        value_type  value () const
        { return value_; }

        state_type state () const
        { return state_; }

        /** TODO @c Fn must accept a single parameter to which @c value_type is
            convertible.  @c Fn must return @c this_type. */
        template <typename Fn>
        this_type bind (Fn f) const;

        /** TODO @c Fn must accept a single parameter to which @c value_type is
            convertible.  @c Fn must return a value that is or is convertible
            to @c this_type. */
        template <typename Fn>
        this_type fmap (Fn f)
        {
            return *this >>= [f](value_type x) {
                return this_type{f(x)};
            };
        }

        using undefined = void;
        undefined join () const;

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

    // operator==().
    template <typename T, typename State>
    bool operator== (monad<T, State> lhs, monad<T, State> rhs)
    { return lhs.value() == rhs.value() && lhs.state() == rhs.state(); }

    // operator!=().
    template <typename T, typename State>
    bool operator!= (monad<T, State> lhs, monad<T, State> rhs)
    { return !(lhs == rhs); }

    // operator>>=().  Fn must have a signature of the form
    // monad<...> (T).
    // (>>=) :: m a -> (a -> m b) -> m b
    template <typename T, typename State, typename Fn>
    auto operator>>= (monad<T, State> m, Fn f) -> decltype(m.bind(f))
    { return m.bind(f); }

    // operator<<=().  Fn must have a signature of the form
    // monad<...> (T).
    // (=<<) :: Monad m => (a -> m b) -> m a -> m b
    template <typename T, typename State, typename Fn>
    auto operator<<= (Fn f, monad<T, State> m) -> decltype(m.bind(f))
    { return m.bind(f); }

    // operator>>().
    // (>>) :: m a -> m b -> m b
    template <typename T1, typename State1, typename T2, typename State2>
    monad<T2, State2> operator>> (monad<T1, State1> lhs, monad<T2, State2> rhs)
    {
        return lhs.bind([rhs](T1) {
            return rhs;
        });
    }

    // join().
    // join :: (Monad m) => m (m a) -> m a
    template <typename T, typename State>
    auto join (monad<T, State> m) -> decltype(m.join())
    { return m.join(); }

    /** TODO @c Fn must accept a single parameter to which @c T is
        convertible.  @c Fn must return a value that is or is convertible to
        <c>monad<T, State></c>.  From the Haskell function <c>fmap :: Functor
        f => (a -> b) -> f a -> f b</c>. */
    template <typename T, typename State, typename Fn>
    auto fmap (Fn f, monad<T, State> m) -> decltype(m.fmap(f))
    { return m.fmap(f); }


    /** TODO (TODO document the wart of needing to have a fixed return type,
        for this and for lift_n).  @c Fn must accept a single parameter to
        which @c T is convertible.  @c Fn must return a value that is or is
        convertible to <c>monad<T, State></c>.  From the Haskell function
        <c>liftM :: (Monad m) => (a -> b) -> (m a -> m b)</c>. */
    template <typename T, typename State, typename Fn>
    monad<T, State> lift (Fn f, monad<T, State> m)
    {
        return m >>= [f](T x) {
            return monad<T, State>{f(x)};
        };
    }

    /** N-ary version of lift(). @c Fn must accept a single@c
        sizeof...(Monads) parameters; the types @c Monads::value_type... must
        be convertible to the respective parameters of @c Fn.  @c Fn must
        return a value that is or is convertible to @c ReturnMonad.  From the
        Haskell function <c>liftM :: (Monad m) => (a -> b) -> (m a -> m
        b)</c>. */
    template <typename ReturnMonad, typename Fn, typename ...Monads>
    ReturnMonad lift_n (Fn f, Monads... monads)
    {
        return detail::lift_n_impl<
            sizeof...(Monads),
            ReturnMonad,
            Fn,
            Monads...
        >::call(f, monads...);
    }

    // sequence().
    // sequence :: Monad m => [m a] -> m [a]
    template <
        typename Iter,
        typename List = list<typename Iter::value_type::value_type>,
        typename State = typename Iter::value_type::state_type
    >
    monad<List, State> sequence (Iter first, Iter last)
    {
        return detail::sequence_impl<
            Iter,
            typename Iter::value_type,
            List,
            State
        >([](Iter it) {return *it;}, first, last);
    }

    template <typename Range>
    auto sequence (Range const & r) ->
        decltype(sequence(std::begin(r), std::end(r)))
    { return sequence(std::begin(r), std::end(r)); }

    // mapM().  Fn must have a signature of the form
    // monad<...> (typename Iter::value_type).
    // mapM :: Monad m => (a -> m b) -> [a] -> m [b]
    template <
        typename Fn,
        typename Iter,
        typename List = list<
            detail::mapped_value_type_t<Fn, Iter>
        >
    >
    auto map (Fn f, Iter first, Iter last) ->
        monad<List, detail::state_type_t<decltype(f(*first))>>
    {
        using monad_type = typename std::remove_cv<decltype(f(*first))>::type;
        using state_type = detail::state_type_t<monad_type>;
        return detail::sequence_impl<Iter, monad_type, List, state_type>(
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
        typename FirstList = list<
            typename detail::mapped_value_type_t<Fn, Iter>::first_type
        >,
        typename SecondList = list<
            typename detail::mapped_value_type_t<Fn, Iter>::second_type
        >
    >
    auto map_unzip (Fn f, Iter first, Iter last) ->
        monad<
            std::pair<FirstList, SecondList>,
            detail::state_type_t<decltype(f(*first))>
        >
    {
        using monad_type = typename std::remove_cv<decltype(f(*first))>::type;
        using state_type = detail::state_type_t<monad_type>;
        using result_type = monad<std::pair<FirstList, SecondList>, state_type>;

        if (first == last)
            return result_type{};

        auto mapped = map(f, first, last);
        using mapped_type = decltype(mapped);
        return mapped >>= [mapped](typename mapped_type::value_type mapped_list) {
            std::size_t size = mapped_list.mutable_value().size();
            std::pair<FirstList, SecondList> data;
            data.first.mutable_value().reserve(size);
            data.second.mutable_value().reserve(size);
            for (auto x : mapped_list.value()) {
                data.first.mutable_value().push_back(x.first);
                data.second.mutable_value().push_back(x.second);
            }
            return result_type{data, mapped.state()};
        };
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
        typename List = list<typename Iter::value_type>
    >
    auto filter (Fn f, Iter first, Iter last) ->
        monad<List, detail::state_type_t<decltype(f(*first))>>
    {
        using monad_type = typename std::remove_cv<decltype(f(*first))>::type;
        using state_type = detail::state_type_t<monad_type>;
        using result_type = monad<List, state_type>;

        if (first == last)
            return result_type{};

        List list;

        detail::reserve(list.mutable_value(), first, last);

        auto prev_value = *first;
        monad_type prev = f(prev_value);
        ++first;

        while (first != last) {
            auto value = *first;
            monad_type m = f(value);
            ++first;
            prev = prev >>= [m, prev_value, &list](bool b) {
                if (b)
                    list.mutable_value().push_back(prev_value);
                return m;
            };
            prev_value = value;
        }

        prev >>= [prev, prev_value, &list](bool b) {
            if (b)
                list.mutable_value().push_back(prev_value);
            return prev;
        };

        return result_type{list, prev.state()};
    }

    template <typename Fn, typename Range>
    auto filter (Fn f, Range const & r) ->
        decltype(filter(f, std::begin(r), std::end(r)))
    { return filter(f, std::begin(r), std::end(r)); }

    // zipWithM().  Fn must have a signature of the form
    // monad<...> (typename Iter1::value_type, typename Iter2::value_type).
    // zipWithM :: (Monad m) => (a -> b -> m c) -> [a] -> [b] -> m [c]
    template <
        typename Fn,
        typename Iter1,
        typename Iter2,
        typename List = list<
            detail::zip_value_type_t<Fn, Iter1, Iter2>
        >
    >
    auto zip (Fn f, Iter1 first1, Iter1 last1, Iter2 first2) ->
        monad<List, detail::state_type_t<decltype(f(*first1, *first2))>>
    {
        using monad_type =
            typename std::remove_cv<decltype(f(*first1, *first2))>::type;
        using state_type = detail::state_type_t<monad_type>;
        using zip_iter = detail::zip_iterator<Iter1, Iter2>;
        zip_iter first{first1, first2};
        zip_iter last{last1, first2};
        return detail::sequence_impl<zip_iter, monad_type, List, state_type>(
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
        typename std::remove_cv<decltype(f(initial_value, *first))>::type
    {
        using monad_type =
            typename std::remove_cv<decltype(f(initial_value, *first))>::type;
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

#endif
