#include <iostream>
#include <string>
#include <vector>

#include "dmfix_win.h"
// 包含我们创建的类型列表头文件
#include "dmtypetraits_typelist.h"

// --- 用于测试的辅助元函数 ---

// 用于 transform 测试：为类型添加指针
template<typename T>
struct add_pointer {
    using type = T*;
};

// 用于 filter 测试：判断类型是否为指针
template<typename T>
struct is_pointer : std::is_pointer<T> {};


// --- 开始测试 ---

int main() {
    // 定义一些用于测试的类型列表
    using EmptyList = dm_typelist<>;
    using SingleList = dm_typelist<int>;
    using MultiList = dm_typelist<char, int, const double, std::string>;

    // --- 1. 测试类型列表查询 (Queries) ---

    // 1a. dm_typelist_size_v
    static_assert(dm_typelist_size_v<EmptyList> == 0, "Test 1a failed: Size of empty list should be 0.");
    static_assert(dm_typelist_size_v<SingleList> == 1, "Test 1a failed: Size of single list should be 1.");
    static_assert(dm_typelist_size_v<MultiList> == 4, "Test 1a failed: Size of multi list should be 4.");

    // 1b. dm_typelist_is_empty_v
    static_assert(dm_typelist_is_empty_v<EmptyList>, "Test 1b failed: EmptyList should be empty.");
    static_assert(!dm_typelist_is_empty_v<MultiList>, "Test 1b failed: MultiList should not be empty.");

    // 1c. dm_typelist_at_t
    static_assert(dm_is_same_v<dm_typelist_at_t<0, MultiList>, char>, "Test 1c failed: Type at index 0 mismatch.");
    static_assert(dm_is_same_v<dm_typelist_at_t<1, MultiList>, int>, "Test 1c failed: Type at index 1 mismatch.");
    static_assert(dm_is_same_v<dm_typelist_at_t<2, MultiList>, const double>, "Test 1c failed: Type at index 2 mismatch.");
    static_assert(dm_is_same_v<dm_typelist_at_t<3, MultiList>, std::string>, "Test 1c failed: Type at index 3 mismatch.");

    // 1d. dm_typelist_front_t
    static_assert(dm_is_same_v<dm_typelist_front_t<MultiList>, char>, "Test 1d failed: Front type mismatch.");

    // 1e. dm_typelist_contains_v (需要 C++17)
#if __cplusplus >= 201703L
    static_assert(dm_typelist_contains_v<int, MultiList>, "Test 1e failed: MultiList should contain int.");
    static_assert(dm_typelist_contains_v<const double, MultiList>, "Test 1e failed: MultiList should contain const double.");
    static_assert(!dm_typelist_contains_v<float, MultiList>, "Test 1e failed: MultiList should not contain float.");
    static_assert(!dm_typelist_contains_v<int, EmptyList>, "Test 1e failed: EmptyList should not contain any type.");
#endif

    // --- 2. 测试类型列表操作 (Operations) ---

    // 2a. dm_typelist_push_front_t
    using PushedFront = dm_typelist_push_front_t<MultiList, bool, void*>;
    using ExpectedPushFront = dm_typelist<bool, void*, char, int, const double, std::string>;
    static_assert(dm_is_same_v<PushedFront, ExpectedPushFront>, "Test 2a failed: push_front result mismatch.");

    // 2b. dm_typelist_push_back_t
    using PushedBack = dm_typelist_push_back_t<MultiList, bool, void*>;
    using ExpectedPushBack = dm_typelist<char, int, const double, std::string, bool, void*>;
    static_assert(dm_is_same_v<PushedBack, ExpectedPushBack>, "Test 2b failed: push_back result mismatch.");

    // 2c. dm_typelist_pop_front_t
    using PoppedFront = dm_typelist_pop_front_t<MultiList>;
    using ExpectedPopFront = dm_typelist<int, const double, std::string>;
    static_assert(dm_is_same_v<PoppedFront, ExpectedPopFront>, "Test 2c failed: pop_front result mismatch.");


    // --- 3. 测试类型列表算法 (Algorithms) ---

    // 3a. dm_typelist_transform_t
    using TransformedList = dm_typelist_transform_t<dm_typelist<int, std::string>, add_pointer>;
    using ExpectedTransform = dm_typelist<int*, std::string*>;
    static_assert(dm_is_same_v<TransformedList, ExpectedTransform>, "Test 3a failed: transform with custom metafunction failed.");
    
    // 使用标准库元函数进行 transform
    using TransformedConstList = dm_typelist_transform_t<dm_typelist<char, int>, std::add_const>;
    using ExpectedConstTransform = dm_typelist<const char, const int>;
    static_assert(dm_is_same_v<TransformedConstList, ExpectedConstTransform>, "Test 3a failed: transform with std::add_const failed.");

    // 3b. dm_typelist_filter_t
    using ToBeFilteredList = dm_typelist<int, char*, float, double*, void, long*>;
    using FilteredList = dm_typelist_filter_t<ToBeFilteredList, is_pointer>;
    using ExpectedFilter = dm_typelist<char*, double*, long*>;
    static_assert(dm_is_same_v<FilteredList, ExpectedFilter>, "Test 3b failed: filter result mismatch.");

    // 测试 filter 后为空列表的情况
    using FilteredEmpty = dm_typelist_filter_t<dm_typelist<int, char>, is_pointer>;
    static_assert(dm_is_same_v<FilteredEmpty, EmptyList>, "Test 3b failed: filter result should be an empty list.");


    // 如果所有 static_assert 都通过，程序将成功编译并运行。
    std::cout << "All compile-time tests for dmtypetraits_typelist passed successfully!" << std::endl;

    return 0;
}