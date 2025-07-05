// C++11 Compatible Tuple Implementation

#include "type_traits.hpp"
#include <iostream>
#include <type_traits>
#include <vector>


template <class... Types> struct Tuple;

namespace detail {

/**
* Conflicting Move Params
* ----------------------------
* A metaprogramming utility to identify
* when a parameter pack will cause conflict when resolving
* the move constructor.
*/
template <class Pack1, class Pack2> struct ConflictingMoveParams;

template <class... Types, class... Params>
struct ConflictingMoveParams<TypePack<Types...>, TypePack<Params...>> {

  static constexpr bool value =
      (sizeof...(Params) == 1) &&
      (IsSame<typename std::decay<typename TypeAt<0, Params...>::type>::type,
              Tuple<Types...>>::value) &&
      sizeof...(Params) == sizeof...(Types);
};
} // namespace detail

//===------------------------------------------------------------===//
//=== Tuple Implementation
//===------------------------------------------------------------===//
template <class Seq, class... Types> struct TupleImpl;

// EBO Class
template <std::size_t Index, class Type> struct TupleLeaf {
  using StoredType = typename RemoveRef<Type>::type;

  template <class PassedType>
  TupleLeaf(PassedType &&value_) : value{std::forward<PassedType>(value_)} {}
  StoredType value;
};

//
template <std::size_t... Indices, class... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleLeaf<Indices, Types>... {

    using Self = TupleImpl<IndexSequence<Indices...>, Types...>;


    //------------------------------
    // Constructors and Assignments
    //------------------------------

    // SFINAE. Disable this constructor if it conflicts with the default move
    // constructor
    template <class... Ts,
    typename EnableIf<!detail::ConflictingMoveParams<
    TypePack<Types...>, TypePack<Ts...>>::value,
    int>::type = 0>
    TupleImpl(Ts &&...values)
    : TupleLeaf<Indices, Types>{std::forward<Ts>(values)}... {}

    TupleImpl(Self &&other)
        : TupleLeaf<Indices, Types>(std::move(
        static_cast<TupleLeaf<Indices, Types> &>(other).value))... {}

    TupleImpl(const Self &other)
        : TupleLeaf<Indices, Types>(
        static_cast<const TupleLeaf<Indices, Types> &>(other).value)... {}


    // Move assignment. Implemented with "poor-mans pack expansion"
    template <class TypeT, std::size_t Index> inline void move_impl(Self &other) {
        static_cast<TupleLeaf<Index, TypeT> &>(*this).value =
            std::move(static_cast<TupleLeaf<Index, TypeT> &>(other).value);
    }

    template <class... TypesP, std::size_t... IndicesP>
    inline void move(Self &&other, IndexSequence<IndicesP...>) {
        (void)std::initializer_list<int>{(move_impl<TypesP, Indices>(other), 0)...};
    }

    Self &operator=(Self &&other) {
        using Seq = MakeIndexSequence<sizeof...(Types)>;
        move<Types...>(std::move(other), Seq{});
        return *this;
    }
};

template <class... Types>
struct Tuple : TupleImpl<MakeIndexSequence<sizeof...(Types)>,
                         typename RemoveRef<Types>::type...> {

  using Self = Tuple<Types...>;
  using Impl = TupleImpl<MakeIndexSequence<sizeof...(Types)>,
                         typename RemoveRef<Types>::type...>;

  template <class... Ts>
  Tuple(Ts &&...values) : Impl{std::forward<Ts>(values)...} {}

  // Tuple(Self &&other) : Impl{std::move(other)} { }
  // Tuple(const Self &other) : Impl{other} {}

  Tuple(Self &&other) = default;
  Tuple(const Self &other) = default;
  Tuple &operator=(Self &&other) = default;
  Tuple &operator=(const Self &other) = default;
};

//===------------------------------------------------------------===//
//=== Tuple Getters
//===------------------------------------------------------------===//

template <std::size_t Index, class... Types>
typename TypeAt<Index, Types...>::type &get(Tuple<Types...> &tup) {
  using Type = typename TypeAt<Index, Types...>::type;
  return static_cast<TupleLeaf<Index, Type> &>(tup).value;
}

template <std::size_t Index, class... Types>
typename TypeAt<Index, Types...>::type &&get(Tuple<Types...> &&tup) {
  using Type = typename TypeAt<Index, Types...>::type;
  return std::move(static_cast<TupleLeaf<Index, Type> &>(tup).value);
}

template <std::size_t Index, class... Types>
const typename TypeAt<Index, Types...>::type &get(const Tuple<Types...> &tup) {
  using Type = typename TypeAt<Index, Types...>::type;
  return static_cast<TupleLeaf<Index, Type> &>(tup).value;
}

template <class... Types>
Tuple<typename RemoveRef<Types>::type...>
make_tuple(Types &&...values) {
  return Tuple<typename RemoveRef<Types>::type...>(
      std::forward<Types>(values)...);
}


//===------------------------------------------------------------===//
//=== Tuple Apply
//===------------------------------------------------------------===//

template <class TupleLike>
struct tuple_size;

template<class... Typs>
struct tuple_size<Tuple<Typs...>> {
    static constexpr std::size_t value = sizeof...(Typs);
};

template <class Fn, class TupleLike, std::size_t... Indices>
auto apply_impl(Fn &&fn, TupleLike &&tupl, IndexSequence<Indices...>)
    -> decltype(std::forward<Fn>(fn)(get<Indices>(std::forward<TupleLike>(tupl))...))
    {
    return std::forward<Fn>(fn)(get<Indices>(tupl)...);
}


template <class Fn, class TupleLike>
auto apply(Fn &&fn, TupleLike &&tupl)
    -> decltype(
        apply_impl(
            std::forward<Fn>(fn), // Function
            std::forward<TupleLike>(tupl), // Tuple
            MakeIndexSequence<
                tuple_size<typename RemoveRef<TupleLike>::type>::value
            >()
        )
    ) {
    return apply_impl(std::forward<Fn>(fn), tupl, MakeIndexSequence<tuple_size<typename RemoveRef<TupleLike>::type>::value>{});
}

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[]) {

    std::vector<int> vv{1, 2, 3, 4, 5};
    std::vector<std::string> vs{"Hi"};

    auto tt = make_tuple(vv);
    auto tt3 = make_tuple(std::move(vv), std::move(vs));


    apply([](std::vector<int> &x, std::vector<std::string> &s) -> void {
        for ( auto &i : x ) {
            std::cout << i << "\n";
        }

        for ( auto &i : s ) {
            std::cout << i << "\n";
        }
    }, tt3);


    std::cout << get<0>(tt).size() << "\n";
    std::cout << get<0>(tt3).size() << "\n";

    return 0;
}
