

#include "dmtypetraits.h"
#include <iostream>
#include <string>
#include <vector>

// 定义一个聚合类型
struct Player {
    int id;
    std::string name;
    double score;
};

// 不是聚合类型（有自定义构造函数）
struct NonAggregate {
    int x;
    NonAggregate(int val) : x(val) {}
};

int main() {
    // 1. 获取成员数量
    std::cout << "Player has " << dm_member_count_v<Player> << " members." << std::endl;

    // 下面这行会触发编译错误，因为 NonAggregate 不是聚合类型
    // std::cout << "NonAggregate has " << dm_member_count_v<NonAggregate> << " members." << std::endl;

    // 2. 遍历成员
    Player player{ 101, "brink", 99.5 };

    std::cout << "\nVisiting members of player:" << std::endl;
    dm_visit_members(player, [](const auto&... members) {
        // C++17 折叠表达式，用于打印所有成员
        auto print_one = [](const auto& member) {
            std::cout << "  - Member value: " << member << " (type: " << typeid(member).name() << ")" << std::endl;
            };
        (print_one(members), ...);
        });

    // 3. 修改成员
    std::cout << "\nOriginal score: " << player.score << std::endl;
    dm_visit_members(player, [](int& id, std::string& name, double& score) {
        // visitor 的参数是引用，可以直接修改
        score = 100.0;
        });
    std::cout << "Modified score: " << player.score << std::endl;

    auto tuple_player = dm_struct_to_tuple(player);

    auto new_player = dm_tuple_to_struct<Player>(tuple_player);

    return 0;
}
