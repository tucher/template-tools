#ifndef TRIE_HPP
#define TRIE_HPP

#include "../typecalc/typecalc2.hpp"
#include "../typecalc/typecalc.hpp"
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


template <typename ... SrcStrings>
struct StaticTrie
{
    using SourceStringsTuple = TypeCalc2::type_tuple<SrcStrings...>;

    template<class Ind> using get_str = typename TypeCalc2::tuple_elem_by_index_t<Ind::value, SourceStringsTuple>;
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
        using max_index = TypeCalc2::max_finder_t<Indexes, StringSizeComparator>;
        static constexpr std::size_t value =  get_str<max_index>::Size;
    };

    template <std::size_t I, typename ThisStringsIndexes, typename TTT=void >
    struct TrieLayer{ };

    template <std::size_t I, typename ...StringsIndexes> struct TrieLayer<I, TypeCalc2::type_tuple<StringsIndexes...>,
            std::enable_if_t< (sizeof...(StringsIndexes) > 1 && I < layer_max_str_len<TypeCalc2::type_tuple<StringsIndexes...>, I>::value) >  >
    {
        using InputStringIndexes = TypeCalc2::type_tuple<StringsIndexes ...>;
        template <typename S1, typename S2> struct StringComparator {
            static constexpr bool value = (get_str<S1>() < get_str<S2>());
        };
        using ThisStringsIndexes = TypeCalc2::sorted_tuple_t <InputStringIndexes, StringComparator>;

        static constexpr std::size_t NodeStringIndex = TypeCalc2::tuple_elem_by_index_t<0, ThisStringsIndexes>::value;
        static constexpr bool HasFullString = I >= TypeCalc2::tuple_elem_by_index_t<NodeStringIndex, SourceStringsTuple>::Size;

        private:

        template<typename I1, typename I2>  struct comparator {
            static constexpr bool value = ss_char_getter_v<I1, I> < ss_char_getter_v<I2, I>;
        };

        template<typename I1, typename I2>
        struct char_Is_same {
            static constexpr bool value =
                    ss_char_getter_v<I1, I> == ss_char_getter_v<I2, I>;};

        using keys = TypeCalc2::unique_only_getter_t<TypeCalc2::sorted_tuple_t<InputStringIndexes, comparator>, char_Is_same>;


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
            using type = TypeCalc2::by_value_filterer_t<Filter, InputStringIndexes>;
        };

        template <typename ...KeyStringIndexes>
        struct LayerSplitter<TypeCalc2::type_tuple<KeyStringIndexes...>>{
            using type = TypeCalc2::type_tuple<
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
    struct TrieLayer<I, TypeCalc2::type_tuple<StringsIndexes...>,
            std::enable_if_t< (sizeof...(StringsIndexes) == 1 || I == layer_max_str_len<TypeCalc2::type_tuple<StringsIndexes...>, I>::value) >
    >
    {
        using InputStringIndexes = TypeCalc2::type_tuple<StringsIndexes ...>;
        using ThisStringsIndexes = InputStringIndexes;
        static constexpr std::size_t NodeStringIndex = TypeCalc2::tuple_elem_by_index_t<0, ThisStringsIndexes>::value;

        static_assert (sizeof... (StringsIndexes) == 1, "Trie error: looks like there are identical strings in source tuple");
        using NextNodes = TypeCalc2::type_tuple<>;
        static constexpr bool HasFullString = true;
        static constexpr bool Last = true;
    };


    static_assert(true, "Duplicates in source string tuple are not allowed"); //TODO
    using L = TrieLayer<0, TypeCalc2::make_typeindex_sequence<sizeof...(SrcStrings)>>;

    template<bool IsLast, bool HasFull, std::size_t I, typename MatchedIndexesSeq>
    struct MatchRes{
        using MatchedStringsIndexes = MatchedIndexesSeq;
        using NodeString = typename TypeCalc2::tuple_elem_by_index_t<I, SourceStringsTuple>;
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
                TypeCalc2::iterateTypeTuple((typename LT::NextNodes*)nullptr, self, self);
            }
        };
        TypeCalc2::iterateTypeTuple((typename L::NextNodes*)nullptr, searcher, searcher);
    }
};

template <typename ... SrcStrings>
struct StaticTrie<TypeCalc2::type_tuple<SrcStrings...>>: StaticTrie<SrcStrings...>{};
#endif // TRIE_HPP
