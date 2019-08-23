#ifndef TYPE_STRING_H
#define TYPE_STRING_H

#include <type_traits>
#include <cstddef>
#include <utility>
#include <array>
#include <cstdint>
#include <cstring>
namespace TypeString {
using CharType = char;

template <CharType... chars>
using tstring = std::integer_sequence<CharType, chars...>;

template <typename>
struct TypeString;

template <CharType... elements>
struct TypeString<tstring<elements...>> {
public:
    static constexpr std::size_t Size = sizeof... (elements);
private:
    template < int i, int cur, CharType t_c, CharType... ttt_c>
    static  constexpr CharType get_impl() {
        if constexpr(cur == i) return t_c;
        else return get_impl<i, cur+1, ttt_c...>();
    }
    template < int i, int cur>
    static  constexpr CharType get_impl() {
        return 0;
    }

    template <CharType Input>
    static constexpr std::uint8_t HexCharToInt()
    {
        static_assert (
                    (Input >= 'a' && Input <= 'f') ||
                    (Input >= 'A' && Input <= 'F') ||
                    (Input >= '0' && Input <= '9')
                    , "Char should be a-f or A-F or 0-9");
        if constexpr (Input >= 'a' && Input <= 'f') return Input - 'a';
        else if constexpr((Input >= 'A') && (Input <= 'F')) return Input - 'A';
        else if constexpr((Input >= '0') && (Input <= '0')) return Input - '0';
        else return 0;
    }

    template <CharType high, CharType low>
    static  constexpr std::uint8_t ByteFromHex() {
        return (HexCharToInt<high>() << 4) | (HexCharToInt<low>());
    }
    template<std::size_t... Index>
    static  constexpr std::array<std::uint8_t, Size/2> hex_to_bytes_helper(std::index_sequence<Index...>) {
        return {ByteFromHex(get<Index*2>(), get<Index*2+1>())...};
    }
public:
    template <int i>
    static constexpr CharType get() {
        if constexpr(i < Size) return  get_impl<i, 0, elements...>();
        else return 0;
    }

    static  constexpr std::array<std::uint8_t, Size/2> hex_to_bytes() {
        static_assert (Size % 2 == 0, "Length should be even");
        return hex_to_bytes_helper(std::make_index_sequence<Size/2>());
    }

    static constexpr std::uint32_t hash() {
        std::uint32_t hash = 5381;
        for(std::size_t i = 0; i < Size; i ++) {
            hash = ((hash << 5) + hash) + std::uint32_t(get<i>());
        }
        return hash;
    }
    static CharType const * c_str() {
        static constexpr CharType  r[] = {elements..., '\0'};
        return r;
    }
};

template <typename> struct is_ss : std::false_type { };
template <typename T> struct is_ss<TypeString<T>> : std::true_type { };

template <typename T>
constexpr bool is_ss_v = is_ss<T>::value;

template <typename S1,  typename S2, std::size_t... I1, std::size_t... I2>
constexpr auto  concatHelper(std::index_sequence<I1...>, std::index_sequence<I2...>) {
    return TypeString<tstring<S1:: template get<I1>()..., S2:: template get<I2>()...>>();
}

template <typename S1, typename S2, typename = typename std::enable_if_t<is_ss_v<S1> && is_ss_v<S2>>>
constexpr auto operator+(S1, S2) {
    using s1indexes = std::make_index_sequence<S1::Size>;
    using s2indexes = std::make_index_sequence<S2::Size>;
    return concatHelper<S1, S2>(s1indexes(), s2indexes());
}

template <typename S1,  typename ...Rest>
struct concat_h {
    typedef decltype(S1() + typename concat_h<Rest...>::result()) result;
};

template<typename S1>
struct concat_h<S1> {
    typedef S1 result;
};

template <typename ...Args>
using concat = typename concat_h<Args...>::result;

template <typename S1,  typename S2, std::size_t...Is>
constexpr bool  compareHelper(std::index_sequence<Is...>) {
    return (true && ... && (S2::template get<Is>() == S1::template get<Is>()));
}

template <typename S1, typename S2, typename = typename std::enable_if<is_ss_v<S1> && is_ss_v<S2>>::type>
constexpr bool operator==(S1, S2) {
    if constexpr(S1::Size != S2::Size) return false;
    else
        return compareHelper<S1, S2>(std::make_integer_sequence<std::size_t, S2::Size>());
}


template <std::size_t cur, std::size_t M, typename S1, typename S2, class TTT=void> struct lex_comparator;
template <std::size_t cur, std::size_t M, typename S1, typename S2>
struct lex_comparator<cur, M, S1, S2, std::enable_if_t<(cur < M && S1::template get<cur>() < S2::template get<cur>())>>

{
    static constexpr bool value = true;
};

template <std::size_t cur, std::size_t M, typename S1, typename S2>
struct lex_comparator<cur, M, S1, S2, std::enable_if_t<(cur < M && S1::template get<cur>() > S2::template get<cur>())>>
{
    static constexpr bool value = false
        ;
};
template <std::size_t cur, std::size_t M, typename S1, typename S2>
struct lex_comparator<cur, M, S1, S2, std::enable_if_t<(cur < M && S1::template get<cur>() == S2::template get<cur>())>>:
lex_comparator<cur +1, M, S1, S2>{}                                                                                                ;

template <std::size_t cur, std::size_t M, typename S1, typename S2>
struct lex_comparator<cur, M, S1, S2, std::enable_if_t<(cur == M )>>
{
    static constexpr bool value = false;
};


template <typename S1, typename S2, typename = typename std::enable_if<is_ss_v<S1> && is_ss_v<S2>>::type>
constexpr bool operator<(S1, S2) {
    return lex_comparator<0, S1::Size < S2::Size? S2::Size: S1::Size, S1, S2>::value;
}

template <typename S1, typename = typename std::enable_if<is_ss_v<S1>>::type>
constexpr auto operator==(S1, const CharType* str) {
    std::size_t l = std::strlen(str);
    if(S1::Size != l) return false;
    return strncmp(str, S1::c_str(), S1::Size) == 0;
}

template <typename S1,  typename S2>
inline constexpr bool compare_v = (S1() == S2());

constexpr int abs_val (std::int64_t x)
{ return x < 0 ? -x : x; }

constexpr int num_digits (std::int64_t x)
{ return x < 0 ? 1 + num_digits (-x) : x < 10 ? 1 : 1 + num_digits (x / 10); }

template<int size, std::int64_t x, CharType... args>
struct numeric_builder {
    typedef typename numeric_builder<size - 1, x / 10, '0' + abs_val (x) % 10, args...>::type type;
};

template<std::int64_t x, CharType... args>
struct numeric_builder<2, x, args...> {
    typedef TypeString<tstring< x < 0 ? '-' : '0' + x / 10, '0' + abs_val (x) % 10, args...> > type;
};

/* special case for one digit (positive numbers only) */
template<std::int64_t x, CharType... args>
struct numeric_builder<1, x, args...> {
    typedef TypeString<tstring<'0' + x, args...> > type;
};


template <std::int64_t x>
using i_to_ss = typename numeric_builder<num_digits (x), x>::type;

template <CharType Filler, std::size_t N, CharType... args>
struct fill_ss_builder {
    using type = typename fill_ss_builder<Filler, N-1, Filler, args...>::type;
};

template <CharType Filler, CharType... args>
struct fill_ss_builder<Filler, 0, args...> {
    using type = TypeString<tstring<args...> >;
};

template<CharType Ch, std::size_t N>
using filled_ss = typename fill_ss_builder<Ch, N>::type;

template<std::size_t Len, int Index>
constexpr int ss_to_i_helper() {
    return 0;
}

constexpr std::int64_t pow10(int i) {
    std::int64_t ret = 1;
    for(int ii = 0; ii < i ; ii ++) ret *= 10;
    return ret;
}

template<std::size_t Len, int Index, CharType ch, CharType ...chars>
constexpr std::int64_t ss_to_i_helper() {
    if constexpr(ch == '-')
        return -ss_to_i_helper<sizeof...(chars), 0, chars...>();
    return std::int64_t(ch-'0') * pow10(Len - Index - 1) + ss_to_i_helper<Len, Index +1, chars...>();
}

template <CharType... chars>
constexpr std::int64_t ss_to_i(TypeString<tstring<chars...>> ) {
    return ss_to_i_helper<sizeof...(chars), 0, chars...>();
}

template <typename T>
constexpr std::int64_t ss_to_i_v = ss_to_i(T());

}

namespace TypeStringLiteralExploder {
    template <typename T, T... chars>
    constexpr TypeString::TypeString<TypeString::tstring<chars...>> operator "" _ts() { return { }; }
}

#define TS(str) decltype(str ## _ts)

#endif // TYPE_STRING_H
