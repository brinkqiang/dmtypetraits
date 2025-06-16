
// Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __DMTYPETRAITS_FUNCTION_H_INCLUDE__
#define __DMTYPETRAITS_FUNCTION_H_INCLUDE__

#include "dmtypetraits_extensions.h" // 引入我们已定义的扩展类型萃取
#include <tuple>
#include <functional>

//-----------------------------------------------------------------------------
// 函数萃取 (Function Traits)
//-----------------------------------------------------------------------------

/**
 * @brief 主函数萃取模板，通过一系列特化来支持不同的可调用类型。
 * @tparam F 可调用类型 (函数指针, 成员函数指针, lambda等)
 */
template <typename F>
struct dm_function_traits;

// --- 带参数的函数特化 ---

template <typename R, typename... Args>
struct dm_function_traits<R(Args...)> {
    using return_type = R;
    using parameters_type = std::tuple<dm_remove_cvref_t<Args>...>; // 保留：纯粹类型
    using raw_parameters_type = std::tuple<Args...>;                  // 新增：原始类型
};

template <typename R, typename... Args>
struct dm_function_traits<R(Args...) noexcept> : dm_function_traits<R(Args...)> {};

template <typename R, typename... Args>
struct dm_function_traits<R (*)(Args...)> : dm_function_traits<R(Args...)> {};

template <typename R, typename... Args>
struct dm_function_traits<R (*)(Args...) noexcept> : dm_function_traits<R(Args...)> {};

template <typename T, typename R, typename... Args>
struct dm_function_traits<R (T::*)(Args...)> {
    using return_type = R;
    using class_type = T;
    using parameters_type = std::tuple<dm_remove_cvref_t<Args>...>; // 保留：纯粹类型
    using raw_parameters_type = std::tuple<Args...>;                  // 新增：原始类型
};

template <typename T, typename R, typename... Args>
struct dm_function_traits<R (T::*)(Args...) const> : dm_function_traits<R (T::*)(Args...)> {};

template <typename T, typename R, typename... Args>
struct dm_function_traits<R (T::*)(Args...) noexcept> : dm_function_traits<R (T::*)(Args...)> {};

template <typename T, typename R, typename... Args>
struct dm_function_traits<R (T::*)(Args...) const noexcept> : dm_function_traits<R (T::*)(Args...)> {};

// --- 无参数的函数特化 ---

template <typename R>
struct dm_function_traits<R()> {
    using return_type = R;
    using parameters_type = void;     // 保留
    using raw_parameters_type = void; // 新增
};

template <typename R>
struct dm_function_traits<R() noexcept> : dm_function_traits<R()> {};

template <typename R>
struct dm_function_traits<R (*)()> : dm_function_traits<R()> {};

template <typename R>
struct dm_function_traits<R (*)() noexcept> : dm_function_traits<R()> {};

template <typename R>
struct dm_function_traits<R (&)()> : dm_function_traits<R()> {};

template <typename R>
struct dm_function_traits<R (&)() noexcept> : dm_function_traits<R()> {};

template <typename T, typename R>
struct dm_function_traits<R (T::*)()> {
    using return_type = R;
    using class_type = T;
    using parameters_type = void;     // 保留
    using raw_parameters_type = void; // 新增
};

template <typename T, typename R>
struct dm_function_traits<R (T::*)() const> : dm_function_traits<R (T::*)()> {};

template <typename T, typename R>
struct dm_function_traits<R (T::*)() noexcept> : dm_function_traits<R (T::*)()> {};

template <typename T, typename R>
struct dm_function_traits<R (T::*)() const noexcept> : dm_function_traits<R (T::*)()> {};


// --- 函数对象和 Lambda 表达式的特化 ---
template <class F>
struct dm_function_traits : dm_function_traits<decltype(&F::operator())> {};


//-----------------------------------------------------------------------------
// 函数萃取相关的便捷别名
//-----------------------------------------------------------------------------

/**
 * @brief 萃取可调用对象的返回类型。
 */
template <typename F>
using dm_function_return_t = typename dm_function_traits<dm_remove_cvref_t<F>>::return_type;

/**
 * @brief 萃取可调用对象的“纯粹”参数类型元组（已移除CV-Ref）。对于无参函数，类型为 void。
 */
template <typename F>
using dm_function_parameters_t = typename dm_function_traits<dm_remove_cvref_t<F>>::parameters_type;

/**
 * @brief (新增) 萃取可调用对象的“原始”参数类型元组（保留CV-Ref）。对于无参函数，类型为 void。
 */
template <typename F>
using dm_function_raw_parameters_t = typename dm_function_traits<dm_remove_cvref_t<F>>::raw_parameters_type;

/**
 * @brief 萃取成员函数所属的类类型。
 */
template <typename F>
using dm_function_class_t = typename dm_function_traits<dm_remove_cvref_t<F>>::class_type;

/**
 * @brief 萃取可调用对象第 I 个“纯粹”参数的类型。
 */
template <size_t I, typename F>
using dm_function_arg_t = std::tuple_element_t<I, dm_function_parameters_t<F>>;

/**
 * @brief (新增) 萃取可调用对象第 I 个“原始”参数的类型。
 */
template <size_t I, typename F>
using dm_function_raw_arg_t = std::tuple_element_t<I, dm_function_raw_parameters_t<F>>;

/**
 * @brief 萃取可调用对象最后一个“纯粹”参数的类型。
 * @note 如果函数没有参数，此别名将导致编译错误。
 */
template <typename F>
using dm_last_parameter_t =
    std::tuple_element_t<std::tuple_size_v<dm_function_parameters_t<F>> - 1,
                         dm_function_parameters_t<F>>;
//-----------------------------------------------------------------------------
// 函数萃取相关的便捷别名
//-----------------------------------------------------------------------------

/**
 * @brief 萃取可调用对象的返回类型。
 */
template <typename F>
using dm_function_return_t = typename dm_function_traits<dm_remove_cvref_t<F>>::return_type;

/**
 * @brief 萃取可调用对象的参数类型元组。对于无参函数，类型为 void。
 */
template <typename F>
using dm_function_parameters_t = typename dm_function_traits<dm_remove_cvref_t<F>>::parameters_type;

/**
 * @brief 萃取成员函数所属的类类型。
 */
template <typename F>
using dm_function_class_t = typename dm_function_traits<dm_remove_cvref_t<F>>::class_type;

/**
 * @brief 萃取可调用对象最后一个参数的类型。
 * @note 如果函数没有参数，此别名将导致编译错误。
 */
template <typename F>
using dm_last_parameter_t =
    std::tuple_element_t<std::tuple_size_v<dm_function_parameters_t<F>> - 1,
                         dm_function_parameters_t<F>>;

//-----------------------------------------------------------------------------
// 高级类型谓词 (Advanced Type Predicates)
//-----------------------------------------------------------------------------

/**
 * @brief 检查类型 T 是否为模板 Ref 的一个特化版本。
 * @example dm_is_specialization_v<std::vector<int>, std::vector> // true
 * @example dm_is_specialization_v<int, std::vector>            // false
 */
namespace dm_detail {
    template <typename T, template <typename...> class Ref>
    struct is_specialization_impl : std::false_type {};

    template <template <typename...> class Ref, typename... Args>
    struct is_specialization_impl<Ref<Args...>, Ref> : std::true_type {};
} // namespace dm_detail

template <typename T, template <typename...> class Ref>
inline constexpr bool dm_is_specialization_v = dm_detail::is_specialization_impl<dm_remove_cvref_t<T>, Ref>::value;


//-----------------------------------------------------------------------------
// 元组工具 (Tuple Utilities)
//-----------------------------------------------------------------------------

namespace dm_detail {
    template <typename T>
    struct remove_first_impl {
        using type = T;
    };

    template <class First, class... Rest>
    struct remove_first_impl<std::tuple<First, Rest...>> {
        using type = std::tuple<Rest...>;
    };
} // namespace dm_detail

/**
 * @brief 从一个 std::tuple 中移除第一个元素类型。
 */
template <typename T>
using dm_remove_first_t = typename dm_detail::remove_first_impl<T>::type;

/**
 * @brief 根据条件返回完整或移除首元素的参数元组实例。
 */
template <bool ShouldRemoveFirst, typename TTuple>
inline auto dm_get_args() {
    if constexpr (ShouldRemoveFirst) {
        return dm_remove_first_t<TTuple>{};
    }
    else {
        return TTuple{};
    }
}

// 将此代码添加到“函数萃取相关的便捷别名”部分
/**
 * @brief 萃取可调用对象第 I 个参数的类型。
 */
template <size_t I, typename F>
using dm_function_arg_t = std::tuple_element_t<I, dm_function_parameters_t<F>>;

#endif // __DMTYPETRAITS_FUNCTION_H_INCLUDE__