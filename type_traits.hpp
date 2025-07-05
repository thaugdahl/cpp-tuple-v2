#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

#include <cstddef>

template <std::size_t... Indices>
struct IndexSequence {};

template <std::size_t Idx1, std::size_t... Rest>
struct IndexSequenceImpl {
    using type = typename IndexSequenceImpl<Idx1-1, Idx1-1, Rest...>::type;
};

template <std::size_t... Rest>
struct IndexSequenceImpl<0, Rest...> {
    using type = IndexSequence<Rest...>;
};

template <std::size_t Top>
using MakeIndexSequence = typename IndexSequenceImpl<Top>::type;


template <std::size_t Index, class... Types>
struct TypeAt;

template <std::size_t Index, class First, class... Types>
struct TypeAt<Index, First, Types...> {
    using type = typename TypeAt<Index-1, Types...>::type;
};

template <class First, class... Types>
struct TypeAt<0, First, Types...> {
    using type = First;
};

template <std::size_t Index>
struct TypeAt<Index> {
    static_assert((Index != Index), "Types list exhausted");
};

template <class Desired, class... Types>
struct TypeIndex;

template <class Desired, class First, class... Types>
struct TypeIndex<Desired, First, Types...> {
    static constexpr std::size_t value = 1 + TypeIndex<Desired, Types...>::value;
};

template <class Desired, class... Types>
struct TypeIndex<Desired, Desired, Types...> {
    static constexpr std::size_t value = 0;
};

template <class Desired>
struct TypeIndex<Desired> {
    static_assert(1 != 1, "Exhausted type list");
};

template <bool Condition, class Type>
struct EnableIf;


template <class T1, class T2>
struct IsSame {
    static constexpr bool value = false;
};

template <class T1>
struct IsSame<T1, T1> {
    static constexpr bool value = true;
};


template <class Type>
struct EnableIf<true, Type> {
    using type = Type;
};

template <class Type>
struct EnableIf<false, Type> { };

template <class... TypesT>
struct TypePack {};



#endif
