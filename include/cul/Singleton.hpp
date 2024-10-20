#pragma once

#include <cassert>
#include <csignal>
#include <type_traits>
namespace cul
{

template <class T>
struct CreateUsingNew final
{
    static_assert(!std::is_array_v<T>, "Use custom deleter for array types");

    static T* CreateInstance()
    {
        return new T();
    }

    static void DestroyInstance(T* ptr)
    {
        delete ptr;
    }
};

template <class T, template <class> class CreationPolicy = CreateUsingNew>
class SingletonMixin
{
public:
    static T& GetInstance()
    {
        if (!SingletonMixin::s_Instance)
        {
            SingletonMixin::s_Instance = CreationPolicy<T>::CreateInstance();
        }
        return *SingletonMixin::s_Instance;
    }

    explicit SingletonMixin(const SingletonMixin&) = delete;
    SingletonMixin& operator=(const SingletonMixin&) = delete;

protected:
    explicit SingletonMixin()
    {
        // TODO: assertions
        if (SingletonMixin::s_Instance)
        {
            raise(SIGTRAP);
        }
        SingletonMixin::s_Instance = static_cast<T*>(this);
    }

    virtual ~SingletonMixin()
    {
        SingletonMixin::s_Instance = nullptr;
    }

private:
    static T* s_Instance;
};

template <class T, template <class> class C>
T* SingletonMixin<T, C>::s_Instance = nullptr;
} // namespace cul
