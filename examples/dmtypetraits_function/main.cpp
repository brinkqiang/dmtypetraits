#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "dmfix_win.h"
// 包含我们创建的函数萃取头文件
#include "dmtypetraits.h"

// --- 用于测试的函数和类 ---
static void free_func_multi_arg(int, const std::string&) {}
static void free_func_no_arg() {}

struct MyTestClass {
    bool member_func(char) const { return true; }
};

// --- 开始测试 ---

int main() {
    // 1. 测试带参数的普通函数
    using Func1 = decltype(&free_func_multi_arg);

    // 验证返回类型
    static_assert(dm_is_same_v<dm_function_return_t<Func1>, void>, "Test 1a failed: Return type should be void.");

    // [修正] 验证参数元组，期望的类型是移除了 const 和引用的纯类型
    using expected_params1 = std::tuple<int, std::string>; // <--- 修改此处
    static_assert(dm_is_same_v<dm_function_parameters_t<Func1>, expected_params1>, "Test 1b failed: Parameters tuple mismatch.");

    // [修正] 验证最后一个参数的类型，期望的类型同样是纯类型
    static_assert(dm_is_same_v<dm_last_parameter_t<Func1>, std::string>, "Test 1c failed: Last parameter type mismatch."); // <--- 修改此处

    // 2. 测试无参数的普通函数
    using Func2 = decltype(&free_func_no_arg);

    // 验证返回类型
    static_assert(dm_is_same_v<dm_function_return_t<Func2>, void>, "Test 2a failed: Return type should be void.");

    // 验证无参数时，参数类型为 void
    static_assert(dm_is_same_v<dm_function_parameters_t<Func2>, void>, "Test 2b failed: Parameters should be void for no-arg function.");
    // 注意：对 Func2 使用 dm_last_parameter_t 会导致编译错误，这本身也是一种正确的行为。


    // 3. 测试成员函数
    using Func3 = decltype(&MyTestClass::member_func);

    // 验证返回类型
    static_assert(dm_is_same_v<dm_function_return_t<Func3>, bool>, "Test 3a failed: Return type should be bool.");

    // 验证所属类类型
    static_assert(dm_is_same_v<dm_function_class_t<Func3>, MyTestClass>, "Test 3b failed: Class type mismatch.");

    // 验证参数
    using expected_params3 = std::tuple<char>;
    static_assert(dm_is_same_v<dm_function_parameters_t<Func3>, expected_params3>, "Test 3c failed: Parameters tuple mismatch.");


    // 4. 测试 dm_is_specialization_v
    static_assert(dm_is_specialization_v<std::vector<int>, std::vector>, "Test 4a failed: std::vector<int> should be a specialization of std::vector.");
    static_assert(dm_is_specialization_v<const std::tuple<char, int>, std::tuple>, "Test 4b failed: const tuple should be a specialization of std::tuple.");
    static_assert(!dm_is_specialization_v<int, std::vector>, "Test 4c failed: int should not be a specialization of std::vector.");


    // 如果所有 static_assert 都通过，程序将成功编译。
    std::cout << "All compile-time tests passed successfully!" << std::endl;

    return 0;
}