#ifndef TRIE_HPP
#define TRIE_HPP

#include "../typecalc/typecalc2.hpp"
//#include "../typecalc/typecalc.hpp"
/*
Requirements to StringLike objects, are:

    struct StringLike {

    // Length of string
        static constexpr std::size_t Size = 42;

    // Method to get a character (need not to be char, any char-like type can be here)
        template<std::size_t i>
        constexpr static char get() {
            return 'f';
        }
    };

    // Globally available compare operator (must work like lexicographical compare)
    template <SSSurrogate1,  SSSurrogate2>
    constexpr bool operator<(SSSurrogate1, SSSurrogate2) {
        return false;
    }
*/
#define INDEXEDVERSION
#ifdef INDEXEDVERSION
template <typename ... SrcStrings>
struct StaticTrie
{
    using SourceStringsTuple = TypeCalc::type_tuple<SrcStrings...>;

    template<class Ind> using get_str = typename TypeCalc::tuple_elem_by_index_t<Ind::value, SourceStringsTuple>;
    template <class StringI, std::size_t CharI, typename TTT=void > struct ss_char_getter{};

    template <class StringI, std::size_t CharI>
    struct ss_char_getter<StringI, CharI, std::enable_if_t<CharI < get_str<StringI>::Size> >{
        static constexpr auto value = get_str<StringI>::template get<CharI>();
    };

    template <class StringI, std::size_t CharI>
    struct ss_char_getter<StringI, CharI, std::enable_if_t<CharI >= get_str<StringI>::Size> >{
        static constexpr auto value = 0;
    };

    template <class StringI, std::size_t CharI> static constexpr auto ss_char_getter_v = ss_char_getter<StringI, CharI>::value;

    template <typename Indexes, std::size_t LayerI> struct layer_max_str_len {
        template <typename I1, typename I2> struct StringSizeComparator {
            static constexpr bool value = get_str<I1>::Size < get_str<I2>::Size;
        };
        using max_index = TypeCalc::max_finder_t<Indexes, StringSizeComparator>;
        static constexpr std::size_t value =  get_str<max_index>::Size;
    };

    template <std::size_t I, typename ThisStringsIndexes, typename TTT=void >
    struct TrieLayer{ };

    template <std::size_t I, typename ...StringsIndexes> struct TrieLayer<I, TypeCalc::type_tuple<StringsIndexes...>,
            std::enable_if_t< (sizeof...(StringsIndexes) > 1 && I < layer_max_str_len<TypeCalc::type_tuple<StringsIndexes...>, I>::value) >  >
    {
        using InputStringIndexes = TypeCalc::type_tuple<StringsIndexes ...>;
        template <typename S1, typename S2> struct StringComparator {
            static constexpr bool value = (get_str<S1>() < get_str<S2>());
        };
        using ThisStringsIndexes = TypeCalc::sorted_tuple_t <InputStringIndexes, StringComparator>;

        static constexpr std::size_t NodeStringIndex = TypeCalc::tuple_elem_by_index_t<0, ThisStringsIndexes>::value;
        static constexpr bool HasFullString = I >= TypeCalc::tuple_elem_by_index_t<NodeStringIndex, SourceStringsTuple>::Size;

        private:

        template<typename I1, typename I2>  struct comparator {
            static constexpr bool value = ss_char_getter_v<I1, I> < ss_char_getter_v<I2, I>;
        };

        template<typename I1, typename I2>
        struct char_Is_same {
            static constexpr bool value =
                    ss_char_getter_v<I1, I> == ss_char_getter_v<I2, I>;};

        using keys = TypeCalc::unique_only_getter_t<TypeCalc::sorted_tuple_t<InputStringIndexes, comparator>, char_Is_same>;


        template <typename KeyStringIndexes>
        struct LayerSplitter{};

        template<auto K, class V> struct ChildLayerT {
            static constexpr auto Key = K;
            using Layer = V;
        };

        template <auto Key> struct split_helper{
            template <typename StrIndex> struct Filter {
                static constexpr bool value = ss_char_getter_v<StrIndex, I> == Key;
            };
            using type = TypeCalc::by_value_filterer_t<Filter, InputStringIndexes>;
        };

        template <typename ...KeyStringIndexes>
        struct LayerSplitter<TypeCalc::type_tuple<KeyStringIndexes...>>{
            using type = TypeCalc::type_tuple<
                ChildLayerT<
                    ss_char_getter_v<KeyStringIndexes, I>,
                    TrieLayer<I+1, typename split_helper<ss_char_getter_v<KeyStringIndexes, I>>::type >
                >...
            >;
        };

        public:
        using NextNodes = typename LayerSplitter<keys>::type;
        static constexpr bool Last = false;
    };


    template <std::size_t I,  typename ...StringsIndexes >
    struct TrieLayer<I, TypeCalc::type_tuple<StringsIndexes...>,
            std::enable_if_t< (sizeof...(StringsIndexes) == 1 || I == layer_max_str_len<TypeCalc::type_tuple<StringsIndexes...>, I>::value) >
    >
    {
        using InputStringIndexes = TypeCalc::type_tuple<StringsIndexes ...>;
        using ThisStringsIndexes = InputStringIndexes;
        static constexpr std::size_t NodeStringIndex = TypeCalc::tuple_elem_by_index_t<0, ThisStringsIndexes>::value;

        static_assert (sizeof... (StringsIndexes) == 1, "Trie error: looks like there are identical strings in source tuple");
        using NextNodes = TypeCalc::type_tuple<>;
        static constexpr bool HasFullString = true;
        static constexpr bool Last = true;
    };


    static_assert(true, "Duplicates in source string tuple are not allowed"); //TODO
    using L = TrieLayer<0, TypeCalc::make_typeindex_sequence<sizeof...(SrcStrings)>>;

    template<bool IsLast, bool HasFull, std::size_t I, typename MatchedIndexesSeq>
    struct MatchRes{
        using MatchedStringsIndexes = MatchedIndexesSeq;
        using NodeString = typename TypeCalc::tuple_elem_by_index_t<I, SourceStringsTuple>;
        static constexpr bool hasFull = HasFull;
        static constexpr bool isLast = IsLast;
        static constexpr std::size_t index = I;
    };
    template<class CharType, class Clb>
    static void search (CharType symbol, Clb clb ) {
        bool to_continue = true;
        auto searcher= [&]( auto * layer, const auto & self) -> void{
            using Item =  std::remove_pointer_t<decltype (layer)>;
            using LT = typename Item::Layer;
            constexpr CharType key = Item::Key;

            if(!to_continue || key != symbol) return;

            symbol = clb(MatchRes<LT::Last, LT::HasFullString, LT::NodeStringIndex, typename LT::ThisStringsIndexes>{}
                              );
            if(symbol==-1) to_continue = false;

            if constexpr(LT::Last) to_continue = false;
            if(to_continue) {
                TypeCalc::iterateTypeTuple((typename LT::NextNodes*)nullptr, self, self);
            }
        };
        TypeCalc::iterateTypeTuple((typename L::NextNodes*)nullptr, searcher, searcher);
    }
};

template <typename ... SrcStrings>
struct StaticTrie<TypeCalc::type_tuple<SrcStrings...>>: StaticTrie<SrcStrings...>{};

#else
namespace StaticTrieImpl {


template <std::size_t I, typename  SString, typename TTT=void >
struct ss_char_getter{};

template <std::size_t I, typename  SString>
struct ss_char_getter<I, SString, std::enable_if_t<I < SString::Size> >{
    static constexpr auto value = SString::template get<I>();
};

template <std::size_t I, typename  SString>
struct ss_char_getter<I, SString, std::enable_if_t<I >= SString::Size> >{
    static constexpr auto value = 0;
};

template <std::size_t I, typename  SString>
constexpr auto ss_char_getter_v = ss_char_getter<I, SString>::value;

template <std::size_t CurrentIndex, typename KeyIndexedStrings, typename SourceIndexedStrings>
struct LayerSplitter{};

template <std::size_t I, typename IndexedStrings, typename TTT=void >
struct TrieLayer{};


template<auto K, class V> struct ChildLayerT {
    static constexpr auto Key = K;
    using Layer = V;
};

template <std::size_t I, auto Key, typename AllIndexedStrings> struct split_helper{};
template <std::size_t I, auto Key, typename ...AllIndexedStrings> struct split_helper<I, Key, TypeCalc::type_tuple<AllIndexedStrings...>>{
    template <typename IndexedStr> struct Filter {
        static constexpr bool value = ss_char_getter_v<I, typename IndexedStr::ItemT> == Key;
    };
    using type = TypeCalc::by_value_filterer_t<Filter, TypeCalc::type_tuple<AllIndexedStrings...>>;
};

template < std::size_t I, typename ...KeyIndexedStrings, typename ...AllIndexedStrings>
struct LayerSplitter<I, TypeCalc::type_tuple<KeyIndexedStrings...>, TypeCalc::type_tuple<AllIndexedStrings...>>{
    using type = TypeCalc::type_tuple<
        ChildLayerT<
            ss_char_getter_v<I, typename KeyIndexedStrings::ItemT>,
            TrieLayer<I+1, typename split_helper<I,ss_char_getter_v<I, typename KeyIndexedStrings::ItemT>, TypeCalc::type_tuple<AllIndexedStrings...> >::type >
        >...
    >;
};

template <typename T> struct SizeExtractor {static constexpr std::size_t value = T::ItemT::Size;};
template <typename S1, typename S2> struct StringComparator {
    static constexpr bool value = !(typename S1::ItemT() < typename S2::ItemT());
}; // TODO
template <typename S1, typename S2, std::size_t Index> struct CharAtIndexComparator {
    static constexpr bool value = ss_char_getter_v<Index, typename S1::ItemT> < ss_char_getter_v<Index, typename S2::ItemT>;
};

template <typename S1, typename S2> struct StringSizeComparator {
    static constexpr bool value = S1::ItemT::Size < S2::ItemT::Size;
};

template <std::size_t I,  typename ... StringWithIndex>
struct TrieLayer<I, TypeCalc::type_tuple<StringWithIndex...>,
        std::enable_if_t< (sizeof...(StringWithIndex) > 1 && I < TypeCalc::max_finder_t<TypeCalc::type_tuple<StringWithIndex...>,StringSizeComparator >::ItemT::Size) >
>
{
    using InputStringTuple = TypeCalc::type_tuple<StringWithIndex ...>;

    using ThisStrings = TypeCalc::sorted_tuple_t  <
        InputStringTuple,
        StringComparator
    >;
    using NodeString = typename TypeCalc::tuple_elem_by_index_t<0, ThisStrings>;
    static constexpr bool HasFullString = I >= NodeString::ItemT::Size;

    private:
    template<typename S1, typename S2>
    using comparator = CharAtIndexComparator<S1, S2, I>;
    template<typename S1, typename S2> struct char_extractor {
        static constexpr bool value = ss_char_getter<I, typename S1::ItemT>::value == ss_char_getter<I, typename S2::ItemT>::value;
    };
    using keys = TypeCalc::unique_only_getter_t<TypeCalc::sorted_tuple_t<InputStringTuple, comparator>, char_extractor>;

    public:
    using NextNodes = typename LayerSplitter<I, keys, InputStringTuple>::type;
    static constexpr bool Last = TypeCalc::tuple_size_v<NextNodes> == 0;
};

template <std::size_t I, typename ... StringWithIndex>
struct TrieLayer<I, TypeCalc::type_tuple<StringWithIndex...>,
        std::enable_if_t< (sizeof...(StringWithIndex) == 1 || I == TypeCalc::max_finder_t<TypeCalc::type_tuple<StringWithIndex...>,StringSizeComparator >::ItemT::Size) >
>
{
    using InputStringTuple = TypeCalc::type_tuple<StringWithIndex ...>;
    using ThisStrings = InputStringTuple;
    using NodeString = TypeCalc::tuple_elem_by_index_t<0, ThisStrings>;

    static_assert (sizeof... (StringWithIndex) == 1, "Trie error: looks like there are identical strings in source tuple");
    using NextNodes = TypeCalc::type_tuple<>;
    static constexpr bool HasFullString = true;
    static constexpr bool Last = true;
};

template <typename Src,  typename V = void>
struct StaticTrie{};

template <typename ... IndexedStrings>
struct StaticTrie<TypeCalc::type_tuple<IndexedStrings...>>
{
    static_assert(true, "Duplicates in source string tuple are not allowed"); //TODO
    using L = TrieLayer<0, TypeCalc::type_tuple<IndexedStrings...>>;

    template<bool IsLast, bool HasFull, std::size_t I, typename StringsT, typename NString>
    struct MatchRes{
        using MatchedStrings = StringsT;
        using SourceStrings = TypeCalc::type_tuple<IndexedStrings...>;
        using NodeString = NString;
        static constexpr bool hasFull = HasFull;
        static constexpr bool isLast = IsLast;
        static constexpr std::size_t index = I;
    };
    template<class CharType, class Clb>
    static void search (CharType symbol, Clb clb ) {
        bool to_continue = true;
        auto searcher= [&]( auto * layer, const auto & self) -> void{
            using Item =  std::remove_pointer_t<decltype (layer)>;
            using LT = typename Item::Layer;
            constexpr CharType key = Item::Key;

            if(!to_continue || key != symbol) return;

            symbol = clb(MatchRes<LT::Last, LT::HasFullString,
                              LT::NodeString::I,
                              typename LT::ThisStrings, typename LT::NodeString::ItemT>{}
                              );
            if(symbol==-1) to_continue = false;

            if constexpr(LT::Last) to_continue = false;
            if(to_continue) {
                TypeCalc::iterateTypeTuple((typename LT::NextNodes*)nullptr, self, self);
            }
        };
        TypeCalc::iterateTypeTuple((typename L::NextNodes*)nullptr, searcher, searcher);
    }
};

}
template <typename ...SStrings>
using StaticTrie =StaticTrieImpl::StaticTrie<TypeCalc::indexed_types_t<SStrings...>>;
#endif

#endif // TRIE_HPP
