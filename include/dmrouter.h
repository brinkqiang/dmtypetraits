
// Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "dmos.h"

#include <memory>
#include <atomic>
#include <unordered_map>
#include <string>
#include <functional>
#include <tuple>
#include <type_traits>
#include <any>
#include <stdexcept>
#include <mutex>

#include "dmtypetraits_function.h"

class CDMRouterModule : public std::enable_shared_from_this<CDMRouterModule> {
public:
    CDMRouterModule() {}
    virtual ~CDMRouterModule() {}

    using EventKey = std::string;

    template <typename... Args>
    static auto ConvertArgsToString(Args&&... args) {
        return std::tuple<std::conditional_t<std::is_same_v<std::decay_t<Args>, const char*>, std::string, std::decay_t<Args>>...>(
            std::forward<Args>(args)...);
    }

    // CallRouter
    template <typename ResultType, typename... Args>
    ResultType CallRouter(const EventKey& eventName, Args&&... args) {
        auto it = m_eventHandlers.find(eventName);
        if (it == m_eventHandlers.end()) {
            throw std::runtime_error("Event not found: " + eventName);
        }

        auto& generic_handler = it->second; // std::function<std::any(std::any)>

        auto tupleArgs = ConvertArgsToString(std::forward<Args>(args)...);
        std::any anyArgs = tupleArgs; // 或者 std::make_any<decltype(tupleArgs)>(std::move(tupleArgs));

        std::any result_any = generic_handler(anyArgs);

        if constexpr (std::is_void_v<ResultType>) {
            // 如果期望的结果是 void，我们不需要从 result_any 中提取值。
            // 可以选择检查 result_any.has_value() 是否为 false，如果注册时保证了 void 返回对应 empty std::any
            return;
        } else {
            // 如果期望非 void 结果但 result_any 为空 (例如，原处理函数返回 void)
            if (!result_any.has_value()) {
                 // 检查 ResultType 是否可以从一个空的 std::any (通过默认构造) “转换”得到是不直接的。
                 // 更好的做法是，如果原始处理函数返回 void，它应该返回 std::any{}。
                 // 如果 result_any 是空的，但 ResultType 不是 void，这通常是一个类型不匹配的错误。
                throw std::runtime_error("Handler for event '" + eventName + "' produced an empty std::any, but a non-void ResultType was expected.");
            }
            try {
                return std::any_cast<ResultType>(result_any);
            } catch (const std::bad_any_cast& e) {
                // 可以提供更详细的错误信息
                std::string error_msg = "Failed to cast result for event '";
                error_msg += eventName;
                error_msg += "'. Expected type '";
                error_msg += typeid(ResultType).name(); 
                error_msg += "' but actual type in std::any was different or incompatible. Original error: ";
                error_msg += e.what();
                throw std::runtime_error(error_msg);
            }
        }
    }

    // RegisterRouter (确保void返回时，generic handler返回空的std::any)
    template <typename Func>
    void RegisterRouter(const EventKey& eventName, Func&& original_handler) {
        using ParametersTupleType = dm_function_parameters_t<Func>; 
        using OriginalReturnType = dm_function_return_t<Func>; 

        m_eventHandlers[eventName] = 
            [handler_capture = std::forward<Func>(original_handler)](std::any args_from_caller) -> std::any {
            
            // 从 std::any 中提取参数元组
            // 注意：这里的 ParametersTupleType 必须与 CallRouter 中 ConvertArgsToString 之后
            // 包装进 std::any 的元组类型完全匹配，否则 any_cast 会失败。
            // 如果 ConvertArgsToString 改变了类型 (例如 const char* -> std::string),
            // 那么 ParametersTupleType 需要反映这种转换后的类型，或者转换步骤需要调整。
            // 为简单起见，此处假设类型是匹配的。
            ParametersTupleType actual_args_tuple = std::any_cast<ParametersTupleType>(args_from_caller);

            if constexpr (std::is_void_v<OriginalReturnType>) {
                std::apply(handler_capture, std::move(actual_args_tuple));
                return std::any{}; // 对于 void 返回，返回一个空的 std::any
            } else {
                return std::apply(handler_capture, std::move(actual_args_tuple));
            }
        };
    }

private:
    using GenericHandler = std::function<std::any(std::any)>;
    std::unordered_map<EventKey, GenericHandler> m_eventHandlers;
};

inline std::shared_ptr<CDMRouterModule> dmrouterGetModule()
{
    static std::once_flag m_oOnce;
    static std::shared_ptr<CDMRouterModule> module;
    std::call_once(m_oOnce, [&]()
        {
            module = std::make_shared<CDMRouterModule>();
        });
    return module;
}