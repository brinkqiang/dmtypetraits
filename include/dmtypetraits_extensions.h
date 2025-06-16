#ifndef __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__
#define __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__

#include "dmcore_typetraits.h"
#include <tuple>
#include <array>
#include <string_view>
#include <functional>

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
    struct has_mapped_type<T, std::void_t<typename T::mapped_type>> : std::true_type {};

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

    // 检测是否可调用 (函数、函数对象、lambda等)
    template<typename F, typename... Args>
    struct is_callable_impl {
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>()(std::declval<Args>()...), std::true_type{});
        template<typename>
        static std::false_type test(...);
        using type = decltype(test<F>(0));
    };

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

} // namespace dm_detail

// 辅助变量模板
template<typename T>
inline constexpr bool dm_has_empty_v = dm_detail::has_empty<T>::value;
template<typename T>
inline constexpr bool dm_has_clear_v = dm_detail::has_clear<T>::value;
template<typename T>
inline constexpr bool dm_has_push_back_v = dm_detail::has_push_back<T>::value;
template<typename T>
inline constexpr bool dm_has_emplace_back_v = dm_detail::has_emplace_back<T>::value;
template<typename T>
inline constexpr bool dm_has_value_type_v = dm_detail::has_value_type<T>::value;
template<typename T>
inline constexpr bool dm_has_key_type_v = dm_detail::has_key_type<T>::value;
template<typename T>
inline constexpr bool dm_has_mapped_type_v = dm_detail::has_mapped_type<T>::value;
template<typename T>
inline constexpr bool dm_has_iterator_v = dm_detail::has_iterator<T>::value;
template<typename T>
inline constexpr bool dm_has_const_iterator_v = dm_detail::has_const_iterator<T>::value;
template<typename T>
inline constexpr bool dm_has_find_v = dm_detail::has_find<T>::value;
template<typename F, typename... Args>
inline constexpr bool dm_is_callable_v = dm_detail::is_callable_impl<F, Args...>::type::value;
template<typename T>
inline constexpr bool dm_is_tuple_v = dm_detail::is_tuple<T>::value;
template<typename T>
inline constexpr bool dm_is_pair_v = dm_detail::is_pair<T>::value;
template<typename T>
inline constexpr bool dm_is_std_array_v = dm_detail::is_std_array<T>::value;
template<typename T>
inline constexpr bool dm_is_c_array_v = dm_detail::is_c_array<T>::value;

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
 * @brief 判断类型 T 是否为函数类型或可调用对象
 */
template<typename T>
inline constexpr bool dm_is_invocable_v = dm_is_function_v<T> || dm_is_callable_v<T>;

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

// 主模板通过辅助模板获取类型
template<typename T>
struct dm_element_type {
    using type = typename dm_detail::element_type_helper<T>::type;
};

// 各种特化版本（这些将优先于主模板被匹配）
template<typename T>
struct dm_element_type<T*> {
    using type = T;
};

template<typename T>
struct dm_element_type<T[]> { // 处理未定界数组
    using type = T;
};

template<typename T, std::size_t N>
struct dm_element_type<T[N]> {
    using type = T;
};

template<typename T, std::size_t N>
struct dm_element_type<std::array<T, N>> {
    using type = T;
};

template<typename T>
using dm_element_type_t = typename dm_element_type<T>::type;

/**
 * @brief 移除所有指针和引用修饰符，获取最终的值类型
 */
template<typename T>
struct dm_remove_all_pointers {
    using type = T;
};
template<typename T>
struct dm_remove_all_pointers<T*> : dm_remove_all_pointers<dm_remove_cv_t<T>> {}; // 递归移除所有指针层级

template<typename T>
using dm_remove_all_pointers_t = typename dm_remove_all_pointers<T>::type;
/**
 * @brief 完全去除类型修饰符 (cv-qualifiers + references + pointers)
 */
namespace dm_detail {
     template<typename T>
     struct pure_type_impl {
         using type = dm_remove_cv_t<dm_remove_all_pointers_t<dm_remove_cvref_t<T>>>;
     };
}
//
// 外部接口先进行 decay
template<typename T>
using dm_pure_type_t = typename dm_detail::pure_type_impl<std::decay_t<T>>::type;

namespace dm_detail {
    template<typename From, typename To>
    struct copy_cvref_impl {
    private:
        // 1. 移除 To 类型的所有 cv 和引用限定符，得到纯净的基础类型
        using base_to_t = dm_remove_cvref_t<To>;
        
        // 2. 为 base_to_t 添加来自 From 的 const 限定符 (如果 From 有的话)
        using const_applied_t = dm_conditional_t<
            dm_is_const_v<dm_remove_reference_t<From>>,
            dm_add_const_t<base_to_t>,
            base_to_t
        >;

        // 3. 在上一步的基础上，添加来自 From 的 volatile 限定符 (如果 From 有的话)
        using cv_applied_t = dm_conditional_t<
            dm_is_volatile_v<dm_remove_reference_t<From>>,
            dm_add_volatile_t<const_applied_t>,
            const_applied_t
        >;

        // 4. 在上一步的基础上，添加来自 From 的引用类型 (左值或右值)
        using ref_applied_t = dm_conditional_t<
            dm_is_lvalue_reference_v<From>,
            dm_add_lvalue_reference_t<cv_applied_t>,
            dm_conditional_t<
                dm_is_rvalue_reference_v<From>,
                dm_add_rvalue_reference_t<cv_applied_t>,
                cv_applied_t // 如果 From 不是引用，则最终类型也不是引用
            >
        >;
    public:
        using type = ref_applied_t;
    };
} // namespace dm_detail

/**
 * @brief 将类型 From 的 cv-qualifiers (const/volatile) 和引用 (&/&&)
 * 拷贝到类型 To 上。
 *
 * @example dm_copy_cvref_t<const int&, float>  // 结果为 const float&
 * @example dm_copy_cvref_t<int&&, char>         // 结果为 char&&
 * @example dm_copy_cvref_t<const volatile T, U> // 结果为 const volatile U
 */
template<typename From, typename To>
using dm_copy_cvref_t = typename dm_detail::copy_cvref_impl<From, To>::type;

/**
 * @brief (C++17) 在编译期获取类型的易读名称字符串。
 *
 * 这是一个依赖于编译器内置宏的调试工具。
 * @tparam T 要获取名称的类型。
 * @return 包含类型名称的 std::string_view。
 */
template<typename T>
constexpr std::string_view dm_type_name() {
#if defined(__clang__)
    // Clang: "std::string_view dm_type_name() [T = int]"
    constexpr std::string_view prefix = "[T = ";
    constexpr std::string_view suffix = "]";
    constexpr std::string_view function = __PRETTY_FUNCTION__;
    const auto start = function.find(prefix) + prefix.size();
    const auto end = function.rfind(suffix);
    return function.substr(start, (end - start));
#elif defined(__GNUC__)
    // GCC: "constexpr std::string_view dm_type_name() [with T = int]"
    constexpr std::string_view prefix = "[with T = ";
    constexpr std::string_view suffix = "]";
    constexpr std::string_view function = __PRETTY_FUNCTION__;
    const auto start = function.find(prefix) + prefix.size();
    const auto end = function.rfind(suffix);
    return function.substr(start, (end - start));
#elif defined(_MSC_VER)
    // MSVC: "auto __cdecl dm_type_name<int>(void)"
    constexpr std::string_view prefix = "dm_type_name<";
    constexpr std::string_view suffix = ">(void)";
    constexpr std::string_view function = __FUNCSIG__;
    const auto start = function.find(prefix) + prefix.size();
    const auto end = function.rfind(suffix);
    return function.substr(start, (end - start));
#else
    return "Unknown Type";
#endif
}
#endif // __DMTYPETRAITS_EXTENSIONS_H_INCLUDE__