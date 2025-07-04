﻿#ifndef __DMTYPETRAITS_CORE_H_INCLUDE__
#define __DMTYPETRAITS_CORE_H_INCLUDE__

#include "dmtypetraits_base.h" // 引入我们定义的基础类型萃取
#include <string_view> // 用于 dm_is_string_like_v

//-----------------------------------------------------------------------------
// SFINAE 检测工具 (Detection Idiom using std::void_t)
//-----------------------------------------------------------------------------
// 这些是构建复合萃取的底层工具

namespace dm_detail {
    // 检测 T 是否有 .begin()
    template<typename T, typename = void>
    struct has_begin : std::false_type {};
    template<typename T>
    struct has_begin<T, std::void_t<decltype(std::declval<T>().begin())>> : std::true_type {};

    // 检测 T 是否有 .end()
    template<typename T, typename = void>
    struct has_end : std::false_type {};
    template<typename T>
    struct has_end<T, std::void_t<decltype(std::declval<T>().end())>> : std::true_type {};

    // 检测 T 是否有 .size()
    template<typename T, typename = void>
    struct has_size : std::false_type {};
    template<typename T>
    struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};
    
    // 检测 T 是否可解引用 (operator*)
    template<typename T, typename = void>
    struct is_dereferenceable : std::false_type {};
    template<typename T>
    struct is_dereferenceable<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type {};

    // 检测 T 是否有箭头操作 (operator->)
    template<typename T, typename = void>
    struct has_arrow_operator : std::false_type {};
    template<typename T>
    struct has_arrow_operator<T, std::void_t<decltype(std::declval<T>().operator->())>> : std::true_type {};

} // namespace dm_detail

// 辅助变量模板
template<typename T>
inline constexpr bool dm_has_begin_v = dm_detail::has_begin<T>::value;
template<typename T>
inline constexpr bool dm_has_end_v = dm_detail::has_end<T>::value;
template<typename T>
inline constexpr bool dm_has_size_v = dm_detail::has_size<T>::value;
template<typename T>
inline constexpr bool dm_is_dereferenceable_v = dm_detail::is_dereferenceable<T>::value;
template<typename T>
inline constexpr bool dm_has_arrow_operator_v = dm_detail::has_arrow_operator<T>::value;


//-----------------------------------------------------------------------------
// 复合类型萃取 (Composite Type Traits)
//-----------------------------------------------------------------------------

/**
 * @brief 判断类型 T 是否为容器。
 *
 * 简单地认为拥有 .begin(), .end(), 和 .size() 成员函数的即为容器。
 * 注意：这不包含 C-style 数组。
 */
template<typename T>
inline constexpr bool dm_is_container_v = dm_has_begin_v<T> && dm_has_end_v<T> && dm_has_size_v<T>;

#if __cplusplus >= 202002L
/**
 * @brief (Concept) 判断类型 T 是否为容器。
 */
template<typename T>
concept dm_container = dm_is_container_v<T>;
#endif

/**
 * @brief 判断类型 T 是否为强类型枚举 (enum class)。
 *
 * 通过判断 T 是一个枚举，但不能隐式转换为其底层类型来实现。
 */
// 建议的替代实现
namespace dm_detail {
    template<typename T, bool = dm_is_enum_v<T>>
    struct is_scoped_enum_impl : std::false_type {};

    template<typename T>
    struct is_scoped_enum_impl<T, true> : std::bool_constant<
        !std::is_convertible_v<T, std::underlying_type_t<T>>
    > {};
} // namespace dm_detail

template<typename T>
inline constexpr bool dm_is_scoped_enum_v = dm_detail::is_scoped_enum_impl<T>::value;

/**
 * @brief 判断类型 T 是否能像字符串一样使用。
 *
 * 通过判断 T 能否隐式转换为 std::string_view 来实现。
 * 这可以匹配 std::string, const char*, std::string_view 等。
 */
template<typename T>
inline constexpr bool dm_is_string_like_v = dm_is_convertible_v<const T&, std::string_view>;


/**
 * @brief 判断类型 T 是否像指针（可解引用，有箭头操作）。
 *
 * 适用于裸指针和大部分智能指针。
 */
template<typename T>
inline constexpr bool dm_is_pointer_like_v = dm_is_dereferenceable_v<T> && (dm_is_pointer_v<T> || dm_has_arrow_operator_v<T>);

#endif // __DMTYPETRAITS_CORE_H_INCLUDE__