#ifndef TYPECALC2_HPP
#define TYPECALC2_HPP

#include <tuple>
#include <array>


namespace TypeCalc {

/////////////Base List
template <typename ...T>
struct type_tuple {

};

/////////////Size
template <typename T> struct tuple_size;
template <typename ... T> struct tuple_size<type_tuple<T...>>{static constexpr std::size_t size = sizeof... (T);};
template <typename T> static constexpr std::size_t tuple_size_v = tuple_size<T>::size;
/////////////TYPE TUPLE ITERATION/////////////

template<class...Ts, class ...Args, class F>
void iterateTypeTuple(type_tuple<Ts...> *, F && func, Args &&...args) {
    using expander = int[];
    (void)expander { 0, ((void)func((Ts*)(nullptr), std::forward<Args>(args)...), 0)... };
}


/////////////FIND TYPE INDEX IN TUPLE//////


template<std::size_t CurrIndex, class T, class ListT, typename TTT=void >
struct tuple_first_type_index {
    static constexpr bool not_found = true;
};

template<std::size_t CurrIndex, class T,  class Head, class... Tail>
struct tuple_first_type_index<CurrIndex, T, type_tuple<Head, Tail...>, std::enable_if_t<!std::is_same_v<Head, T>>>
    : tuple_first_type_index<CurrIndex+1, T, type_tuple<Tail...>> {
};

template<std::size_t CurrIndex, class T,  class Head, class... Tail >
struct tuple_first_type_index<CurrIndex, T, type_tuple<Head, Tail...>, std::enable_if_t<std::is_same_v<Head, T>>> {
    static constexpr std::size_t value = CurrIndex;
    static constexpr bool not_found = false;
};

template <class T, class TupleT>
constexpr std::size_t tuple_first_type_index_v = tuple_first_type_index<0, T, TupleT>::value;

template <class T, class TupleT>
constexpr bool type_in_tuple_v = !tuple_first_type_index<0, T, TupleT>::not_found;


static_assert (tuple_first_type_index_v<int, type_tuple<float, bool, int, int>> == 2, "works");
static_assert (type_in_tuple_v<int, type_tuple<float, bool, int, int>>, "works");
static_assert (!type_in_tuple_v<double, type_tuple<float, bool, int, int>>, "works");


/////////////GET ELEM BY INDEX IN TUPLE//////

template<std::size_t CurrIndex, class ListT, typename TTT=void >
struct tuple_elem_by_index {
};

template<std::size_t CurrIndex, class Head, class... Tail>
struct tuple_elem_by_index<CurrIndex,  type_tuple<Head, Tail...>, std::enable_if_t<(CurrIndex > 0)> >
    : tuple_elem_by_index<CurrIndex-1, type_tuple<Tail...> > {
};

template<std::size_t CurrIndex, class Head, class... Tail >
struct tuple_elem_by_index<CurrIndex, type_tuple<Head, Tail...>, std::enable_if_t< (CurrIndex == 0)> > {
    using type = Head;
};

template <std::size_t N, class TupleT>
using tuple_elem_by_index_t = typename tuple_elem_by_index<N, TupleT>::type;


static_assert (std::is_same_v< tuple_elem_by_index_t<1, type_tuple<float, bool, int, int>>, bool>, "works");
static_assert (std::is_same_v< tuple_elem_by_index_t<2, type_tuple<float, bool, int, int>>, int>, "works");


/////////////EXTRACT TYPES SUBTUPLE FROM TUPLE////////////

template<std::size_t FromI, std::size_t ToI, class TupleT, class Sequence, typename TTT=void >
struct tuple_types_interval {};

template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) < ToI-FromI>>
    : tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is..., FromI+sizeof... (Is)>> {};

template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) == ToI-FromI>> {
    using type = type_tuple<tuple_elem_by_index_t<Is,TupleT >...>;
};

template<std::size_t FromI, std::size_t ToI, class TupleT>
using tuple_types_interval_t= typename tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<>>::type;

static_assert(std::is_same_v<type_tuple<float>, tuple_types_interval_t<1,2,  type_tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<type_tuple<float, char>, tuple_types_interval_t<1,3,  type_tuple<int, float, char, bool>>>, "dddd");

static_assert(std::is_same_v<type_tuple<float, char>, tuple_types_interval_t<1,3,  type_tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<type_tuple<float, char, bool>, tuple_types_interval_t<1,4,  type_tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<type_tuple<char, bool>, tuple_types_interval_t<2,4,  type_tuple<int, float, char, bool>>>, "dddd");


/////////////CONCAT TUPLES/////////////

template <typename TL1, typename TL2>
struct concatenator2{};

template <typename ...T1, typename ...T2>
struct concatenator2<type_tuple<T1...>, type_tuple<T2...>>{
    using type = type_tuple<T1..., T2...>;
};

template <typename Res, typename ...T>
struct concatenator{};

template <typename Res, typename TH, typename ...TR>
struct concatenator<Res, TH, TR...>: concatenator<typename concatenator2<Res, TH>::type, TR...>{

};
template <typename ...ResT>
struct concatenator<type_tuple<ResT...>>{
    using type = type_tuple<ResT...>;
};


template<typename ... input_t>
using tuple_cat_t= typename concatenator<type_tuple<>, input_t...>::type;

static_assert (std::is_same_v<tuple_cat_t<type_tuple<int, bool>, type_tuple<char>, type_tuple<double, void*>>, type_tuple<int, bool, char, double, void*>> );



/////////////
template<class In>
struct DefaulExtractor {static constexpr int value = In::value;};

template<class In1, class In2>
struct DefaulCompare {static constexpr int value = In1::value < In2::value;};
template<class In1, class In2>
struct DefaulIsSame {static constexpr int value = In1::value == In2::value;};
/////////////FIND MINUMUM TYPE by ::value MEMBER IN TUPLE

template<class ElemsTuple, template<typename, typename> class Comparator, typename TTT = void>
struct min_finder {};

template<class Elem, template<typename, typename> class Comparator>
struct min_finder<type_tuple<Elem>, Comparator> {
    using type = Elem;
};

template<template<typename,typename> class Comparator, class ElemFirst, class ElemSecond, class ... ElemsRest>
struct min_finder<type_tuple<ElemFirst, ElemSecond, ElemsRest...>, Comparator, std::enable_if_t<Comparator<ElemFirst, ElemSecond>::value == true>>:
        min_finder<type_tuple<ElemFirst, ElemsRest...>, Comparator> {};

template<template<typename, typename> class Comparator, class ElemFirst, class ElemSecond, class ... ElemsRest>
struct min_finder<type_tuple<ElemFirst, ElemSecond, ElemsRest...>, Comparator, std::enable_if_t<Comparator<ElemFirst, ElemSecond>::value== false>>:
        min_finder<type_tuple<ElemSecond, ElemsRest...>, Comparator> {};

template<class Tuple, template<typename, typename> class Comparator = DefaulCompare>
using min_finder_t =  typename min_finder<Tuple, Comparator>::type;

namespace min_finder_test {
    static_assert(min_finder_t<type_tuple<std::integral_constant<int, 5>, std::integral_constant<int, 4>,  std::integral_constant<int, 8>>>::value == 4);

    template <int V = 0, class T = void> struct Elem{ static constexpr int erverv = V; using Type = T;};

    template <typename In1, typename In2>
    struct Comparator {static constexpr bool value = In1::erverv < In2::erverv;};
    using seq = type_tuple<Elem<2, int>, Elem<-1, float>, Elem<8, bool>>;
    static_assert (min_finder_t<seq, Comparator>::erverv == -1);
    static_assert (std::is_same_v<min_finder_t<seq, Comparator>::Type, float>);
}

/////////////FIND MAXIMUM TYPE by ::value MEMBER IN TUPLE

template<class ElemsTuple, template<typename, typename> class Comparator, typename TTT = void>
struct max_finder {
    static_assert (!std::is_same_v<TTT, void>, "Use type_tuple<Elem1, Elem2 ...> please");
};

template<template<typename, typename> class Comparator, class Elem>
struct max_finder<type_tuple<Elem>, Comparator> {
    using type = Elem;
};

template<template<typename, typename> class Comparator, class ElemFirst, class ElemSecond, class ... ElemsRest>
struct max_finder<type_tuple<ElemFirst, ElemSecond, ElemsRest...>, Comparator, std::enable_if_t<(Comparator<ElemFirst, ElemSecond>::value == false)>>:
        max_finder<type_tuple<ElemFirst, ElemsRest...>, Comparator> {};

template<template<typename, typename> class Comparator, class ElemFirst, class ElemSecond, class ... ElemsRest>
struct max_finder<type_tuple<ElemFirst, ElemSecond, ElemsRest...>, Comparator, std::enable_if_t< Comparator<ElemFirst, ElemSecond>::value == true>>:
        max_finder<type_tuple<ElemSecond, ElemsRest...>, Comparator> {};

template<class Tuple, template<typename, typename> class Comparator = DefaulCompare>
using max_finder_t =  typename max_finder<Tuple, Comparator>::type;

namespace max_finder_test {
    template <int V = 0, class T = void> struct Elem{ static constexpr int erverv = V; using Type = T;};

    template <typename In1, typename In2>
    struct Comparator {static constexpr int value = In1::erverv < In2::erverv;};
    using seq = type_tuple<Elem<2, int>, Elem<16, float>, Elem<8, bool>>;
    static_assert (max_finder_t<seq, Comparator>::erverv == 16);
    static_assert (std::is_same_v<max_finder_t<seq, Comparator>::Type, float>);
}

///////////// MERGE SORT
namespace MergeSort {
template <typename L> struct in2splitter{};

template <>
struct in2splitter<type_tuple<>> {
    using first = type_tuple<>;
    using second = type_tuple<>;
};

template <typename M>
struct in2splitter<type_tuple<M>> {
    using first = type_tuple<M>;
    using second = type_tuple<>;
};

template <typename F, typename S, typename ...Rest>
struct in2splitter<type_tuple<F, S, Rest...>> {
    using splitted = in2splitter<type_tuple<Rest...>>;
    using first = tuple_cat_t<type_tuple<F>, typename splitted::first>;
    using second = tuple_cat_t<type_tuple<S>, typename splitted::second>;
};
namespace splitter_test {
    template <int i>
    using I = std::integral_constant<int, i>;
    using src1 = type_tuple<I<0>, I<1>, I<2>, I<3>, I<4> ,I<5>>;
    using src2 = type_tuple<I<0>, I<1>>;
    using splitted2 = in2splitter<src2>;
    static_assert (std::is_same_v<typename splitted2::first, type_tuple<I<0>>>);
    static_assert (std::is_same_v<typename splitted2::second, type_tuple<I<1>>>);

    using splitted1 = in2splitter<src1>;

    static_assert (std::is_same_v<typename splitted1::first, type_tuple<I<0>, I<2>, I<4>>>);
    static_assert (std::is_same_v<typename splitted1::second, type_tuple<I<1>, I<3>, I<5>>>);
}

template <template<typename, typename> class Cmp, typename L1, typename L2, typename TTT = void> struct merger {};
template <template<typename, typename> class Cmp, typename ... L2Elems> struct merger<Cmp, type_tuple<>, type_tuple<L2Elems...>> {
    using ret = type_tuple<L2Elems...>;
};
template <template<typename, typename> class Cmp, typename ... L1Elems> struct merger<Cmp, type_tuple<L1Elems...>, type_tuple<>> {
    using ret = type_tuple<L1Elems...>;
};

template <template<typename, typename> class Cmp, typename L1H, typename ... L1Elems, typename L2H, typename ... L2Elems>
struct merger<Cmp, type_tuple<L1H, L1Elems...>, type_tuple<L2H, L2Elems...>, std::enable_if_t <Cmp<L1H, L2H>::value == false> > {
    using ret = tuple_cat_t<type_tuple<L2H>, typename merger<Cmp,
        type_tuple<L1H, L1Elems...>, type_tuple<L2Elems...>
        >::ret>;
};

template <template<typename, typename> class Cmp, typename L1H, typename ... L1Elems, typename L2H, typename ... L2Elems>
struct merger<Cmp, type_tuple<L1H, L1Elems...>, type_tuple<L2H, L2Elems...>, std::enable_if_t <Cmp<L1H, L2H>::value == true> > {
    using ret = tuple_cat_t<type_tuple<L1H>, typename merger<Cmp,
        type_tuple<L1Elems...>, type_tuple<L2H, L2Elems...>
        >::ret>;
};

namespace merger_test {
    template <int i>
    using I = std::integral_constant<int, i>;
    template <typename M1, typename M2> struct Cmp {static constexpr bool value = M1::value < M2::value;};
    using l1 = type_tuple<I<1>, I<3>, I<5>, I<7>>;
    using l2 = type_tuple<I<2>, I<4>, I<6>>;
    using merged = merger<Cmp, l1, l2>::ret;

    static_assert (std::is_same_v<merged, type_tuple<I<1>, I<2>, I<3>, I<4>,I<5>, I<6>, I<7>>>);
}

template <template<typename, typename> class Cmp, typename L1> struct merge_sorter {};
template <template<typename, typename> class Cmp> struct merge_sorter<Cmp, type_tuple<>> {
    using ret = type_tuple<>;
};
template <template<typename, typename> class Cmp, typename S> struct merge_sorter<Cmp, type_tuple<S>> {
    using ret = type_tuple<S>;
};

template <template<typename, typename> class Cmp, typename F, typename S, typename ... Rest>
struct merge_sorter<Cmp, type_tuple<F, S, Rest...>> {
    using splitted = in2splitter<type_tuple<F, S, Rest...>>;
    using leftSorted = typename merge_sorter<Cmp, typename splitted::first>::ret;
    using rightSorted = typename merge_sorter<Cmp, typename splitted::second>::ret;
    using ret = typename merger<Cmp, leftSorted, rightSorted>::ret;
};

namespace merge_sorter_test {
    template <int i>
    using I = std::integral_constant<int, i>;
    template <typename M1, typename M2> struct Cmp {static constexpr bool value = M1::value < M2::value;};
    using l = type_tuple<
I<12>,
I<9>,
I<7>,
I<8>,
I<18>,
I<17>,
I<16>,
I<6>,
I<15>,
I<2>,
I<19>,
I<5>,
I<4>,
I<13>,
I<11>,
I<1>,
I<3>,
I<14>,
I<10>,
I<20>
>;
    using sorted = typename merge_sorter<Cmp, l>::ret;

    static_assert (std::is_same_v<sorted, type_tuple<
                   I<1>,
                   I<2>,
                   I<3>,
                   I<4>,
                   I<5>,
                   I<6>,
                   I<7>,
                   I<8>,
                   I<9>,
                   I<10>,
                   I<11>,
                   I<12>,
                   I<13>,
                   I<14>,
                   I<15>,
                   I<16>,
                   I<17>,
                   I<18>,
                   I<19>,
                   I<20>

                   >>);
}
}

template<class TupleT, template<typename, typename> class ValExtractor = DefaulCompare >
using sorted_tuple_t = typename MergeSort::merge_sorter<ValExtractor, TupleT>::ret;

/////////////REMOVE DUPLICATES IN SORTED TUPLE


template<class SortedTupleT, class DupsFreeTupleT, template<typename, typename> class ValExtractor,  typename TTT=void >
struct unique_only_getter {};

template<template<typename, typename> class ValExtractor, typename ...DupsFree>
struct unique_only_getter<type_tuple<>,  type_tuple<DupsFree...>, ValExtractor>{ //empty
    using type = type_tuple<DupsFree...>;
};

template<class SourceH, class ...SourceRest, template<typename, typename> class ValExtractor>
struct unique_only_getter<type_tuple<SourceH, SourceRest...>,  type_tuple<>, ValExtractor>:
        unique_only_getter<type_tuple<SourceRest...>, type_tuple<SourceH>, ValExtractor>
{
};

template<class SourceH, class ...SourceRest, class UniqueH, class ...UniqueRest ,template<typename, typename> class ValExtractor>
struct unique_only_getter<type_tuple<SourceH, SourceRest...>,  type_tuple<UniqueH, UniqueRest...>, ValExtractor,
        std::enable_if_t < ValExtractor<SourceH, UniqueH>::value == true > >
    : unique_only_getter<type_tuple<SourceRest...>, type_tuple<UniqueH, UniqueRest...>, ValExtractor> {};

template<class SourceH, class ...SourceRest, class UniqueH, class ...UniqueRest ,template<typename, typename> class ValExtractor>
struct unique_only_getter<type_tuple<SourceH, SourceRest...>,  type_tuple<UniqueH, UniqueRest...>, ValExtractor,
        std::enable_if_t < ValExtractor<SourceH, UniqueH>::value == false > >
    : unique_only_getter<type_tuple<SourceRest...>, type_tuple<SourceH, UniqueH, UniqueRest...>, ValExtractor> {};


template<class SortedTupleT, template<typename, typename> class ValExtractor = DefaulIsSame>
using unique_only_getter_t = typename unique_only_getter<SortedTupleT, type_tuple<>, ValExtractor>::type;


namespace unique_only_getter_test {
template<int V>
using C = std::integral_constant<int, V>;
using to_sort = type_tuple<
                C<5>,
                C<4>,
                C<8>,
                C<4>,
                C<5>,
                C<2>,
                C<8>
>;

using uniques = unique_only_getter_t<sorted_tuple_t<to_sort>>;

static_assert(tuple_size_v<uniques> == 4);
static_assert(std::is_same_v<uniques,  type_tuple<
              C<8>,
              C<5>,
              C<4>,
              C<2>
              >>, "Distinct fail");

template <int V = 0, class T = void> struct Elem{ static constexpr int erverv = V; using Type = T;};

template <typename In1, typename In2>
struct IsSameTest {static constexpr bool value = In1::erverv == In2::erverv;};
template <typename In1, typename In2>
struct Comparator {static constexpr bool value = In1::erverv < In2::erverv;};

using seq = type_tuple<Elem<2, int>, Elem<-1, float>, Elem<8, bool>, Elem<-1, float>, Elem<2, int>>;
using uniques2 = unique_only_getter_t<sorted_tuple_t<seq, Comparator>, IsSameTest>;
static_assert(std::is_same_v<uniques2,
              type_tuple<Elem<8, bool>, Elem<2, int>, Elem<-1, float>>
              >, "Distinct fail");

}


/////////////FILTER TYPES WITH equal ::value to given

template<template<typename> class Condition, class InputTuple, class ResultT, typename TTT=void >
struct by_value_filterer {};

template<template<typename> class Condition >
struct by_value_filterer<Condition, type_tuple<>, type_tuple<> > {
    using type = type_tuple<>;
};


template<template<typename> class Condition, class InputT, class ...ResultTypes >
struct by_value_filterer<Condition, type_tuple<InputT>, type_tuple<ResultTypes...>,
        std::enable_if_t<Condition<InputT>::value == true> > {
    using type = type_tuple<ResultTypes..., InputT>;
};

template<template<typename> class Condition, class InputT, class ...ResultTypes >
struct by_value_filterer<Condition, type_tuple<InputT>,type_tuple<ResultTypes...>,
        std::enable_if_t<Condition<InputT>::value == false> > {
    using type = type_tuple<ResultTypes...>;
};


template<template<typename> class Condition, class InputT, class ...OtherInputTs, class ...ResultTypes >
struct by_value_filterer<Condition, type_tuple<InputT, OtherInputTs...>, type_tuple<ResultTypes...>,
        std::enable_if_t<Condition<InputT>::value == true> > :
        by_value_filterer<Condition, type_tuple<OtherInputTs...>, type_tuple<ResultTypes..., InputT> >
{

};

template<template<typename> class Condition, class InputT, class ...OtherInputTs, class ...ResultTypes >
struct by_value_filterer<Condition, type_tuple<InputT, OtherInputTs...>, type_tuple<ResultTypes...>,
        std::enable_if_t<Condition<InputT>::value == false> > :
        by_value_filterer<Condition, type_tuple<OtherInputTs...>, type_tuple<ResultTypes...> >
{

};

template <template<typename> class Condition, class InputT>
using by_value_filterer_t = typename by_value_filterer<Condition, InputT, type_tuple<>>::type;

namespace by_value_filterer_test {
template<int V>
using C = std::integral_constant<int, V>;
using to_sort = type_tuple<
                C<5>,
                C<4>,
                C<8>,
                C<4>,
                C<5>,
                C<2>,
                C<8>
>;
template <class C> struct cond {static constexpr bool value = C::value == 4;};
using filtered = by_value_filterer_t<cond, to_sort>;
static_assert(tuple_size_v<filtered> == 2);
static_assert(std::is_same_v<filtered,  type_tuple<
              C<4>,
              C<4>
              >>, "Filter fail");
}


/////////////MINMAX
template <class T>
static constexpr auto StaticMin(const T & v) {
    return v;
}

template <class FirstT, class SecondT, class ...OtherT>
static constexpr auto StaticMin(const FirstT & f, const SecondT & s, const OtherT & ... others) {
    if(f < s) return StaticMin(f, others...);
    return StaticMin(s, others...);
}


template <class T>
static constexpr auto StaticMax(const T & v) {
    return v;
}

template <class FirstT, class SecondT, class ...OtherT>
static constexpr auto StaticMax(const FirstT & f, const SecondT & s, const OtherT & ... others) {
    if(f > s) return StaticMax(f, others...);
    return StaticMax(s, others...);
}


template <class ...OtherT>
static constexpr auto StaticMax(const type_tuple<OtherT...> /*args*/) {
  return StaticMax(OtherT()...);
}

/////////////REPACK WITH INDEXES//////////////
template< typename ItemType, std::size_t Index>
struct IndexedType {
    using ItemT = ItemType;
    static constexpr std::size_t I = Index;
};
template <std::size_t I, typename Out, typename ... In>
struct indexer{

};

template <std::size_t I, typename ... Items>
struct indexer<I, type_tuple<Items...>>{
    using type = type_tuple<Items...>;
};

template <std::size_t I, typename ... Items, typename ...SrcItems>
struct indexer<I, type_tuple<Items...>, type_tuple<SrcItems...>>:
        indexer<I, type_tuple<Items...>, SrcItems...>
{
};


template <std::size_t I, typename ...IndexedTypes, typename FirstItem,  typename ... OtherItems>
struct indexer<I, type_tuple<IndexedTypes...>, FirstItem,  OtherItems...>:
        indexer<I + 1, type_tuple<IndexedTypes..., IndexedType<FirstItem, I>>, OtherItems...>{
};


template <typename ... SrcT>
using indexed_types_t = typename indexer<0, type_tuple<>, SrcT...>::type;

namespace  indexed_types_test{
using indexed_types_test = indexed_types_t<type_tuple<int, float, double>>;

static_assert (tuple_elem_by_index_t<0, indexed_types_test>::I == 0);
static_assert (tuple_elem_by_index_t<1, indexed_types_test>::I == 1);
static_assert (tuple_elem_by_index_t<2, indexed_types_test>::I == 2);

static_assert (std::is_same_v<int, tuple_elem_by_index_t<0, indexed_types_test>::ItemT>);
static_assert (std::is_same_v<float, tuple_elem_by_index_t<1, indexed_types_test>::ItemT>);
static_assert (std::is_same_v<double, tuple_elem_by_index_t<2, indexed_types_test>::ItemT>);
}

template <std::size_t N, typename T, typename Out> struct repeater{};
template <std::size_t N, typename T, typename ...OutT> struct repeater<N, T, type_tuple<OutT...>>:
    repeater<N-1, T, type_tuple<OutT..., T> >
{};

template <typename T, typename ...OutT> struct repeater<0, T, type_tuple<OutT...>> {
    using type = type_tuple<OutT...>;
};
template<std::size_t N, typename T>
using repeater_t = typename repeater<N, T, type_tuple<>>::type;
namespace repeater_t_test {
static_assert (std::is_same_v<repeater_t<5, bool>, type_tuple<bool, bool, bool, bool, bool>> );
}


/////////////TUPLE ITERATION/////////////

template<class F, class...Ts, std::size_t...Is, class ...Args>
void iterateTuple(std::tuple<Ts...> & tuple, F && func, std::index_sequence<Is...>, Args &&...args){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple), std::forward<Args>(args)...), 0)... };
}

template<class F, class...Ts, class ...Args>
void iterateTuple(std::tuple<Ts...> & tuple, F && func, Args &&...args){
    iterateTuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>(), std::forward<Args>(args)...);
}

template<class F, class...Ts, std::size_t...Is, class ...Args>
void iterateTuple(const std::tuple<Ts...> & tuple, F && func, std::index_sequence<Is...>, Args &&...args){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple), std::forward<Args>(args)...), 0)... };
}

template<class F, class...Ts, class ...Args>
void iterateTuple(const std::tuple<Ts...> & tuple, F && func, Args &&...args){
    iterateTuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>(), std::forward<Args>(args)...);
}


/////////////TypeIndex sequence

template <typename>  struct typeindex_sequence{};
template <std::size_t ... Is>  struct typeindex_sequence<std::index_sequence<Is...>>{
    using type = type_tuple<std::integral_constant<std::size_t, Is>...>;
};
template <std::size_t N>
using make_typeindex_sequence = typename typeindex_sequence<std::make_index_sequence<N>>::type;

}


#endif // TYPECALC2_HPP
