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

#ifndef __DMROUTER_H_INCLUDE__
#define __DMROUTER_H_INCLUDE__

#include "dmos.h"
#include "dmtypetraits_function.h"

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>


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
        std::any anyArgs = tupleArgs;

        std::any result_any = generic_handler(anyArgs);

        if constexpr (std::is_void_v<ResultType>) {
            return;
        }
        else {
            if (!result_any.has_value()) {
                throw std::runtime_error("Handler for event '" + eventName + "' produced an empty std::any, but a non-void ResultType was expected.");
            }
            try {
                return std::any_cast<ResultType>(result_any);
            }
            catch (const std::bad_any_cast& e) {
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

    // RegisterRouter
    template <typename Func>
    void RegisterRouter(const EventKey& eventName, Func&& original_handler) {
        using ParametersTupleType = dm_function_parameters_t<Func>;
        using OriginalReturnType = dm_function_return_t<Func>;

        m_eventHandlers[eventName] =
            [handler_capture = std::forward<Func>(original_handler)](std::any args_from_caller) -> std::any {

            ParametersTupleType actual_args_tuple = std::any_cast<ParametersTupleType>(args_from_caller);

            if constexpr (std::is_void_v<OriginalReturnType>) {
                std::apply(handler_capture, std::move(actual_args_tuple));
                return std::any{}; // 对于 void 返回，返回一个空的 std::any
            }
            else {
                return std::make_any<OriginalReturnType>(std::apply(handler_capture, std::move(actual_args_tuple)));
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

#endif // __DMROUTER_H_INCLUDE__