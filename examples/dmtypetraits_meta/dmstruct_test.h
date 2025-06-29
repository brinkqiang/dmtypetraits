
#ifndef __DMSTRUCT_TEST_H__
#define __DMSTRUCT_TEST_H__

#include <string>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <array>
#include <cassert>
#include <system_error>

struct MyStruct {
// export_begin    
    int foo;
    std::string bar;
// export_end
};


// 枚举类型
enum class Status : uint8_t {
    Ok,
    Warning,
    Error
};

// 嵌套结构体
// 注意：所有需要序列化的自定义结构体都必须是聚合类型
// (没有用户自定义的构造函数、没有私有/保护的非静态成员、没有基类、没有虚函数)
struct Metadata {
// export_begin  
    std::string author;
    uint64_t timestamp;
// export_end
    // 为了断言验证，需要提供 operator==
    bool operator==(const Metadata& other) const {
        return author == other.author && timestamp == other.timestamp;
    }
};

// 主数据结构，包含多种复杂类型
struct ComplexData {
// export_begin  
    int id;
    Status status;
    Metadata metadata; // 嵌套聚合类型
    std::map<std::string, int> properties;
    std::vector<float> sensor_readings;
// export_end
    // 为了断言验证，需要提供 operator==
    bool operator==(const ComplexData& other) const {
        return id == other.id &&
            status == other.status &&
            metadata == other.metadata &&
            properties == other.properties &&
            sensor_readings == other.sensor_readings;
    }
};



#endif // __DMSTRUCT_TEST_H__
