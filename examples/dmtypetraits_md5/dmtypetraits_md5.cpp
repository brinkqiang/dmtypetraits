#include <iostream>
#include "dmtypetraits_platformhash.h"
#include "dmfix_win_utf8.h"

template<typename PlatformTag>
void print_platform_info() {
    using Traits = dm::platform::PlatformHashTraits<PlatformTag>;
    std::cout << "平台名称: " << Traits::name() << std::endl;
    std::cout << "是否Unix-like: " << (Traits::is_unix_like() ? "是" : "否") << std::endl;
    std::cout << "文件分隔符: " << Traits::file_separator() << std::endl;
}

int main() {
    std::cout << "=== 编译时平台检测 ===" << std::endl;

    // 使用编译时确定的平台类型
    using CurrentPlatform = dm::platform::CurrentPlatform;
    print_platform_info<CurrentPlatform>();

    std::cout << "\n=== 动态平台检测 ===" << std::endl;

    std::cout << "\n=== 基于哈希的平台类型 ===" << std::endl;

    // 使用哈希值确定的平台类型
    using HashPlatform = dm::platform::CurrentPlatform;
    using HashTraits = dm::platform::PlatformHashTraits<HashPlatform>;
    std::cout << "哈希平台名称: " << HashTraits::name() << std::endl;
    std::cout << "哈希值: 0x" << std::hex << HashPlatform::hash_value << std::endl;

    return 0;
}