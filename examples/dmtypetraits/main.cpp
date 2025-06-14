#include "dmtypetraits.h"
#include <iostream>
#include <string>

// --- 函数模板定义 ---

#if __cplusplus >= 202002L
// C++20 版本: 使用 concept 进行约束
template<dm_integral T>
void process_value(T val) {
    std::cout << "[C++20 Concept] Processing an integral value: " << val << std::endl;
}

template<dm_floating_point T>
void process_value(T val) {
    std::cout << "[C++20 Concept] Processing a floating point value: " << val << std::endl;
}

#else
// C++17 版本: 使用 SFINAE 和 dm_is_..._v
template<typename T, std::enable_if_t<dm_is_integral_v<T>, int> = 0>
void process_value(T val) {
    std::cout << "[C++17 SFINAE] Processing an integral value: " << val << std::endl;
}

template<typename T, std::enable_if_t<dm_is_floating_point_v<T>, int> = 0>
void process_value(T val) {
    std::cout << "[C++17 SFINAE] Processing a floating point value: " << val << std::endl;
}
#endif

// 通用版本，处理其他类型
void process_value(const std::string& val) {
    std::cout << "Processing a string: " << val << std::endl;
}


int main() {
    std::cout << std::boolalpha;
    std::cout << "Is int an integral? " << dm_is_integral_v<int> << std::endl;
    std::cout << "Is std::string a class? " << dm_is_class_v<std::string> << std::endl;
    std::cout << "---" << std::endl;

    // 根据编译环境调用不同版本的 process_value
    process_value(123);
    process_value(456.789);
    process_value(std::string("hello"));

#if __cplusplus >= 202002L
    std::cout << "--- C++20 features are enabled ---" << std::endl;
    using my_type = const volatile int&;
    using stripped_type = dm_remove_cvref_t<my_type>;
    std::cout << "Is dm_remove_cvref_t<const volatile int&> same as int? "
        << dm_is_same_v<stripped_type, int> << std::endl;
#else
    std::cout << "--- C++17 mode, C++20 features are disabled ---" << std::endl;
#endif

    return 0;
}