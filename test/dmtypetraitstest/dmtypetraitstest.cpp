#include "gtest.h"
#include "dmtypetraits.h" // 包含总头文件

#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include "dmformat.h"
// --- 测试用的辅助类型和函数 ---
struct MyClass {
    void member_func(const int&) const {}
    int member_var;
};

enum OldEnum { A, B };
enum class ScopedEnum { X, Y };

void free_function(double, bool) {}

// --- Test Suite for dmbase_typetraits.h ---
TEST(DmBaseTypeTraitsTest, BasicTraits) {
    // 主要类型类别
    static_assert(dm_is_integral_v<int>, "Test Failed: int is integral");
    static_assert(!dm_is_integral_v<float>, "Test Failed: float is not integral");
    static_assert(dm_is_pointer_v<int*>, "Test Failed: int* is pointer");
    static_assert(dm_is_class_v<MyClass>, "Test Failed: MyClass is a class");
    static_assert(dm_is_enum_v<OldEnum>, "Test Failed: OldEnum is an enum");

    // 类型关系
    static_assert(dm_is_same_v<int, int>, "Test Failed: int is same as int");
    static_assert(!dm_is_same_v<int, unsigned int>, "Test Failed: int is not same as unsigned int");
    static_assert(dm_is_base_of_v<std::true_type, std::is_integral<int>>, "Test Failed");

    // 类型转换
    static_assert(dm_is_same_v<dm_remove_cv_t<const volatile int>, int>, "Test Failed: remove_cv");
    static_assert(dm_is_same_v<dm_remove_reference_t<int&>, int>, "Test Failed: remove_reference");
    static_assert(dm_is_same_v<dm_decay_t<const char(&)[6]>, const char*>, "Test Failed: decay");
    static_assert(dm_is_same_v<dm_remove_cvref_t<const int&>, int>, "Test Failed: remove_cvref");
}

// --- Test Suite for dmcore_typetraits.h ---
TEST(DmCoreTypeTraitsTest, CompositeTraits) {
    // dm_is_container_v
    static_assert(dm_is_container_v<std::vector<int>>, "Test Failed: vector is a container");
    static_assert(dm_is_container_v<std::string>, "Test Failed: string is a container");
    static_assert(!dm_is_container_v<int[5]>, "Test Failed: C-array is not a container by our definition");
    static_assert(!dm_is_container_v<int>, "Test Failed: int is not a container");

    // dm_is_scoped_enum_v
    static_assert(dm_is_scoped_enum_v<ScopedEnum>, "Test Failed: ScopedEnum is a scoped enum");
    static_assert(!dm_is_scoped_enum_v<OldEnum>, "Test Failed: OldEnum is not a scoped enum");

    // dm_is_string_like_v
    static_assert(dm_is_string_like_v<std::string>, "Test Failed: std::string is string-like");
    static_assert(dm_is_string_like_v<const char*>, "Test Failed: const char* is string-like");
    static_assert(dm_is_string_like_v<std::string_view>, "Test Failed: string_view is string-like");
    static_assert(!dm_is_string_like_v<int>, "Test Failed: int is not string-like");

    // dm_is_pointer_like_v
    static_assert(dm_is_pointer_like_v<int*>, "Test Failed: raw pointer is pointer-like");
    static_assert(dm_is_pointer_like_v<std::unique_ptr<int>>, "Test Failed: unique_ptr is pointer-like");
    static_assert(!dm_is_pointer_like_v<std::vector<int>>, "Test Failed: vector is not pointer-like");
}

// --- Test Suite for dmtypetraits_extensions.h ---
TEST(DmExtensionsTypeTraitsTest, AdvancedTraits) {
    // 容器分类
    static_assert(dm_is_sequence_container_v<std::vector<int>>, "Test Failed");
    static_assert(dm_is_associative_container_v<std::map<int, char>>, "Test Failed");
    static_assert(dm_is_map_container_v<std::map<int, char>>, "Test Failed");
    static_assert(dm_is_set_container_v<std::set<int>>, "Test Failed");
    static_assert(!dm_is_map_container_v<std::set<int>>, "Test Failed");

    // 数组判断
    static_assert(dm_is_any_array_v<int[10]>, "Test Failed");
    static_assert(dm_is_any_array_v<std::array<int, 10>>, "Test Failed");
    static_assert(!dm_is_any_array_v<std::vector<int>>, "Test Failed");

    // 指针判断
    static_assert(dm_is_smart_pointer_v<std::shared_ptr<int>>, "Test Failed");
    static_assert(!dm_is_smart_pointer_v<int*>, "Test Failed");

    // 元素类型萃取
    static_assert(dm_is_same_v<dm_element_type_t<std::vector<MyClass>>, MyClass>, "Test Failed");
    static_assert(dm_is_same_v<dm_element_type_t<int*>, int>, "Test Failed");
    static_assert(dm_is_same_v<dm_element_type_t<const int[5]>, const int>, "Test Failed");

    // 高级类型转换
    static_assert(dm_is_same_v<dm_remove_all_pointers_t<int***>, int>, "Test Failed");
    static_assert(dm_is_same_v<dm_pure_type_t<const volatile int* const&>, int>, "Test Failed");
    static_assert(dm_is_same_v<dm_copy_cvref_t<const int&, float>, const float&>, "Test Failed");
    static_assert(dm_is_same_v<dm_copy_cvref_t<int&&, char>, char&&>, "Test Failed");
    static_assert(dm_is_same_v<dm_copy_cvref_t<const volatile MyClass, int>, const volatile int>, "Test Failed");

    // dm_type_name
    EXPECT_TRUE(dm_type_name<int>().find("int") != std::string_view::npos);
    EXPECT_TRUE(dm_type_name<MyClass>().find("MyClass") != std::string_view::npos);
}

// --- Test Suite for dmtypetraits_logical.h ---
TEST(DmLogicalTypeTraitsTest, LogicalCombiners) {
    static_assert(dm_conjunction_v<std::is_integral<int>, std::is_signed<int>>, "Test Failed");
    static_assert(!dm_conjunction_v<std::is_integral<int>, std::is_class<int>>, "Test Failed");

    static_assert(dm_disjunction_v<std::is_class<int>, std::is_integral<int>>, "Test Failed");
    static_assert(!dm_disjunction_v<std::is_class<int>, std::is_pointer<int>>, "Test Failed");

    static_assert(dm_negation_v<std::is_const<int>>, "Test Failed");
}


// --- Test Suite for dmtypetraits_function.h ---
TEST(DmFunctionTypeTraitsTest, FunctionTraits) {
    auto lambda = [](const std::string& s, int* p) -> bool { return s.empty() && p; };

    // 返回类型萃取
    static_assert(dm_is_same_v<dm_function_return_t<decltype(lambda)>, bool>, "Test Failed");

    // --- 测试“纯粹”参数类型 (旧功能) ---
    using clean_params = dm_function_parameters_t<decltype(lambda)>;
    static_assert(dm_is_same_v<clean_params, std::tuple<std::string, int*>>, "Test Failed: Clean params should strip cv-ref");
    static_assert(dm_is_same_v<dm_function_arg_t<0, decltype(lambda)>, std::string>, "Test Failed: 0-th clean arg should be std::string");

    // --- 测试“原始”参数类型 (新功能) ---
    using raw_params = dm_function_raw_parameters_t<decltype(lambda)>;
    static_assert(dm_is_same_v<raw_params, std::tuple<const std::string&, int*>>, "Test Failed: Raw params should keep cv-ref");
    static_assert(dm_is_same_v<dm_function_raw_arg_t<0, decltype(lambda)>, const std::string&>, "Test Failed: 0-th raw arg should be const std::string&");


    // 成员函数测试
    using member_func_clean_params = dm_function_parameters_t<decltype(&MyClass::member_func)>;
    static_assert(dm_is_same_v<member_func_clean_params, std::tuple<int>>, "Test Failed");

    using member_func_raw_params = dm_function_raw_parameters_t<decltype(&MyClass::member_func)>;
    static_assert(dm_is_same_v<member_func_raw_params, std::tuple<const int&>>, "Test Failed"); // int没有cv-ref，所以两者相同

    // 类类型萃取
    static_assert(dm_is_same_v<dm_function_class_t<decltype(&MyClass::member_func)>, MyClass>, "Test Failed");

    fmt::print("{}\n", dm_type_name<clean_params>());
    fmt::print("{}\n", dm_type_name<member_func_clean_params>());
    fmt::print("{}\n", dm_type_name<member_func_raw_params>());
}
// --- Test Suite for dmtypetraits_typelist.h ---
TEST(DmTypeListTest, TypeListOperations) {
    using MyList = dm_typelist<int, float, const char*, int>;

    // 查询
    static_assert(dm_typelist_size_v<MyList> == 4, "Test Failed: size");
    static_assert(!dm_typelist_is_empty_v<MyList>, "Test Failed: is_empty");
    static_assert(dm_typelist_is_empty_v<dm_typelist<>>, "Test Failed: is_empty");
    static_assert(dm_is_same_v<dm_typelist_at_t<1, MyList>, float>, "Test Failed: at");
    static_assert(dm_is_same_v<dm_typelist_front_t<MyList>, int>, "Test Failed: front");
    static_assert(dm_typelist_contains_v<float, MyList>, "Test Failed: contains true");
    static_assert(!dm_typelist_contains_v<double, MyList>, "Test Failed: contains false");

    // 操作
    using PushedFront = dm_typelist_push_front_t<MyList, bool>;

    static_assert(dm_is_same_v<PushedFront, dm_typelist<bool, int, float, const char*, int>>, "Test Failed");

    using PushedBack = dm_typelist_push_back_t<MyList, bool>;
    static_assert(dm_is_same_v<PushedBack, dm_typelist<int, float, const char*, int, bool>>, "Test Failed");

    using PoppedFront = dm_typelist_pop_front_t<MyList>;
    static_assert(dm_is_same_v<PoppedFront, dm_typelist<float, const char*, int>>, "Test Failed");

    // 算法
    using Pointers = dm_typelist_transform_t<dm_typelist<int, char>, std::add_pointer>;
    static_assert(dm_is_same_v<Pointers, dm_typelist<int*, char*>>, "Test Failed: transform");

    using Integrals = dm_typelist_filter_t<dm_typelist<int, float, char, long, double>, std::is_integral>;
    static_assert(dm_is_same_v<Integrals, dm_typelist<int, char, long>>, "Test Failed: filter");


    fmt::print("{}\n", dm_type_name<PushedFront>());
    fmt::print("{}\n", dm_type_name<PushedBack>());
    fmt::print("{}\n", dm_type_name<PoppedFront>());
    fmt::print("{}\n", dm_type_name<Pointers>());
    fmt::print("{}\n", dm_type_name<Integrals>());
}