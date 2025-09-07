#include <iostream>
#include "dmtypetraits_platformhash.h"
#include "dmfix_win_utf8.h"


int main() {
    std::cout << "=== 编译时平台检测 ===" << std::endl;

    dm::platform::PrintCurrentPlatformInfo();

    std::cout << "\n=== 基于静态函数的平台类型 ===" << std::endl;
    using HashPlatform = dm::platform::CurrentPlatform;
    using HashTraits = dm::platform::PlatformHashTraits<HashPlatform>;
    std::cout << "哈希平台名称: " << HashTraits::name() << std::endl;
    std::cout << "哈希值: 0x" << std::hex << HashPlatform::hash_value << std::endl;

    return 0;
}