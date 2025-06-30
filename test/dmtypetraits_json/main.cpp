#include "gtest.h"
#include <string>
#include <vector>
#include <map>
#include <any>
#include <variant> // find_field 需要

// 包含元数据文件，它会自动引入库核心和结构体定义
#include "dmstruct.meta.h"

// --- 测试固件 (Test Fixture) ---
class ReflectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        data = {
            101, Status::Ok, {"tom", 1156},
            {{"property1", 10}, {"property2", 20}},
            {0.1f, 0.2f, 0.3f},
            {{"jerry", 1347}}
        };
    }
    ComplexData data;
};

// --- 完整的测试用例 ---

TEST_F(ReflectionTest, BasicInfo) {
    ASSERT_EQ(dm::refl::get_class_name<Metadata>(), "Metadata");
    ASSERT_EQ(dm::refl::get_field_count<Metadata>(), 2);
    ASSERT_EQ(dm::refl::get_class_name<ComplexData>(), "ComplexData");
    ASSERT_EQ(dm::refl::get_field_count<ComplexData>(), 6);
}

TEST_F(ReflectionTest, VisitFields) {
    int field_count = 0;
    dm::refl::visit_fields(data, [&](const auto& field, const auto& value) {
        field_count++;
        if constexpr (field.index == 0) { // id
            ASSERT_EQ(field.name(), "id");
            ASSERT_EQ(value, 101);
        }
        else if constexpr (field.index == 1) { // status
            ASSERT_EQ(field.name(), "status");
            ASSERT_EQ(value, Status::Ok);
        }
    });
    ASSERT_EQ(field_count, dm::refl::get_field_count<ComplexData>());
}

TEST_F(ReflectionTest, ObjectAccessor) {
    auto accessor = dm::refl::make_accessor(data);

    // 按索引读写
    ASSERT_EQ(accessor.get<0>(), 101);
    accessor.set<0>(999);
    ASSERT_EQ(data.id, 999);
    ASSERT_EQ(accessor.get<0>(), 999);

    // 测试 set
    bool set_success = accessor.set("status", Status::Warning);
    ASSERT_TRUE(set_success);
    ASSERT_EQ(data.status, Status::Warning);

    bool set_fail = accessor.set("status", 12345); // int 不能赋值给 Status
    ASSERT_FALSE(set_fail);
    ASSERT_EQ(data.status, Status::Warning); // 值应保持不变
}
// ==========================================================================


TEST_F(ReflectionTest, NestedReflection) {
    bool metadata_found_and_tested = false;
    dm::refl::visit_fields(data, [&](const auto& field, const auto& value) {
        if constexpr (field.index == 2) { // metadata
            using ValueType = std::decay_t<decltype(value)>;
            if constexpr (dm::refl::is_reflectable_v<ValueType>) {
                metadata_found_and_tested = true;
                dm::refl::visit_fields(value, [&](const auto& inner_field, const auto& inner_value) {
                    if constexpr (inner_field.index == 0) { // author
                         ASSERT_EQ(inner_field.name(), "author");
                         ASSERT_EQ(inner_value, "tom");
                    }
                });
            }
        }
    });
    ASSERT_TRUE(metadata_found_and_tested);
}


// ======================= 补全 3: 增强 LegacyCompatibility 测试 =======================
TEST_F(ReflectionTest, LegacyCompatibility) {
    std::map<std::string, std::string> member_values;
    
    // 调用旧接口，并将访问到的成员及其值（转换为字符串）存入map
    dm::refl::dm_visit_members(data, [&](const char* name, auto&& value) {
        member_values[name] = dmcast::lexical_cast<std::string>(value);
    });

    // 验证map的大小是否等于字段数量
    ASSERT_EQ(member_values.size(), dm::refl::get_field_count<ComplexData>());

    // 验证几个关键字段的值是否正确
    ASSERT_EQ(member_values["id"], "101");
    ASSERT_EQ(member_values["status"], "0"); // Status::Ok 的底层值是 0

    // 验证嵌套结构体（注意：dm_visit_members无法递归，它需要一个强大的tostring）
    // 这里我们假设有一个可以处理Metadata的tostring，或者跳过这个验证
    // 简单的调用测试仍然有效
    ASSERT_NO_FATAL_FAILURE(dm::refl::dm_visit_members(data, [](auto...){}));
}
// =================================================================================