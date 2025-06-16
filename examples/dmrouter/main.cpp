#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "dmfix_win.h"
#include "dmtypetraits.h"
#include "dmformat.h"

int main(int argc, char* argv[])
{
    std::shared_ptr<CDMRouterModule> router = dmrouterGetModule();
    if (router)
    {
        router->RegisterRouter("/getid", [](const int& a, const int& b) -> int {
            fmt::print("getid: {} {}\n", a, b);
            return a + b;
            });

        router->RegisterRouter("/helloworld", [](const std::string& name, const std::string& value) -> std::string {
            fmt::print("helloworld: {} {}\n", name, value);
            return name + value;
            });
        router->RegisterRouter("/helloworld_getid", [](const std::string& name, const std::string& value, int nID) -> std::string {
            fmt::print("helloworld_getid: {} {} {}\n", name, value, nID);
            return name + value + std::to_string(nID);
            });

        // 调用 CallRouter 时指定期望的返回类型作为模板参数
        std::string router_ret = router->CallRouter<std::string>("/helloworld", "hello", "world");
        int         router_ret2 = router->CallRouter<int>("/getid", 1, 2);
        std::string router_ret3 = router->CallRouter<std::string>("/helloworld_getid", "hello", "world", 3);

        // 现在可以直接使用类型化的结果，无需 std::any_cast
        fmt::print("{} + {} = {}\n", router_ret, router_ret2, router_ret3);

        try
        {
            // 这个调用如果类型不匹配（例如，期望 string，但实际参数导致内部转换失败或类型不符）
            // 或者期望的 ResultType 与实际handler返回的类型不符，CallRouter 内部的 any_cast 会抛出异常
            auto router_ret4 = router->CallRouter<std::string>("/helloworld_getid", "hello", "world", "world_string_instead_of_int");
            fmt::print("Router_ret4: {}\n", router_ret4); // 如果上面一行因为类型不匹配的参数导致内部any_cast失败，这里不会执行
        }
        catch (const std::exception& e)
        {
            fmt::print("error: {}\n", e.what());
            // 预期的错误可能是:
            // 1. 在 RegisterRouter 的 lambda 中：std::any_cast<ParametersTupleType>(args_from_caller) 失败（如果 ConvertArgsToString 的结果与 ParametersTupleType 不符）
            // 2. 在 CallRouter 中：std::any_cast<ResultType>(result_any) 失败（如果用户指定的 ResultType 与 handler 实际返回的类型不符）
            // 您的例子中，"/helloworld_getid" 期望第三个参数是 int，但您传入了 "world_string_instead_of_int"。
            // ConvertArgsToString 会将它变成 std::string。
            // RegisterRouter 中对应的 lambda 期望 std::tuple<std::string, std::string, int>。
            // 当它尝试 std::any_cast<std::tuple<..., int>>(包含 std::tuple<..., std::string> 的 std::any) 时会失败。
        }

        router->RegisterRouter("/user/profile",
            [](int id, const std::string& name, const std::vector<std::string>& devs) -> std::string { // 明确返回类型
                return fmt::format("/user/profile {}: {} [{}]\n", id, name, fmt::join(devs, ", "));
            });

        std::vector<std::string> devs = { "admin", "developer" };

        auto result = router->CallRouter<std::string>(
            "/user/profile",
            1001,
            "John Doe",
            devs
        );
        fmt::print("{}\n", result);
    }
    return 0;
}