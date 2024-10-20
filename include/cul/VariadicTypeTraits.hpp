namespace cul
{

namespace variadic_tt
{

template <template <class, class> class BinPred, class... Ts>
struct All;

template <template <class, class> class BinPred, class A, class B, class... Ts>
struct All<BinPred, A, B, Ts...>
{
    static constexpr bool value{ BinPred<A, B>::value &&
                                 All<BinPred, B, Ts...>::value };
};

template <template <class, class> class BinPred, class T>
struct All<BinPred, T>
{
    static constexpr bool value{ true };
};

template <template <class, class> class BinPred>
struct All<BinPred>
{
    static constexpr bool value{ true };
};

template <template <class, class> class BinPred, class... Ts>
using All_v = All<BinPred, Ts...>::value;

template <template <class, class> class BinPred, class... Ts>
struct Exist;

template <template <class, class> class BinPred, class A, class B, class... Ts>
struct Exist<BinPred, A, B, Ts...>
{
    static constexpr bool value{ BinPred<A, B>::value ||
                                 All<BinPred, B, Ts...>::value };
};

template <template <class, class> class BinPred, class T>
struct Exist<BinPred, T>
{
    static constexpr bool value{ false };
};

template <template <class, class> class BinPred>
struct Exist<BinPred>
{
    static constexpr bool value{ true };
};

template <template <class, class> class BinPred, class... Ts>
using Exist_v = Exist<BinPred, Ts...>::value;

} // namespace variadic_tt

} // namespace cul
