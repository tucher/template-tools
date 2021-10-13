#include <utility>

template<class FunctorTFirst, class ... FunctorTRest> 
struct composition: FunctorTFirst, composition<FunctorTRest...> {
    template <class ArgsT> auto constexpr operator()(ArgsT && arg) {
        return composition<FunctorTRest...>::operator()( 
                  FunctorTFirst::operator() (std::forward<ArgsT>(arg) ) 
        );
    }
};
template<class  FunctorT> 
struct composition<FunctorT>: FunctorT {
     using FunctorT::operator();
};
template<class... Ts> composition(Ts...) -> composition<Ts...>;

struct TT1 {int v;};
struct TT2 {int v;};
struct TT3 {int v;};
struct TT4 {int v;};

auto comp2 = composition{
    [](TT1 && p) { return TT2{p.v + 1}; },
    [](TT2 && p) { return TT3{p.v + 1}; },
    [](TT3 && p) { return TT4{p.v + 1}; }
};
static_assert (std::same_as<decltype (comp2(TT1{2})), TT4>);
static_assert (comp2(TT1{2}).v == 5);