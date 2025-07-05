#include "type_traits.hpp"
#include <iostream>
#include <type_traits>
#include <vector>

template <std::size_t Index, class Type> struct TupleLeaf {
  using StoredType = typename std::remove_reference<Type>::type;

  template <class PassedType>
  TupleLeaf(PassedType &&value_) : value{std::forward<PassedType>(value_)} {}
  StoredType value;
};

template <class... Types> struct Tuple;

namespace detail {

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

template <class Seq, class... Types> struct TupleImpl;

template <std::size_t... Indices, class... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleLeaf<Indices, Types>... {

  using Self = TupleImpl<IndexSequence<Indices...>, Types...>;

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
                         typename std::remove_reference<Types>::type...> {

  using Self = Tuple<Types...>;
  using Impl = TupleImpl<MakeIndexSequence<sizeof...(Types)>,
                         typename std::remove_reference<Types>::type...>;

  template <class... Ts>
  Tuple(Ts &&...values) : Impl{std::forward<Ts>(values)...} {}

  // Tuple(Self &&other) : Impl{std::move(other)} { }
  // Tuple(const Self &other) : Impl{other} {}

  Tuple(Self &&other) = default;
  Tuple(const Self &other) = default;
  Tuple &operator=(Self &&other) = default;
  Tuple &operator=(const Self &other) = default;
};

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
Tuple<typename std::remove_reference<Types>::type...>
make_tuple(Types &&...values) {
  return Tuple<typename std::remove_reference<Types>::type...>(
      std::forward<Types>(values)...);
}

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[]) {

  std::vector<int> vv{1, 2, 3, 4, 5};

  auto tt = make_tuple(vv);
  auto tt2 = make_tuple(std::move(vv));
  auto tt3 = std::move(tt);

  auto tt4 = tt2;

  (void)tt3;

  std::cout << get<0>(tt).size() << "\n";
  std::cout << get<0>(tt2).size() << "\n";
  std::cout << get<0>(tt3).size() << "\n";
  std::cout << get<0>(tt4).size() << "\n";

  return 0;
}
