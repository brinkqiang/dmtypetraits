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

#ifndef __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__
#define __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__

#include "dmtypetraits_core.h"
#include <tuple>
#include <array>
#include <string_view>
#include <functional>
#include <optional>
#include <variant>

//-----------------------------------------------------------------------------
// 更多 SFINAE 检测工具
//-----------------------------------------------------------------------------
namespace dm_detail {
    // 检测 T 是否有 .empty()
    template<typename T, typename = void>
    struct has_empty : std::false_type {};
    template<typename T>
    struct has_empty<T, std::void_t<decltype(std::declval<T>().empty())>> : std::true_type {};

    // 检测 T 是否有 .clear()
    template<typename T, typename = void>
    struct has_clear : std::false_type {};
    template<typename T>
    struct has_clear<T, std::void_t<decltype(std::declval<T>().clear())>> : std::true_type {};

    // 检测 T 是否有 .push_back()
    template<typename T, typename = void>
    struct has_push_back : std::false_type {};
    template<typename T>
    struct has_push_back<T, std::void_t<decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>> : std::true_type {};

    // 检测 T 是否有 .emplace_back()
    template<typename T, typename = void>
    struct has_emplace_back : std::false_type {};
    template<typename T>
    struct has_emplace_back<T, std::void_t<decltype(std::declval<T>().emplace_back())>> : std::true_type {};

    // 检测 T 是否有 value_type 成员类型
    template<typename T, typename = void>
    struct has_value_type : std::false_type {};
    template<typename T>
    struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type {};

    // 检测 T 是否有 key_type 成员类型 (关联容器)
    template<typename T, typename = void>
    struct has_key_type : std::false_type {};
    template<typename T>
    struct has_key_type<T, std::void_t<typename T::key_type>> : std::true_type {};

    // 检测 T 是否有 mapped_type 成员类型 (map类容器)
    template<typename T, typename = void>
    struct has_mapped_type : std::false_type {};
    template<typename T>
    struct has_mapped_type<T, std::void_t<typename T::mapped_type>> : std::true_type {}; // <--- BUG FIX HERE

    // 检测 T 是否有 iterator 成员类型
    template<typename T, typename = void>
    struct has_iterator : std::false_type {};
    template<typename T>
    struct has_iterator<T, std::void_t<typename T::iterator>> : std::true_type {};

    // 检测 T 是否有 const_iterator 成员类型
    template<typename T, typename = void>
    struct has_const_iterator : std::false_type {};
    template<typename T>
    struct has_const_iterator<T, std::void_t<typename T::const_iterator>> : std::true_type {};

    // 检测 T 是否有 find() 方法
    template<typename T, typename = void>
    struct has_find : std::false_type {};
    template<typename T>
    struct has_find<T, std::void_t<decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>> : std::true_type {};

    // 检测是否为 std::tuple
    template<typename T>
    struct is_tuple : std::false_type {};
    template<typename... Args>
    struct is_tuple<std::tuple<Args...>> : std::true_type {};

    // 检测是否为 std::pair
    template<typename T>
    struct is_pair : std::false_type {};
    template<typename T1, typename T2>
    struct is_pair<std::pair<T1, T2>> : std::true_type {};

    // 检测是否为 std::array
    template<typename T>
    struct is_std_array : std::false_type {};
    template<typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};

    // 检测是否为C风格数组
    template<typename T>
    struct is_c_array : std::false_type {};
    template<typename T>
    struct is_c_array<T[]> : std::true_type {};
    template<typename T, std::size_t N>
    struct is_c_array<T[N]> : std::true_type {};
    
    // 检测是否为 std::optional
    template<typename T> struct is_optional : std::false_type {};
    template<typename T> struct is_optional<std::optional<T>> : std::true_type {};

    // 检测是否为 std::variant
    template<typename T> struct is_variant : std::false_type {};
    template<typename... Ts> struct is_variant<std::variant<Ts...>> : std::true_type {};

} // namespace dm_detail

// 辅助变量模板
template<typename T> inline constexpr bool dm_has_empty_v = dm_detail::has_empty<T>::value;
template<typename T> inline constexpr bool dm_has_clear_v = dm_detail::has_clear<T>::value;
template<typename T> inline constexpr bool dm_has_push_back_v = dm_detail::has_push_back<T>::value;
template<typename T> inline constexpr bool dm_has_emplace_back_v = dm_detail::has_emplace_back<T>::value;
template<typename T> inline constexpr bool dm_has_value_type_v = dm_detail::has_value_type<T>::value;
template<typename T> inline constexpr bool dm_has_key_type_v = dm_detail::has_key_type<T>::value;
template<typename T> inline constexpr bool dm_has_mapped_type_v = dm_detail::has_mapped_type<T>::value;
template<typename T> inline constexpr bool dm_has_iterator_v = dm_detail::has_iterator<T>::value;
template<typename T> inline constexpr bool dm_has_const_iterator_v = dm_detail::has_const_iterator<T>::value;
template<typename T> inline constexpr bool dm_has_find_v = dm_detail::has_find<T>::value;
template<typename T> inline constexpr bool dm_is_tuple_v = dm_detail::is_tuple<T>::value;
template<typename T> inline constexpr bool dm_is_pair_v = dm_detail::is_pair<T>::value;
template<typename T> inline constexpr bool dm_is_std_array_v = dm_detail::is_std_array<T>::value;
template<typename T> inline constexpr bool dm_is_c_array_v = dm_detail::is_c_array<T>::value;
template<typename T> inline constexpr bool dm_is_optional_v = dm_detail::is_optional<dm_remove_cvref_t<T>>::value;
template<typename T> inline constexpr bool dm_is_variant_v = dm_detail::is_variant<dm_remove_cvref_t<T>>::value;
template<typename T> inline constexpr bool dm_is_monostate_v = std::is_same_v<dm_remove_cvref_t<T>, std::monostate>;


//-----------------------------------------------------------------------------
// 更复杂的复合类型萃取
//-----------------------------------------------------------------------------

/**
 * @brief 判断类型 T 是否为序列容器 (支持 push_back)
 */
template<typename T>
inline constexpr bool dm_is_sequence_container_v = dm_is_container_v<T> && dm_has_push_back_v<T>;

/**
 * @brief 判断类型 T 是否为关联容器 (有 key_type 和 find)
 */
template<typename T>
inline constexpr bool dm_is_associative_container_v = dm_is_container_v<T> && dm_has_key_type_v<T> && dm_has_find_v<T>;

/**
 * @brief 判断类型 T 是否为映射容器 (map 类型，有 mapped_type)
 */
template<typename T>
inline constexpr bool dm_is_map_container_v = dm_is_associative_container_v<T> && dm_has_mapped_type_v<T>;

/**
 * @brief 判断类型 T 是否为集合容器 (set 类型，无 mapped_type)
 */
template<typename T>
inline constexpr bool dm_is_set_container_v = dm_is_associative_container_v<T> && !dm_has_mapped_type_v<T>;

/**
 * @brief (别名) 判断类型 T 是否为映射容器
 */
template<typename T>
inline constexpr bool dm_is_map_v = dm_is_map_container_v<T>;

/**
 * @brief (别名) 判断类型 T 是否为集合容器
 */
template<typename T>
inline constexpr bool dm_is_set_v = dm_is_set_container_v<T>;

/**
 * @brief 判断类型 T 是否为任意形式的数组 (C数组 或 std::array)
 */
template<typename T>
inline constexpr bool dm_is_any_array_v = dm_is_array_v<T> || dm_is_std_array_v<T>;

/**
 * @brief 判断类型 T 是否为可迭代的 (有 begin/end)
 */
template<typename T>
inline constexpr bool dm_is_iterable_v = dm_has_begin_v<T> && dm_has_end_v<T>;

/**
 * @brief (C++17) 判断类型 F 是否能以参数 Args... 进行调用。
 */
template<typename F, typename... Args>
inline constexpr bool dm_is_invocable_v = std::is_invocable_v<F, Args...>;

/**
 * @brief 判断类型 T 是否为智能指针 (排除裸指针)
 */
template<typename T>
inline constexpr bool dm_is_smart_pointer_v = dm_is_pointer_like_v<T> && !dm_is_pointer_v<T>;

/**
 * @brief 判断类型 T 是否为元组类型 (tuple 或 pair)
 */
template<typename T>
inline constexpr bool dm_is_tuple_like_v = dm_is_tuple_v<T> || dm_is_pair_v<T>;

//-----------------------------------------------------------------------------
// 数值类型分类
//-----------------------------------------------------------------------------

/**
 * @brief 判断类型 T 是否为有符号整数 (不包括 bool 和 char)
 */
template<typename T>
inline constexpr bool dm_is_signed_integer_v = dm_is_integral_v<T> && dm_is_signed_v<T> &&
                                               !dm_is_same_v<T, bool> && !dm_is_same_v<T, char>;

/**
 * @brief 判断类型 T 是否为无符号整数 (不包括 bool)
 */
template<typename T>
inline constexpr bool dm_is_unsigned_integer_v = dm_is_integral_v<T> && dm_is_unsigned_v<T> &&
                                                 !dm_is_same_v<T, bool>;

/**
 * @brief 判断类型 T 是否为数值类型 (整数 或 浮点数)
 */
template<typename T>
inline constexpr bool dm_is_numeric_v = dm_is_integral_v<T> || dm_is_floating_point_v<T>;

//-----------------------------------------------------------------------------
// 实用工具萃取
//-----------------------------------------------------------------------------
namespace dm_detail {
    // 辅助模板，用于安全地检测和提取 ::value_type
    template <typename T, typename = void>
    struct element_type_helper { using type = void; };

    template <typename T>
    struct element_type_helper<T, std::void_t<typename T::value_type>> {
        using type = typename T::value_type;
    };
}

template<typename T> struct dm_element_type {
    using type = typename dm_detail::element_type_helper<T>::type;
};
template<typename T> struct dm_element_type<T*> { using type = T; };
template<typename T> struct dm_element_type<T[]> { using type = T; };
template<typename T, std::size_t N> struct dm_element_type<T[N]> { using type = T; };
template<typename T, std::size_t N> struct dm_element_type<std::array<T, N>> { using type = T; };
template<typename T> using dm_element_type_t = typename dm_element_type<T>::type;


template<typename T>
struct dm_get_array_size : std::integral_constant<size_t, 0> {};
template<typename T, std::size_t N>
struct dm_get_array_size<T[N]> : std::integral_constant<size_t, N> {};
template<typename T, std::size_t N>
struct dm_get_array_size<std::array<T, N>> : std::integral_constant<size_t, N> {};
template<typename T>
inline constexpr size_t dm_get_array_size_v = dm_get_array_size<dm_remove_cvref_t<T>>::value;


template<typename T> struct dm_remove_all_pointers { using type = T; };
template<typename T> struct dm_remove_all_pointers<T*> {
    using type = typename dm_remove_all_pointers<dm_remove_cvref_t<T>>::type;
};
template<typename T>
using dm_remove_all_pointers_t = typename dm_remove_all_pointers<T>::type;


template<typename T>
using dm_pure_type_t = dm_remove_cv_t<dm_remove_all_pointers_t<dm_decay_t<T>>>;


namespace dm_detail {
    template<typename From, typename To>
    struct copy_cvref_impl {
    private:
        using base_to_t = dm_remove_cvref_t<To>;
        using const_applied_t = dm_conditional_t<dm_is_const_v<dm_remove_reference_t<From>>, dm_add_const_t<base_to_t>, base_to_t>;
        using cv_applied_t = dm_conditional_t<dm_is_volatile_v<dm_remove_reference_t<From>>, dm_add_volatile_t<const_applied_t>, const_applied_t>;
        using ref_applied_t = dm_conditional_t<dm_is_lvalue_reference_v<From>, dm_add_lvalue_reference_t<cv_applied_t>,
            dm_conditional_t<dm_is_rvalue_reference_v<From>, dm_add_rvalue_reference_t<cv_applied_t>, cv_applied_t>>;
    public:
        using type = ref_applied_t;
    };

    template<typename Variant, typename Visitor>
    struct variant_helper;

    template<template<typename...> class V, typename... Ts, typename Visitor>
    struct variant_helper<V<Ts...>, Visitor> {
        static constexpr decltype(auto) visit(Visitor&& visitor) {
            return visitor.template operator()<Ts...>();
        }
    };
} 

/**
 * @brief 将类型 From 的 cv-qualifiers 和引用拷贝到类型 To 上。
 */
template<typename From, typename To>
using dm_copy_cvref_t = typename dm_detail::copy_cvref_impl<From, To>::type;

/**
 * @brief 访问一个 std::variant 的所有备选类型。
 */
template <typename Variant, typename Visitor>
constexpr decltype(auto) dm_visit_variant(Variant&&, Visitor&& visitor) {
    return dm_detail::variant_helper<dm_remove_cvref_t<Variant>, Visitor>::visit(std::forward<Visitor>(visitor));
}

namespace dm_detail {
    // 后备方案：从编译器签名中提取原始类型名称
    template<typename T>
    constexpr std::string_view get_raw_type_name() {
#if defined(__clang__)
        constexpr std::string_view prefix = "[T = ";
        constexpr std::string_view suffix = "]";
        constexpr std::string_view function = __PRETTY_FUNCTION__;
        const auto start = function.find(prefix) + prefix.size();
        const auto end = function.rfind(suffix);
        if (start < end) {
            return function.substr(start, (end - start));
        }
        return function;
#elif defined(__GNUC__)
        constexpr std::string_view prefix = "[with T = ";
        constexpr std::string_view suffix = "]";
        constexpr std::string_view function = __PRETTY_FUNCTION__;
        const auto start = function.find(prefix) + prefix.size();
        const auto end = function.rfind(suffix);
        if (start < end) {
            return function.substr(start, (end - start));
        }
        return function;
#elif defined(_MSC_VER)
        constexpr std::string_view prefix = "get_raw_type_name<"; // 注意，我们现在调用的是这个内部函数
        constexpr std::string_view suffix = ">(void)";
        constexpr std::string_view function = __FUNCSIG__;
        const auto start = function.find(prefix) + prefix.size();
        const auto end = function.rfind(suffix);
        if (start < end) {
            return function.substr(start, (end - start));
        }
        return function;
#else
        return "Unknown Type";
#endif
    }
} // namespace dm_detail

template<typename T>
constexpr std::string_view dm_type_name() {
    // 使用 dm_pure_type_t 来获取最根本的类型，去除所有修饰符
    using CleanT = dm_pure_type_t<T>;

    // 为常用类型提供特化名称
    if constexpr (std::is_same_v<CleanT, std::string>) {
        return "std::string";
    }
    else if constexpr (std::is_same_v<CleanT, std::string_view>) {
        return "std::string_view";
    }
    else if constexpr (std::is_same_v<CleanT, int>) {
        return "int";
    }
    else if constexpr (std::is_same_v<CleanT, long>) {
        return "long";
    }
    else if constexpr (std::is_same_v<CleanT, long long>) {
        return "long long";
    }
    else if constexpr (std::is_same_v<CleanT, double>) {
        return "double";
    }
    else if constexpr (std::is_same_v<CleanT, float>) {
        return "float";
    }
    else if constexpr (std::is_same_v<CleanT, bool>) {
        return "bool";
    }
    else if constexpr (std::is_same_v<CleanT, char>) {
        return "char";
    }
    // 还可以继续添加其他你需要的类型，例如 std::vector, std::map 等
    // ...
    else {
        // 如果不是我们特化的类型，则调用原始的后备方案
        return dm_detail::get_raw_type_name<T>();
    }
}

#endif // __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__
