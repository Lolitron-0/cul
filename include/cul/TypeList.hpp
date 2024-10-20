#pragma once
#include <type_traits>
#include <utility>

namespace cul
{
namespace typelist
{

struct NullType
{
};

template <class... T>
struct TypeList
{
    using Head = NullType;
    using Tail = NullType;
};

using EmptyTypeList = TypeList<>;

template <class H, class... T>
struct TypeList<H, T...>
{
    using Head = H;
    using Tail = TypeList<T...>;
};

template <class TL>
struct IsEmpty : std::true_type
{
};

template <>
struct IsEmpty<EmptyTypeList> : std::true_type
{
};

template <class... Args>
struct IsEmpty<TypeList<Args...>>
    : std::integral_constant<
          bool,
          std::is_same<typename TypeList<Args...>::Head, NullType>::value &&
              IsEmpty<typename TypeList<Args...>::Tail>::
                  value> // inheriting whole constant type to have ::value
{
};

template <class TL>
struct Length : std::integral_constant<unsigned int, 0>
{
};

template <class... Args>
struct Length<TypeList<Args...>>
    : std::integral_constant<
          unsigned int,
          IsEmpty<TypeList<Args...>>::value
              ? 0
              : 1 + Length<typename TypeList<Args...>::Tail>::value>
{
};

template <unsigned int N, class TL>
struct TypeAt
{
    using type = NullType;
};

template <class... Args>
struct TypeAt<0, TypeList<Args...>>
{
    using type = typename TypeList<Args...>::Head;
};

template <unsigned int N, class... Args>
struct TypeAt<N, TypeList<Args...>>
{
    static_assert(N > Length<TypeList<Args...>>::value);
    using type = typename TypeAt<N - 1, typename TypeList<Args...>::Tail>::type;
};

template <class T, class TL>
struct Contains : std::false_type
{
};

template <class... Args>
struct Contains<NullType, TypeList<Args...>> : std::false_type
{
};

template <class T, class... Args>
struct Contains<T, TypeList<Args...>>
    : std::integral_constant<
          bool, std::is_same<T, typename TypeList<Args...>::Head>::value ||
                    Contains<T, typename TypeList<Args...>::Tail>::value>
{
};

/**
 * @brief Helper type to iterate through TypeList
 * @tparam StaicFunctor is a (template) class/struct with static void Call(...)
 * function
 */
template <class TL, template <class T> class StaticFunctor>
struct ForEach;


template <template <class T> class StaticFunctor>
struct ForEach<TypeList<>, StaticFunctor>
{
public:
    template <class... TArgs>
    static void Iterate(auto&&... args)
    {
    }
};

template <class Last, template <class T> class StaticFunctor>
struct ForEach<TypeList<Last>, StaticFunctor>
{
public:
    template <class... Args>
    static void Iterate(Args&&... args)
    {
        StaticFunctor<Last>::Call(std::forward<Args>(args)...);
    }
};

template <class Hd, class... Tl, template <class T> class StaticFunctor>
struct ForEach<TypeList<Hd, Tl...>, StaticFunctor>
{
public:
    template <class... Args>
    static void Iterate(Args&&... args)
    {
        StaticFunctor<Hd>::Call(std::forward<Args>(args)...);
        ForEach<TypeList<Tl...>, StaticFunctor>::Iterate(
            std::forward<Args>(args)...);
    }
};

template <class TL, template <class T> class StaticFunctor>
struct ForEachTemplate;


template <template <class T> class StaticFunctor>
struct ForEachTemplate<TypeList<>, StaticFunctor>
{
public:
    template <class... TArgs>
    static void Iterate(auto&&... args)
    {
    }
};

template <class Last, template <class T> class StaticFunctor>
struct ForEachTemplate<TypeList<Last>, StaticFunctor>
{
public:
    template <class... TArgs>
    static void Iterate(auto&&... args)
    {
        StaticFunctor<Last>::template Call<TArgs...>(args...);
    }
};

template <class Hd, class... Tl, template <class T> class StaticFunctor>
struct ForEachTemplate<TypeList<Hd, Tl...>, StaticFunctor>
{
public:
    template <class... TArgs>
    static void Iterate(auto&&... args)
    {
        StaticFunctor<Hd>::template Call<TArgs...>(args...);
        ForEach<TypeList<Tl...>, StaticFunctor>::Iterate(args...);
    }
};

} // namespace typelist
} // namespace cul
