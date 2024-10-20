// copyright userver

#pragma once
#include <cassert>
#include <csignal>
#include <optional>
#include <string_view>
#include <type_traits>

namespace cul
{

namespace internal
{

#ifdef SIGTRAP
#define DEBUG_BREAK(...) std::raise(SIGTRAP);
#else
#define DEBUG_BREAK(...) std::raise(SIGABRT);
#endif

constexpr bool EqualIgnoreCaseLowercase(std::string_view lowercase,
                                        std::string_view other) noexcept
{
    const auto size{ lowercase.size() };
    for (int32_t i{ 0 }; i < size; i++)
    {
        const auto lowercaseChar{ lowercase[i] };
        if (lowercaseChar != other[i])
        {
            if (!('a' <= lowercaseChar && lowercaseChar <= 'z') ||
                lowercaseChar != other[i] + ('a' - 'A'))
            {
                return false;
            }
        }
    }

    return true;
}

constexpr bool ContainsUppercaseAscii(std::string_view sv)
{
    for (auto c : sv)
    {
        if ('A' <= c && c <= 'Z')
        {
            return true;
        }
    }
    return false;
}

template <class First, class Second>
struct SwitchTypeDetectionResult
{
    using first_type = First;
    using second_type = Second;
    constexpr SwitchTypeDetectionResult& Case(First, Second) noexcept
    {
        return *this;
    }
};

struct SwitchTypesDetector
{
    template <class First, class Second>
    constexpr auto Case(First, Second) noexcept
    {
        using first_type =
            std::conditional_t<std::is_convertible_v<First, std::string_view>,
                               std::string_view, First>;
        using second_type =
            std::conditional_t<std::is_convertible_v<Second, std::string_view>,
                               std::string_view, Second>;
        return SwitchTypeDetectionResult<first_type, second_type>{};
    }
};

template <class First, class Second>
class SwitchByFirst
{
public:
    constexpr explicit SwitchByFirst(First first) noexcept
        : m_Key{ first }
    {
    }

    constexpr SwitchByFirst& Case(First first, Second second) noexcept
    {
        if (!m_Result && m_Key == first)
        {
            m_Result.emplace(second);
        }
        return *this;
    }

    constexpr std::optional<Second> GetResult() noexcept
    {
        return m_Result;
    }

private:
    First m_Key;
    std::optional<Second> m_Result;
};

template <class First, class Second>
class SwitchBySecond
{
public:
    constexpr explicit SwitchBySecond(Second second) noexcept
        : m_Key{ second }
    {
    }

    constexpr SwitchBySecond& Case(First first, Second second) noexcept
    {
        if (!m_Result && m_Key == second)
        {
            m_Result.emplace(first);
        }
        return *this;
    }

    constexpr std::optional<First> GetResult() noexcept
    {
        return m_Result;
    }

private:
    Second m_Key;
    std::optional<First> m_Result;
};

template <class Second>
class SwitchByFirstIgnoreCase
{
public:
    constexpr explicit SwitchByFirstIgnoreCase(std::string_view key) noexcept
        : m_Key{ key }
    {
    }

    constexpr SwitchByFirstIgnoreCase& Case(std::string_view first,
                                            Second second) noexcept
    {
        // clang-format off
        assert(!internal::ContainsUppercaseAscii(first) &&
"All strings in Case should be lowercase for case insensitive search");
        // clang-format on

        if (!m_Result && m_Key.size() == first.size() &&
            internal::EqualIgnoreCaseLowercase(first, m_Key))
        {
            m_Result.emplace(second);
        }
        return *this;
    }

    constexpr std::optional<Second> GetResult() noexcept
    {
        return m_Result;
    }

private:
    std::string_view m_Key;
    std::optional<Second> m_Result;
};

template <class First>
class SwitchBySecondIgnoreCase
{
public:
    constexpr explicit SwitchBySecondIgnoreCase(std::string_view key) noexcept
        : m_Key{ key }
    {
    }

    constexpr SwitchBySecondIgnoreCase& Case(First first,
                                             std::string_view second) noexcept
    {
        // clang-format off
        assert(!internal::ContainsUppercaseAscii(second) &&
"All strings in Case should be lowercase for case insensitive search");
        // clang-format on

        if (!m_Result && m_Key.size() == second.size() &&
            internal::EqualIgnoreCaseLowercase(second, m_Key))
        {
            m_Result.emplace(second);
        }
        return *this;
    }

    constexpr std::optional<First> GetResult() noexcept
    {
        return m_Result;
    }

private:
    std::string_view m_Key;
    std::optional<First> m_Result;
};

} // namespace internal

template <class BuilderFunc>
class BiMap
{
    using TypesPair =
        std::invoke_result_t<const BuilderFunc&, internal::SwitchTypesDetector>;

public:
    constexpr BiMap(BuilderFunc&& func) noexcept
        : m_Func{ std::move(func) }
    {
    }

    using First = TypesPair::first_type;
    using Second = TypesPair::second_type;

    constexpr std::optional<Second> FindByFirst(First value) const noexcept
    {
        return m_Func(internal::SwitchByFirst<First, Second>{ value })
            .GetResult();
    }

    constexpr std::optional<First> FindBySecond(Second value) const noexcept
    {
        return m_Func(internal::SwitchBySecond<First, Second>{ value })
            .GetResult();
    }

    template <class T>
    using MappedTypeForT =
        std::conditional_t<std::is_convertible_v<T, First>, Second, First>;

    template <class T>
    constexpr std::optional<MappedTypeForT<T>> Find(T value) const noexcept
    {
        // xor
        static_assert(std::is_convertible_v<T, First> !=
                          std::is_convertible_v<T, Second>,
                      "T is either not converible type or ambiguous");

        if constexpr (std::is_convertible_v<T, First>)
        {
            return FindByFirst(value);
        }
        else
        {
            return FindBySecond(value);
        }
    }

    constexpr std::optional<Second>
    FindByFirstIgnoreCase(std::string_view value) const noexcept
    {
        return m_Func(internal::SwitchByFirstIgnoreCase<Second>{ value })
            .GetResult();
    }

    constexpr std::optional<First>
    FindBySecondIgnoreCase(std::string_view value) const noexcept
    {
        return m_Func(internal::SwitchBySecondIgnoreCase<First>{ value })
            .GetResult();
    }

    constexpr std::optional<MappedTypeForT<std::string_view>>
    FindIgnoreCase(std::string_view value) const noexcept
    {
        // xor
        static_assert(std::is_convertible_v<std::string_view, First> !=
                          std::is_convertible_v<std::string_view, Second>,
                      "std::string_view is either not converible to map types "
                      "or ambiguous");

        if constexpr (std::is_convertible_v<std::string_view, First>)
        {
            return FindByFirstIgnoreCase(value);
        }
        else
        {
            return FindBySecondIgnoreCase(value);
        }
    }

private:
    BuilderFunc m_Func;
};

} // namespace cul
