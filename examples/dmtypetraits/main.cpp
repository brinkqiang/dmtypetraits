#include "dmtypetraits.h" // <--- 已更新
#include <iostream>
#include <string>

// DmComponentInfo 结构体定义保持不变...
struct DmComponentInfo {
    int member_exists;
    unsigned int is_bitfield : 1;
    static double static_member;
    static const int static_const_member = 100;
    mutable std::string mutable_member;
    const int const_member;
    int& ref_member;
    int* pointer_member;
    const std::string& const_ref_member;
    char array_member[16];
    void FunctionMember(int) {}
    std::string operator()() const { return "callable"; }

    DmComponentInfo(int& ref)
        : const_member(0), ref_member(ref), const_ref_member("hello") {}
};
double DmComponentInfo::static_member = 3.14;


int main() {
    // 主函数内的所有测试代码保持不变，它们现在调用的是新头文件中的宏
    std::cout << std::boolalpha;

    std::cout << "--- Member Existence ---" << std::endl;
    std::cout << "Has 'member_exists': " << DM_MEMBER_HAS(DmComponentInfo, member_exists) << std::endl;
    std::cout << "Has 'non_existent_member': " << DM_MEMBER_HAS(DmComponentInfo, non_existent_member) << std::endl;
    std::cout << std::endl;

    std::cout << "--- Bit-field Check ---" << std::endl;
    std::cout << "Is 'is_bitfield' a bit-field?   " << DM_MEMBER_IS_BITFIELD(DmComponentInfo, is_bitfield) << std::endl;
    std::cout << std::endl;
    
    // ... 其他测试代码
    
    return 0;
}