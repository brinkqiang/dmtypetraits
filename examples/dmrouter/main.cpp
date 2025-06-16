#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "dmfix_win.h"
#include "dmrouter.h"
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

        // ���� CallRouter ʱָ�������ķ���������Ϊģ�����
        std::string router_ret = router->CallRouter<std::string>("/helloworld", "hello", "world");
        int         router_ret2 = router->CallRouter<int>("/getid", 1, 2);
        std::string router_ret3 = router->CallRouter<std::string>("/helloworld_getid", "hello", "world", 3);

        // ���ڿ���ֱ��ʹ�����ͻ��Ľ�������� std::any_cast
        fmt::print("{} + {} = {}\n", router_ret, router_ret2, router_ret3);

        try
        {
            // �������������Ͳ�ƥ�䣨���磬���� string����ʵ�ʲ��������ڲ�ת��ʧ�ܻ����Ͳ�����
            // ���������� ResultType ��ʵ��handler���ص����Ͳ�����CallRouter �ڲ��� any_cast ���׳��쳣
            auto router_ret4 = router->CallRouter<std::string>("/helloworld_getid", "hello", "world", "world_string_instead_of_int");
            fmt::print("Router_ret4: {}\n", router_ret4); // �������һ����Ϊ���Ͳ�ƥ��Ĳ��������ڲ�any_castʧ�ܣ����ﲻ��ִ��
        }
        catch (const std::exception& e)
        {
            fmt::print("error: {}\n", e.what());
            // Ԥ�ڵĴ��������:
            // 1. �� RegisterRouter �� lambda �У�std::any_cast<ParametersTupleType>(args_from_caller) ʧ�ܣ���� ConvertArgsToString �Ľ���� ParametersTupleType ������
            // 2. �� CallRouter �У�std::any_cast<ResultType>(result_any) ʧ�ܣ�����û�ָ���� ResultType �� handler ʵ�ʷ��ص����Ͳ�����
            // ���������У�"/helloworld_getid" ���������������� int������������ "world_string_instead_of_int"��
            // ConvertArgsToString �Ὣ����� std::string��
            // RegisterRouter �ж�Ӧ�� lambda ���� std::tuple<std::string, std::string, int>��
            // �������� std::any_cast<std::tuple<..., int>>(���� std::tuple<..., std::string> �� std::any) ʱ��ʧ�ܡ�
        }

        router->RegisterRouter("/user/profile",
            [](int id, const std::string& name, const std::vector<std::string>& devs) -> std::string { // ��ȷ��������
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