#ifndef __DMINTERFACE_TRAITS_H_INCLUDE__
#define __DMINTERFACE_TRAITS_H_INCLUDE__

#include "dmtypetraits.h"
#include "dmmoduleptr.h"

//-----------------------------------------------------------------------------
// 接口相关的类型萃取
//-----------------------------------------------------------------------------
namespace dm_detail {
    // 检测 T 是否有 Release() 方法
    template<typename T, typename = void>
    struct has_release : std::false_type {};
    template<typename T>
    struct has_release<T, std::void_t<decltype(std::declval<T>().Release())>> : std::true_type {};

    // 检测 T 是否是接口类 (有虚析构函数且有Release方法)
    template<typename T, typename = void>
    struct is_interface_class : std::false_type {};
    template<typename T>
    struct is_interface_class<T, std::enable_if_t<
        dm_is_polymorphic_v<T> && 
        has_release<T>::value &&
        dm_is_abstract_v<T>
    >> : std::true_type {};

    // 检测 T 是否是 DmModulePtr 类型
    template<typename T>
    struct is_dm_module_ptr : std::false_type {};
    template<typename T>
    struct is_dm_module_ptr<DmModulePtr<T>> : std::true_type {};

    // 获取 DmModulePtr 的接口类型
    template<typename T>
    struct dm_module_ptr_interface {
        using type = void;
    };
    template<typename T>
    struct dm_module_ptr_interface<DmModulePtr<T>> {
        using type = T;
    };

    // 检测类型 T 是否可以构造为 DmModulePtr
    template<typename T, typename = void>
    struct is_dm_module_compatible : std::false_type {};
    template<typename T>
    struct is_dm_module_compatible<T, std::enable_if_t<
        dm_is_pointer_v<T> && 
        has_release<dm_remove_all_pointers_t<T>>::value
    >> : std::true_type {};

} // namespace dm_detail

// 辅助变量模板
template<typename T>
inline constexpr bool dm_has_release_v = dm_detail::has_release<T>::value;

template<typename T>
inline constexpr bool dm_is_interface_class_v = dm_detail::is_interface_class<T>::value;

template<typename T>
inline constexpr bool dm_is_dm_module_ptr_v = dm_detail::is_dm_module_ptr<T>::value;

template<typename T>
inline constexpr bool dm_is_dm_module_compatible_v = dm_detail::is_dm_module_compatible<T>::value;

// 类型别名
template<typename T>
using dm_module_ptr_interface_t = typename dm_detail::dm_module_ptr_interface<T>::type;

//-----------------------------------------------------------------------------
// 接口工厂函数萃取
//-----------------------------------------------------------------------------
namespace dm_detail {
    // 检测是否存在对应的工厂函数
    // 例如：dmprojectinfoGetModule() 对应接口 Idmprojectinfo
    template<typename Interface>
    struct has_get_module_function {
        // 这里可以根据命名约定来推断工厂函数名
        // 比如 Idmprojectinfo -> dmprojectinfoGetModule
        // 但由于每个接口的工厂函数名可能不同，这个检测比较复杂
        // 暂时提供一个通用的框架
        static constexpr bool value = false;
    };

    // 可以为特定接口特化这个模板
    // 例如：
    // template<>
    // struct has_get_module_function<Idmprojectinfo> {
    //     static constexpr bool value = true;
    // };

} // namespace dm_detail

template<typename Interface>
inline constexpr bool dm_has_get_module_function_v = dm_detail::has_get_module_function<Interface>::value;

//-----------------------------------------------------------------------------
// 安全的接口转换工具
//-----------------------------------------------------------------------------

/**
 * @brief 安全地将裸指针转换为 DmModulePtr
 * 只有当 T 是合法的接口类型时才允许转换
 */
template<typename T>
constexpr auto dm_make_module_ptr(T* ptr) -> std::enable_if_t<dm_is_interface_class_v<T>, DmModulePtr<T>> {
    return DmModulePtr<T>(ptr);
}

/**
 * @brief 检查 DmModulePtr 是否为空
 */
template<typename T>
constexpr bool dm_is_module_valid(const DmModulePtr<T>& ptr) {
    return ptr.get() != nullptr;
}

/**
 * @brief 获取接口的原始指针 (仅用于必要时)
 */
template<typename T>
constexpr T* dm_get_raw_interface(const DmModulePtr<T>& ptr) {
    return ptr.get();
}

//-----------------------------------------------------------------------------
// 接口类型约束 (用于模板参数)
//-----------------------------------------------------------------------------

/**
 * @brief 模板约束：要求 T 必须是接口类
 */
template<typename T>
using dm_require_interface = std::enable_if_t<dm_is_interface_class_v<T>, int>;

/**
 * @brief 模板约束：要求 T 必须是 DmModulePtr 类型
 */
template<typename T>
using dm_require_module_ptr = std::enable_if_t<dm_is_dm_module_ptr_v<T>, int>;

//-----------------------------------------------------------------------------
// 接口类型信息获取
//-----------------------------------------------------------------------------

/**
 * @brief 获取接口的实际类型名称 (编译时)
 * 可用于调试和日志
 */
template<typename T>
struct dm_interface_name {
    static constexpr const char* value = "Unknown Interface";
};

// 可以为特定接口特化这个模板
// 例如：
// template<>
// struct dm_interface_name<Idmprojectinfo> {
//     static constexpr const char* value = "Idmprojectinfo";
// };

template<typename T>
inline constexpr const char* dm_interface_name_v = dm_interface_name<T>::value;

//-----------------------------------------------------------------------------
// 实用宏定义 (简化接口声明)
//-----------------------------------------------------------------------------

/**
 * @brief 声明接口工厂函数类型萃取的特化
 */
#define DM_DECLARE_INTERFACE_TRAITS(InterfaceName, ModuleName) \
    namespace dm_detail { \
        template<> \
        struct has_get_module_function<InterfaceName> { \
            static constexpr bool value = true; \
        }; \
    } \
    template<> \
    struct dm_interface_name<InterfaceName> { \
        static constexpr const char* value = #InterfaceName; \
    };

/**
 * @brief 声明模块指针类型别名
 */
#define DM_DECLARE_MODULE_PTR_ALIAS(InterfaceName, AliasName) \
    typedef DmModulePtr<InterfaceName> AliasName;

// 使用示例：
// DM_DECLARE_INTERFACE_TRAITS(Idmprojectinfo, dmprojectinfo)
// DM_DECLARE_MODULE_PTR_ALIAS(Idmprojectinfo, dmprojectinfoPtr)

#endif // __DMINTERFACE_TRAITS_H_INCLUDE__