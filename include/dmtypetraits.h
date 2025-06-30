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

/*
#### 模块架构

文件内的包含顺序遵循了从底层依赖到高层功能的建议架构，有助于理解整个库的组织方式。

* **1. 基础层 (Base Layer)**
    * `dmtypetraits_base.h`: 封装了标准库 `<type_traits>` 的基础功能。

* **2. 核心层 (Core Layer)**
    * `dmtypetraits_core.h`: 提供了基于 SFINAE（Substitution Failure Is Not An Error）的复合类型萃取。

* **3. 扩展层 (Extension Layer)**
    * `dmtypetraits_extensions.h`: 包含了更复杂的复合类型萃取和其它实用工具。

* **4. 功能模块 (Feature Modules)**
    * `dmtypetraits_logical.h`: 提供编译期的逻辑组合工具。
    * `dmtypetraits_function.h`: 用于萃取函数（包括普通函数、Lambda、成员函数等）的属性，如返回类型和参数类型。
    * `dmtypetraits_typelist.h`: 提供类型列表相关的元编程工具。
    * `dmtypetraits_md5.h`: 提供 MD5 哈希计算功能，主要用于序列化模块中的类型校验。
    * `dmtypetraits_reflection.h`: 提供无侵入式的编译期反射功能。
    * `dmtypetraits_reflection_intrusive.h`: 提供侵入式的编译期反射功能。
    * `dmtypetraits_pack.h`: 提供高性能的二进制序列化和反序列化功能。     
*/

#include "dmtypetraits_base.h"
#include "dmtypetraits_core.h"
#include "dmtypetraits_extensions.h"
#include "dmtypetraits_logical.h"
#include "dmtypetraits_function.h"
#include "dmtypetraits_typelist.h"
#include "dmtypetraits_md5.h"
#include "dmtypetraits_reflection.h"
#include "dmtypetraits_reflection_intrusive.h"
#include "dmtypetraits_pack.h"
#endif // __DMTYPETRAITS_H_INCLUDE__