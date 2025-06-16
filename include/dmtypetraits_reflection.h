#ifndef __DMTYPETRAITS_REFLECTION_H_INCLUDE__
#define __DMTYPETRAITS_REFLECTION_H_INCLUDE__

#include "dmbase_typetraits.h"
#include <utility>
#include <type_traits>

#if __cplusplus < 202002L
#error "dmtypetraits_reflection.h requires C++20 or later for consteval and requires expressions."
#endif

namespace dm_detail {

/**
 * @brief 一个可以隐式转换为任何类型的“万能转换器”。
 */
struct UniversalConverter {
    template <typename T>
    operator T() const;
};

/**
 * @brief 递归地、在编译期计算一个聚合类型 T 的成员数量。
 */
template <typename T, typename... Args>
consteval size_t count_members_impl() {
    if constexpr (!requires { T{ {Args{}}..., {UniversalConverter{}} }; }) {
        return sizeof...(Args);
    }
    else {
        return count_members_impl<T, Args..., UniversalConverter>();
    }
}

} // namespace dm_detail

/**
 * @brief (C++20) 获取一个聚合类型 T 的成员数量。
 */
template <typename T>
inline constexpr size_t dm_member_count_v = dm_detail::count_members_impl<dm_remove_cvref_t<T>>();

/**
 * @brief (C++20) 使用访问者函数遍历一个聚合对象的所有成员。
 */
template <typename T, typename Visitor>
constexpr decltype(auto) dm_visit_members(T&& object, Visitor&& visitor) {
    using CleanT = dm_remove_cvref_t<T>;
    constexpr size_t Count = dm_member_count_v<CleanT>;
    
    // 通过 if constexpr 和结构化绑定，根据成员数量调用不同的 visitor。
    if constexpr (Count == 1) {
        auto&& [m1] = object;
        return visitor(m1);
    } else if constexpr (Count == 2) {
        auto&& [m1, m2] = object;
        return visitor(m1, m2);
    } else if constexpr (Count == 3) {
        auto&& [m1, m2, m3] = object;
        return visitor(m1, m2, m3);
    } else if constexpr (Count == 4) {
        auto&& [m1, m2, m3, m4] = object;
        return visitor(m1, m2, m3, m4);
    } else if constexpr (Count == 5) {
        auto&& [m1, m2, m3, m4, m5] = object;
        return visitor(m1, m2, m3, m4, m5);
    } else if constexpr (Count == 6) {
        auto&& [m1, m2, m3, m4, m5, m6] = object;
        return visitor(m1, m2, m3, m4, m5, m6);
    } // ... 可以根据需要扩展到更多成员
    else {
        // 如果成员数量为0或超出支持范围，给出编译期错误。
        static_assert(Count <= 6, "dm_visit_members currently supports up to 6 members. You can extend it if needed.");
        return visitor();
    }
}

#endif // __DMTYPETRAITS_REFLECTION_H_INCLUDE__