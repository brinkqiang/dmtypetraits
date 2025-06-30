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

#ifndef __DMTYPETRAITS_H_INCLUDE__
#define __DMTYPETRAITS_H_INCLUDE__

/**
 * @file dmtypetraits.h
 * @brief 模板元编程库 (Template Metaprogramming Library) 的统一入口。
 *
 * 包含了所有核心的类型萃取、类型列表和元编程工具。
 * 使用者只需包含此头文件即可使用库的全部功能。
 */

// 包含顺序建议从底层依赖到高层功能，有助于理解架构。

// 1. 基础层：对标准库 <type_traits> 的封装
#include "dmtypetraits_base.h"

// 2. 核心层：基于 SFINAE 的复合类型萃取
#include "dmtypetraits_core.h"

// 3. 扩展层：更复杂的复合萃取和实用工具
#include "dmtypetraits_extensions.h"

// 4. 功能模块：
#include "dmtypetraits_logical.h"       // 逻辑组合工具
#include "dmtypetraits_function.h"      // 函数萃取
#include "dmtypetraits_typelist.h"      // 类型列表工具

#include "dmtypetraits_md5.h"           // MD5
#include "dmtypetraits_reflection.h"    // 反射

#include "dmtypetraits_pack.h"
#include "dmtypetraits_reflection_intrusive.h"
#endif // __DMTYPETRAITS_H_INCLUDE__