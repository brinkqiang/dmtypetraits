#include <gtest.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <tuple>

// 包含我们要测试的头文件
// dmtypetraits_extensions.h 会自动包含 dmtypetraits.h 和 dmbase_typetraits.h
#include "dmtypetraits_extensions.h"

// --- 用于测试的辅助类型 ---

// 一个简单的类
struct MyClass {
    int x;
};

// 一个带虚函数的类 (多态)
struct MyPolyClass {
    virtual ~MyPolyClass() = default;
    void func() {}
};

// C-style 枚举
enum LegacyEnum { A, B };

// C++11 强类型枚举
enum class ScopedEnum { X, Y };

// 一个不像指针但可解引用的类型
struct DereferenceableButNotPointer {
    int operator*() const { return 0; }
};


// --- 测试 dmtypetraits.h 中的复合萃取 ---

TEST(DmTypeTraitsTest, IsContainer) {
    ASSERT_TRUE((dm_is_container_v<std::vector<int>>));
    ASSERT_TRUE((dm_is_container_v<std::string>));
    ASSERT_TRUE((dm_is_container_v<std::list<double>>));
    ASSERT_TRUE((dm_is_container_v<std::map<int, double>>));
    ASSERT_FALSE((dm_is_container_v<int>));
    ASSERT_FALSE((dm_is_container_v<int[10]>)); // C-style 数组没有 .size() 等成员
    ASSERT_FALSE((dm_is_container_v<MyClass>));
}

TEST(DmTypeTraitsTest, IsScopedEnum) {
    ASSERT_TRUE((dm_is_scoped_enum_v<ScopedEnum>));
    ASSERT_FALSE((dm_is_scoped_enum_v<LegacyEnum>));
    ASSERT_FALSE((dm_is_scoped_enum_v<int>));
    ASSERT_FALSE((dm_is_scoped_enum_v<MyClass>));
}

TEST(DmTypeTraitsTest, IsStringLike) {
    ASSERT_TRUE((dm_is_string_like_v<std::string>));
    ASSERT_TRUE((dm_is_string_like_v<const char*>));
    ASSERT_TRUE((dm_is_string_like_v<char*>));
    ASSERT_TRUE((dm_is_string_like_v<std::string_view>));
    const char fixed_array[] = "hello";
    ASSERT_TRUE((dm_is_string_like_v<decltype(fixed_array)>));

    ASSERT_FALSE((dm_is_string_like_v<int>));
    ASSERT_FALSE((dm_is_string_like_v<std::vector<char>>));
}

TEST(DmTypeTraitsTest, IsPointerLike) {
    ASSERT_TRUE((dm_is_pointer_like_v<int*>));
    ASSERT_TRUE((dm_is_pointer_like_v<const MyClass*>));
    ASSERT_TRUE((dm_is_pointer_like_v<std::unique_ptr<MyClass>>));
    ASSERT_TRUE((dm_is_pointer_like_v<std::shared_ptr<int>>));
    
    ASSERT_FALSE((dm_is_pointer_like_v<int>));
    ASSERT_FALSE((dm_is_pointer_like_v<MyClass>));
    ASSERT_FALSE((dm_is_pointer_like_v<std::vector<int>>));
    // 这个类型虽然可以解引用，但是没有 -> 操作，也不是指针，所以为 false
    ASSERT_FALSE((dm_is_pointer_like_v<DereferenceableButNotPointer>)); 
}


// --- 测试 dmtypetraits_extensions.h 中的萃取 ---

TEST(DmTypeTraitsExtensionsTest, IsSequenceContainer) {
    ASSERT_TRUE((dm_is_sequence_container_v<std::vector<int>>));
    ASSERT_TRUE((dm_is_sequence_container_v<std::list<int>>));
    ASSERT_TRUE((dm_is_sequence_container_v<std::string>)); // string 也有 push_back

    ASSERT_FALSE((dm_is_sequence_container_v<std::map<int, int>>));
    ASSERT_FALSE((dm_is_sequence_container_v<std::set<int>>));
    ASSERT_FALSE((dm_is_sequence_container_v<std::array<int, 5>>)); // std::array 没有 push_back
}

TEST(DmTypeTraitsExtensionsTest, IsAssociativeContainer) {
    ASSERT_TRUE((dm_is_associative_container_v<std::map<int, int>>));
    ASSERT_TRUE((dm_is_associative_container_v<std::set<int>>));
    ASSERT_TRUE((dm_is_associative_container_v<std::multimap<int, int>>));
    ASSERT_TRUE((dm_is_associative_container_v<std::unordered_set<int>>));
    
    ASSERT_FALSE((dm_is_associative_container_v<std::vector<int>>));
    ASSERT_FALSE((dm_is_associative_container_v<std::string>));
}

TEST(DmTypeTraitsExtensionsTest, IsMapContainer) {
    ASSERT_TRUE((dm_is_map_container_v<std::map<int, float>>));
    ASSERT_TRUE((dm_is_map_container_v<std::unordered_map<int, float>>));
    
    ASSERT_FALSE((dm_is_map_container_v<std::set<int>>));
    ASSERT_FALSE((dm_is_map_container_v<std::vector<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsSetContainer) {
    ASSERT_TRUE((dm_is_set_container_v<std::set<int>>));
    ASSERT_TRUE((dm_is_set_container_v<std::unordered_set<int>>));
    
    ASSERT_FALSE((dm_is_set_container_v<std::map<int, int>>));
    ASSERT_FALSE((dm_is_set_container_v<std::vector<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsAnyArray) {
    ASSERT_TRUE((dm_is_any_array_v<int[10]>));
    ASSERT_TRUE((dm_is_any_array_v<const char[]>));
    ASSERT_TRUE((dm_is_any_array_v<std::array<MyClass, 5>>));
    
    ASSERT_FALSE((dm_is_any_array_v<int*>));
    ASSERT_FALSE((dm_is_any_array_v<std::vector<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsIterable) {
    ASSERT_TRUE((dm_is_iterable_v<std::vector<int>>));
    // 根据当前 has_begin 的 SFINAE 实现，它检测 T::begin()，所以C数组为false
    ASSERT_FALSE((dm_is_iterable_v<int[10]>));
    ASSERT_TRUE((dm_is_iterable_v<std::string>));
    ASSERT_TRUE((dm_is_iterable_v<std::array<int, 3>>));

    ASSERT_FALSE((dm_is_iterable_v<MyClass>));
    ASSERT_FALSE((dm_is_iterable_v<int>));
}

TEST(DmTypeTraitsExtensionsTest, IsInvocable) {
    auto lambda = []() {};
    ASSERT_TRUE((dm_is_invocable_v<decltype(lambda)>));
    ASSERT_TRUE((dm_is_invocable_v<void()>)); // 函数类型
    ASSERT_TRUE((dm_is_invocable_v<void(*)()>)); // 函数指针
    
    ASSERT_FALSE((dm_is_invocable_v<int>));
    ASSERT_FALSE((dm_is_invocable_v<MyClass>));
}

TEST(DmTypeTraitsExtensionsTest, IsSmartPointer) {
    ASSERT_TRUE((dm_is_smart_pointer_v<std::unique_ptr<int>>));
    ASSERT_TRUE((dm_is_smart_pointer_v<std::shared_ptr<MyClass>>));
    
    ASSERT_FALSE((dm_is_smart_pointer_v<int*>)); // 裸指针不是智能指针
    ASSERT_FALSE((dm_is_smart_pointer_v<MyClass>));
}

TEST(DmTypeTraitsExtensionsTest, IsTupleLike) {
    ASSERT_TRUE((dm_is_tuple_like_v<std::tuple<int, float>>));
    ASSERT_TRUE((dm_is_tuple_like_v<std::pair<int, char>>));
    
    ASSERT_FALSE((dm_is_tuple_like_v<std::array<int, 2>>));
    ASSERT_FALSE((dm_is_tuple_like_v<MyClass>));
}

// --- 测试数值类型萃取 ---

TEST(DmTypeTraitsNumericTest, IsSignedInteger) {
    ASSERT_TRUE((dm_is_signed_integer_v<int>));
    ASSERT_TRUE((dm_is_signed_integer_v<long long>));
    ASSERT_TRUE((dm_is_signed_integer_v<signed char>));
    
    ASSERT_FALSE((dm_is_signed_integer_v<unsigned int>));
    ASSERT_FALSE((dm_is_signed_integer_v<bool>));
    ASSERT_FALSE((dm_is_signed_integer_v<char>));
    ASSERT_FALSE((dm_is_signed_integer_v<float>));
}

TEST(DmTypeTraitsNumericTest, IsUnsignedInteger) {
    ASSERT_TRUE((dm_is_unsigned_integer_v<unsigned int>));
    ASSERT_TRUE((dm_is_unsigned_integer_v<uint64_t>));
    ASSERT_TRUE((dm_is_unsigned_integer_v<unsigned char>));

    ASSERT_FALSE((dm_is_unsigned_integer_v<int>));
    ASSERT_FALSE((dm_is_unsigned_integer_v<bool>));
    ASSERT_FALSE((dm_is_unsigned_integer_v<double>));
}

TEST(DmTypeTraitsNumericTest, IsNumeric) {
    ASSERT_TRUE((dm_is_numeric_v<int>));
    ASSERT_TRUE((dm_is_numeric_v<unsigned long>));
    ASSERT_TRUE((dm_is_numeric_v<float>));
    ASSERT_TRUE((dm_is_numeric_v<double>));
    ASSERT_TRUE((dm_is_numeric_v<bool>));
    
    ASSERT_FALSE((dm_is_numeric_v<MyClass>));
    ASSERT_FALSE((dm_is_numeric_v<const char*>));
}

// --- 测试实用工具萃取 ---

TEST(DmTypeTraitsUtilityTest, ElementType) {
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::vector<MyClass>>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<int[5]>, int>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::array<float, 10>>, float>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<const char*>, const char>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::string>, char>));
    
    // 对于非容器/数组/指针类型，应为 void (或未定义，取决于SFINAE实现)
    // 修正后的 dm_element_type 对于没有 value_type 的类型会得到 void
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<int>, void>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<MyClass>, void>));
}

TEST(DmTypeTraitsUtilityTest, RemoveAllPointers) {
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int*>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int**>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<MyClass***>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<const int*>, const int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int* const>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<const MyClass* const* volatile*>, const MyClass* const>));
}

TEST(DmTypeTraitsUtilityTest, PureType) {
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<int>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const int>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<int&>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const volatile int&>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<int*>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const int* const>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<MyClass**>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const volatile MyClass* const* &>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<int[5]>, int*>)); // C++20的remove_cvref结合decay，数组会退化为指针
}