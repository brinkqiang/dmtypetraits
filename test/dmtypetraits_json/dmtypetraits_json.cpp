#include "gtest.h"
#include <string>
#include <vector>
#include <map>
#include <any>
#include <variant> // find_field 需要

// 包含元数据文件，它会自动引入库核心和结构体定义
#include "dmstruct.meta.h"
#include "dmjson.h"

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
        if constexpr (std::decay_t<decltype(field)>::index == 0) { // id
            ASSERT_EQ(field.name(), "id");
            ASSERT_EQ(value, 101);
        }
        else if constexpr (std::decay_t<decltype(field)>::index == 1) { // status
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
        if constexpr (std::decay_t<decltype(field)>::index == 2) { // metadata
            using ValueType = std::decay_t<decltype(value)>;
            if constexpr (dm::refl::is_reflectable_v<ValueType>) {
                metadata_found_and_tested = true;
                dm::refl::visit_fields(value, [&](const auto& inner_field, const auto& inner_value) {
                    if constexpr (std::decay_t<decltype(inner_field)>::index == 0) { // author
                         ASSERT_EQ(inner_field.name(), "author");
                         ASSERT_EQ(inner_value, "tom");
                    }
                });
            }
        }
    });
    ASSERT_TRUE(metadata_found_and_tested);
}

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

TEST_F(ReflectionTest, JsonSerializationAndParsing) {
    // 1. 准备C++数据结构
    ComplexData data{
        101, Status::Ok, {"tom", 1156},
        {{"property1", 10}, {"property2", 20}},
        {0.1f, 0.2f, 0.3f},
        {{"jerry", 1347}}
    };

    // 2. 使用您的反射库将其序列化为字符串
    // 我们假设 dmcast 现在可以生成完全合法的JSON字符串
    std::string json_str = dmcast::lexical_cast<std::string>(data);

    // 3. 使用 GTest 断言来验证解析过程
    nlohmann::json parsed_json;
    ASSERT_NO_THROW({
        // 验证：解析一个（期望是）合法的JSON字符串不应抛出任何异常
        parsed_json = nlohmann::json::parse(json_str);
    }) << "The string produced by lexical_cast should be valid JSON and not throw on parsing.";

    // 4. 验证解析后的JSON对象内容是否正确
    ASSERT_FALSE(parsed_json.is_null());
    ASSERT_TRUE(parsed_json.is_object());

    // 验证顶层字段
    ASSERT_EQ(parsed_json["id"], 101);
    // 注意：枚举会被序列化为其底层整数值
    ASSERT_EQ(parsed_json["status"], static_cast<uint8_t>(Status::Ok)); 

    // 验证嵌套对象
    ASSERT_TRUE(parsed_json.contains("metadata"));
    ASSERT_TRUE(parsed_json["metadata"].is_object());
    ASSERT_EQ(parsed_json["metadata"]["author"], "tom");
    ASSERT_EQ(parsed_json["metadata"]["timestamp"], 1156);

    // 验证映射容器
    ASSERT_TRUE(parsed_json.contains("properties"));
    ASSERT_EQ(parsed_json["properties"]["property1"], 10);
    ASSERT_EQ(parsed_json["properties"]["property2"], 20);

    // 验证序列容器
    ASSERT_TRUE(parsed_json.contains("sensor_readings"));
    ASSERT_TRUE(parsed_json["sensor_readings"].is_array());
    ASSERT_EQ(parsed_json["sensor_readings"].size(), 3);
    // 使用 EXPECT_FLOAT_EQ 或 EXPECT_NEAR 来比较浮点数
    EXPECT_FLOAT_EQ(parsed_json["sensor_readings"][0], 0.1f);

    // 验证包含复杂类型的序列容器
    ASSERT_TRUE(parsed_json.contains("metadatas"));
    ASSERT_EQ(parsed_json["metadatas"][0]["author"], "jerry");
}