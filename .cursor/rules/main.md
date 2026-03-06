本项目是一个参照虚幻引擎架构实现的渲染引擎，在设计理念和部分代码实现上与虚幻引擎高度相似，但也存在若干差异。

> **注意：** 引擎目前处于高速迭代阶段，文档存在一定的滞后性。模块的整体设计思路已基本确定，不会有重大调整。若在执行任务过程中发现文档描述与实际代码存在出入，请在终端输出警告并给出更新建议。

# 构建

构建顺序如下：

**第一步：** 构建 `NilouHeaderTool`。该工具负责解析 C++ 头文件并自动生成反射相关代码。
```
xmake build NilouHeaderTool
```

**第二步：** 构建 `NilouShaderTool`。该工具负责解析着色器代码并自动生成着色器结构体对应的 C++ 绑定。
```
xmake build NilouShaderTool
```

**第三步：** 构建主目标 `NilouEditor` 或 `NilouGame`。二者目前除模块链接方式不同外基本一致，未来 `NilouEditor` 将引入编辑器 UI 等专属功能。
```
xmake build NilouEditor
```

# 模块

引擎的运行时代码位于 `Engine/Source/Runtime`，按功能划分为 `Core`、`Engine` 等若干模块。每个模块的 `Public` 目录在构建时会作为公共 include 路径，自动对其依赖者可见。

> 当前模块系统较为基础，主要完成了依赖关系的梳理。后续计划支持模块动态加载/卸载及热重载等高级功能。

## module_rules

引擎模块在构建脚本中通过 `module_rules` 函数定义，主要完成以下工作：

- **生成导入导出宏头文件：** 在编译前生成 `Definitions.<MODULE>.h`，并通过编译器的 `/FI` 参数强制包含。采用此方式而非 `add_defines` 的原因是后者会在每次构建时触发全量重新编译。
- **调用代码生成工具：** 依次执行 `NilouHeaderTool` 和 `NilouShaderTool`。

## target_rules

引擎支持 `Editor` 和 `Game` 两种构建目标，在构建脚本中通过 `target_rules` 函数定义：

- **Editor 目标：** 模块以动态库（DLL）形式编译。
- **Game 目标：** 模块以静态库形式编译。

## 代码生成

编译前，`NilouHeaderTool` 会扫描 C++ 头文件，对标注了 `NCLASS`、`NSTRUCT`、`NPROPERTY` 等宏的类型自动生成反射与序列化代码。

# 着色器

项目使用 [Slang](https://shader-slang.com/) 着色器语言。着色器源文件存放于 `Engine\Shaders` 目录下。其中，只有 `Engine\Shaders\Public` 目录下的着色器是全局可见的。

着色器编写须遵循以下规范：

- **模块声明：** 每个着色器文件必须以 Slang 模块声明语句开头，即 `module` 或 `implementing`。
- **参数封装：** 所有着色器参数必须封装在 `ParameterBlock<T>` 中。一个 `ParameterBlock<T>` 在底层通常对应一个 Vulkan Descriptor Set（或 D3D12 Descriptor Heap）。
- **材质着色器：** 材质着色器中名为 `MaterialParameters` 的参数是一个特殊参数，材质系统会将它的类型的元数据存储在对应的材质中，这样可以用字段名设置材质参数。

## 代码生成

编译前，`NilouShaderTool` 会扫描着色器代码并生成对应的 C++ 结构体绑定，规则如下：

- 只有以模块声明语句（`module` 或 `implementing`）开头的着色器文件才会被解析。
- 只有声明在 `ParameterBlock<T>` 内部的结构体类型（包括其成员的结构体类型）才会生成 C++ 绑定。
- 嵌套的 `ParameterBlock<T>` 不会被递归解析。