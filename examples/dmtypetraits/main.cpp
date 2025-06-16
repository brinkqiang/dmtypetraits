#include "dmtypetraits.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "dmfix_win.h"
// 用于测试的枚举
enum CStyleEnum { A, B };
enum class ScopedEnum { X, Y };

// 用于测试的自定义智能指针
template<typename T>
class MySmartPtr {
public:
    explicit MySmartPtr(T* p = nullptr) : ptr_(p) {}
    ~MySmartPtr() { delete ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
private:
    T* ptr_;
};

int main() {
    std::cout << std::boolalpha;

    // 1. 测试 dm_is_container_v
    std::cout << "--- Testing dm_is_container_v ---" << std::endl;
    std::cout << "std::vector<int> is a container? " << dm_is_container_v<std::vector<int>> << std::endl;
    std::cout << "std::string is a container? " << dm_is_container_v<std::string> << std::endl;
    std::cout << "int is a container? " << dm_is_container_v<int> << std::endl;
    std::cout << "int[5] is a container? " << dm_is_container_v<int[5]> << std::endl;

    // 2. 测试 dm_is_scoped_enum_v
    std::cout << "\n--- Testing dm_is_scoped_enum_v ---" << std::endl;
    std::cout << "CStyleEnum is a scoped enum? " << dm_is_scoped_enum_v<CStyleEnum> << std::endl;
    std::cout << "ScopedEnum is a scoped enum? " << dm_is_scoped_enum_v<ScopedEnum> << std::endl;
    std::cout << "int is a scoped enum? " << dm_is_scoped_enum_v<int> << std::endl;


    // 3. 测试 dm_is_string_like_v
    std::cout << "\n--- Testing dm_is_string_like_v ---" << std::endl;
    std::cout << "std::string is string-like? " << dm_is_string_like_v<std::string> << std::endl;
    std::cout << "const char* is string-like? " << dm_is_string_like_v<const char*> << std::endl;
    std::cout << "std::string_view is string-like? " << dm_is_string_like_v<std::string_view> << std::endl;
    std::cout << "int is string-like? " << dm_is_string_like_v<int> << std::endl;

    // 4. 测试 dm_is_pointer_like_v
    std::cout << "\n--- Testing dm_is_pointer_like_v ---" << std::endl;
    std::cout << "int* is pointer-like? " << dm_is_pointer_like_v<int*> << std::endl;
    std::cout << "std::unique_ptr<int> is pointer-like? " << dm_is_pointer_like_v<std::unique_ptr<int>> << std::endl;
    std::cout << "MySmartPtr<double> is pointer-like? " << dm_is_pointer_like_v<MySmartPtr<double>> << std::endl;
    std::cout << "std::vector<int> is pointer-like? " << dm_is_pointer_like_v<std::vector<int>> << std::endl;

    return 0;
}