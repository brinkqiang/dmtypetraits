#include <iostream>
#include "dmtypetraits_platformhash.h"
#include "dmfix_win_utf8.h"


int main() {

    using HashPlatform = dm::platform::CurrentPlatform;
    using HashTraits = dm::platform::PlatformHashTraits<HashPlatform>;

    std::cout << "平台名称: " << HashTraits::name() << std::endl;
    std::cout << "是否Unix-like: " << (HashTraits::is_unix_like() ? "是" : "否") << std::endl;
    std::cout << "文件分隔符: " << HashTraits::file_separator() << std::endl;
    std::cout << "平台哈希值: 0x" << std::hex << HashPlatform::hash_value << std::endl;
    return 0;
}