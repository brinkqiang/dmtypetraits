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
#include "dmtypetraits.h"

// --- 用于测试的辅助类型 ---
struct MyClass { int x; };
struct MyPolyClass { virtual ~MyPolyClass() = default; void func() {} };
enum LegacyEnum { A, B };
enum class ScopedEnum { X, Y };
struct DereferenceableButNotPointer { int operator*() const { return 0; } };

// --- 测试 dmtypetraits.h 中的复合萃取 ---
TEST(DmTypeTraitsTest, IsContainer) {
    ASSERT_TRUE((dm_is_container_v<std::vector<int>>));
    ASSERT_TRUE((dm_is_container_v<std::string>));
    ASSERT_TRUE((dm_is_container_v<std::list<double>>));
    ASSERT_TRUE((dm_is_container_v<std::map<int, double>>));
    ASSERT_FALSE((dm_is_container_v<int>));
    ASSERT_FALSE((dm_is_container_v<int[10]>));
    ASSERT_FALSE((dm_is_container_v<MyClass>));
}

TEST(DmTypeTraitsTest, IsScopedEnum) {
    ASSERT_TRUE((dm_is_scoped_enum_v<ScopedEnum>));
    ASSERT_FALSE((dm_is_scoped_enum_v<LegacyEnum>));
    ASSERT_FALSE((dm_is_scoped_enum_v<int>));
}

TEST(DmTypeTraitsTest, IsStringLike) {
    ASSERT_TRUE((dm_is_string_like_v<std::string>));
    ASSERT_TRUE((dm_is_string_like_v<const char*>));
    const char fixed_array[] = "hello";
    ASSERT_TRUE((dm_is_string_like_v<decltype(fixed_array)>));
    ASSERT_FALSE((dm_is_string_like_v<std::vector<char>>));
}

TEST(DmTypeTraitsTest, IsPointerLike) {
    ASSERT_TRUE((dm_is_pointer_like_v<int*>));
    ASSERT_TRUE((dm_is_pointer_like_v<std::unique_ptr<MyClass>>));
    ASSERT_FALSE((dm_is_pointer_like_v<DereferenceableButNotPointer>));
}

// --- 测试 dmtypetraits_extensions.h 中的萃取 ---
TEST(DmTypeTraitsExtensionsTest, IsSequenceContainer) {
    ASSERT_TRUE((dm_is_sequence_container_v<std::vector<int>>));
    ASSERT_FALSE((dm_is_sequence_container_v<std::map<int, int>>));
    ASSERT_FALSE((dm_is_sequence_container_v<std::array<int, 5>>));
}

TEST(DmTypeTraitsExtensionsTest, IsAssociativeContainer) {
    ASSERT_TRUE((dm_is_associative_container_v<std::map<int, int>>));
    ASSERT_TRUE((dm_is_associative_container_v<std::unordered_set<int>>));
    ASSERT_FALSE((dm_is_associative_container_v<std::vector<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsMapContainer) {
    ASSERT_TRUE((dm_is_map_container_v<std::map<int, float>>));
    ASSERT_FALSE((dm_is_map_container_v<std::set<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsSetContainer) {
    ASSERT_TRUE((dm_is_set_container_v<std::set<int>>));
    ASSERT_FALSE((dm_is_set_container_v<std::map<int, int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsAnyArray) {
    ASSERT_TRUE((dm_is_any_array_v<int[10]>));
    ASSERT_TRUE((dm_is_any_array_v<std::array<MyClass, 5>>));
    ASSERT_FALSE((dm_is_any_array_v<std::vector<int>>));
}

TEST(DmTypeTraitsExtensionsTest, IsIterable) {
    ASSERT_TRUE((dm_is_iterable_v<std::vector<int>>));
    ASSERT_FALSE((dm_is_iterable_v<int[10]>));
    ASSERT_TRUE((dm_is_iterable_v<std::array<int, 3>>));
    ASSERT_FALSE((dm_is_iterable_v<MyClass>));
}

TEST(DmTypeTraitsExtensionsTest, IsInvocable) {
    auto lambda = []() {};
    ASSERT_TRUE((dm_is_invocable_v<decltype(lambda)>));
    ASSERT_TRUE((dm_is_invocable_v<void(*)()>));
    ASSERT_FALSE((dm_is_invocable_v<int>));
}

TEST(DmTypeTraitsExtensionsTest, IsSmartPointer) {
    ASSERT_TRUE((dm_is_smart_pointer_v<std::unique_ptr<int>>));
    ASSERT_FALSE((dm_is_smart_pointer_v<int*>));
}

TEST(DmTypeTraitsExtensionsTest, IsTupleLike) {
    ASSERT_TRUE((dm_is_tuple_like_v<std::tuple<int, float>>));
    ASSERT_TRUE((dm_is_tuple_like_v<std::pair<int, char>>));
    ASSERT_FALSE((dm_is_tuple_like_v<std::array<int, 2>>));
}

TEST(DmTypeTraitsNumericTest, IsSignedInteger) {
    ASSERT_TRUE((dm_is_signed_integer_v<int>));
    ASSERT_FALSE((dm_is_signed_integer_v<unsigned int>));
    ASSERT_FALSE((dm_is_signed_integer_v<char>));
}

TEST(DmTypeTraitsNumericTest, IsUnsignedInteger) {
    ASSERT_TRUE((dm_is_unsigned_integer_v<unsigned int>));
    ASSERT_FALSE((dm_is_unsigned_integer_v<int>));
    ASSERT_FALSE((dm_is_unsigned_integer_v<bool>));
}

TEST(DmTypeTraitsNumericTest, IsNumeric) {
    ASSERT_TRUE((dm_is_numeric_v<int>));
    ASSERT_TRUE((dm_is_numeric_v<float>));
    ASSERT_FALSE((dm_is_numeric_v<MyClass>));
}

// --- 测试实用工具萃取 ---

TEST(DmTypeTraitsUtilityTest, ElementType) {
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::vector<MyClass>>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<int[5]>, int>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::array<float, 10>>, float>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<const char*>, const char>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<std::string>, char>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<int>, void>));
    ASSERT_TRUE((std::is_same_v<dm_element_type_t<MyClass>, void>));
}

TEST(DmTypeTraitsUtilityTest, RemoveAllPointers) {
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<int**>, int>));
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<const int*>, const int>));
    // --- 修正点 ---
    // 原期望值为 const MyClass* const，是错误的。
    // 正确的期望值是移除所有指针后剩下的 const MyClass。
    ASSERT_TRUE((std::is_same_v<dm_remove_all_pointers_t<const MyClass* const* volatile*>, const MyClass>));
}

TEST(DmTypeTraitsUtilityTest, PureType) {
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const volatile int&>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const int* const>, int>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<const volatile MyClass* const* &>, MyClass>));
    ASSERT_TRUE((std::is_same_v<dm_pure_type_t<int[5]>, int*>));
}