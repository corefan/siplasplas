#ifndef SIPLASPLAS_UTILITY_INVOKE_HPP
#define SIPLASPLAS_UTILITY_INVOKE_HPP

#include "function_traits.hpp"
#include "logger.hpp"
#include <utility>
#include <functional>
#include <iostream>

namespace cpp
{

namespace detail
{
    template<typename Class, typename Object,
        bool IsBase = std::is_base_of<Class, Object>::value
    >
    class InvokeMemberFunction
    {
    public:
        template<typename Function, typename Object_, typename... Args>
        static auto apply(Function function, Object_&& object, Args&&... args)
        {
            constexpr ctti::type_id_t typeIds[] = { ctti::type_id<std::decay_t<Args>>()..., ctti::type_id<void>() };

            utility::log().debug("About to invoke member function:");
            utility::log().debug("Object: {} @{}", ctti::type_id<std::decay_t<Object_>>().name(), reinterpret_cast<void*>(&object));

            for(std::size_t i = 0; i < sizeof...(args); ++i)
            {
                utility::log().debug("  #{} {}", i, typeIds[i].name());
            }

            std::cout << reinterpret_cast<void*>(&object) << std::endl;

            return (std::forward<Object_>(object).*function)(std::forward<Args>(args)...);
        }
    };

    template<typename Class, typename T>
    class InvokeMemberFunction<Class, std::reference_wrapper<T>, false>
    {
    public:
        template<typename Function, typename Reference_, typename... Args>
        static auto apply(Function function, Reference_&& reference, Args&&... args)
        {
            return ((std::forward<Reference_>(reference).get()).*function)(std::forward<Args>(args)...);
        }
    };

    template<typename Class, typename Object>
    class InvokeMemberFunction<Class, Object, false>
    {
    public:
        template<typename Function, typename Pointer_, typename... Args>
        static auto apply(Function function, Pointer_&& pointer, Args&&... args)
        {
            return ((*std::forward<Pointer_>(pointer)).*function)(std::forward<Args>(args)...);
        }
    };

    template<typename R, typename Class, typename... Params, typename Head, typename... Tail>
    auto invokeMemberFunction(R (Class::*function)(Params...), Head&& head, Tail&&... tail)
    {
        return InvokeMemberFunction<Class, std::decay_t<Head>>::apply(function, std::forward<Head>(head), std::forward<Tail>(tail)...);
    }

    template<typename R, typename Class, typename... Params, typename Head, typename... Tail>
    auto invokeMemberFunction(R (Class::*function)(Params...) const, Head&& head, Tail&&... tail)
    {
        return InvokeMemberFunction<Class, std::decay_t<Head>>::apply(function, std::forward<Head>(head), std::forward<Tail>(tail)...);
    }
}

/**
 * \ingroup utility
 * \brief Invokes the [`Callable`](http://en.cppreference.com/w/cpp/concept/Callable) object with the given arguments.
 * See [`std::invoke()`](http://en.cppreference.com/w/cpp/utility/functional/invoke)
 */
template<typename Callable, typename... Args>
auto invoke(Callable&& callable, Args&&... args)
{
    return std::forward<Callable>(callable)(std::forward<Args>(args)...);
}

template<typename R, typename Class, typename... Params, typename... Args>
auto invoke(R (Class::*function)(Params...), Args&&... args)
{
    return detail::invokeMemberFunction(function, std::forward<Args>(args)...);
}

template<typename R, typename Class, typename... Params, typename... Args>
auto invoke(R (Class::*function)(Params...) const, Args&&... args)
{
    return detail::invokeMemberFunction(function, std::forward<Args>(args)...);
}

}

#endif // SIPLASPLAS_UTILITY_INVOKE_HPP