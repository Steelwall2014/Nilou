# Nilou
一个自娱自乐的实时渲染引擎  
![](figures/nilou.png)
_她真的好美_
## Overview

原先是我的毕业设计，毕业设计中实现了pbr（部分）、GPU Driven的地形系统（还没搞VT）、大气渲染（还有bug）、FFT海面（这个倒是很完整）  
毕业项目的链接：https://pan.baidu.com/s/104S4944MNkbFUYGAdGSm5A?pwd=y4ws  
提取码：y4ws  
后来我觉得这个还有再继续做下去的价值，可以变成我试验各种图形学算法的游乐场，但是我发现这个项目当时的架构根本就没什么可扩展性，因此开始了长达4个月的重构之旅。  
重构选用了xmake作为构建工具。在重构中我尽量尝试模仿虚幻引擎的Actor和组件机制以及渲染体系，虽然依然很丑，但是已经有了那么一点样子。
构建系统使用xmake，编译器为clang-cl，只能运行在windows系统上。
## 运行
第一次运行之前，或者添加了被UCLASS()标记的类之后，需要手动运行一次HeaderTool（我暂时还没有找到能够自动判断是否需要运行的方法）。  
HeaderTool的编译：
```sh
xmake build -v HeaderTool
```
HeaderTool的运行：
```sh
xmake run HeaderTool
```
系统的编译和运行：
```sh
xmake build -v Nilou  
xmake run Nilou
```
## Features
- 一个比较完备的场景管理架构。参照虚幻引擎搭建了Actor和Component的架构，使得整个项目更有扩展性。
- 简单的侵入式反射，可以查询类的继承关系，可以根据名称创建类的对象。通过一个HeaderTool解析头文件，为打上UCLASS()标记的类生成反射信息，然后写入*.generated.cpp中，加入编译。
- 系统中实现了一个简单的虚拟文件系统，用来管理纹理、材质、网格模型等资源。纹理、材质、网格模型具有序列化方法，参考glb格式定义了一种文件格式（nasset）来保存这些资源。
- 对顶点着色器、片元着色器的抽象。顶点着色器和片元着色器都是MaterialShader，其本身是不完整的。顶点着色器还需要顶点工厂和材质，片元着色器还需要材质，才能构成一个完整的着色器，而顶点工厂和材质则需要实现一系列接口。此外也可以定义GlobalShader，一般用在计算着色器上。
- Shader permutation的支持，MaterialShader和GlobalShader都可以定义一个FPermutationDomain，多个FShaderPermutation*（如FShaderPermutationBool），也就是Domain中的不同维度。系统会自动编译shader所有的permutation，用户可以为每个维度指定值，然后选取对应的permutation。总体来说和UE中对应的功能类似。
- 分离的逻辑线程和渲染线程，两个线程使用一些同步手段，使得渲染线程不会落后于逻辑线程超过1帧。逻辑线程向渲染线程发送渲染指令的方式是使用ENQUEUE_RENDER_COMMAND宏，这个宏会向渲染线程的任务队列中插入任务，渲染线程在每一帧的开始会从任务队列中取出任务来执行。
- 对顶点工厂、材质、着色器使用glslang库进行解析，生成一些反射信息，帮助进行资源的绑定。整个过程类似于这样：在渲染端保存一个map，key为变量名称，value为binding point；逻辑端需要提供另一个map作为输入，key为变量名称，value为各种资源。渲染时渲染端根据自己的map，去到输入的map中名称对应的资源。
- 提供了委托模板类，输入系统，还有一些渲染流程中的回调（比如PreRenderDelegate），是建立在这个委托模板上的，Actor可以在自己的构造函数中为按键挂上回调函数从而实现按键的响应。
- 采用延迟渲染，支持PBR，支持GLTF格式模型，支持3DTiles格式模型的多线程加载，3DTiles瓦片的换入换出使用LRU算法。
- Cascaded Shadow Map，PCF软阴影。CSM默认使用8级，前4级每帧更新，后4级轮流更新。PCF是7×7的。CSM的分割比例暂时采用了UE的方法，也就是幂函数分割，不过效果不是很好，而且也还没做不同级别边界的blend
- 支持GPU Driven的地形，参考的是Far Cry 5的地形方案，使用计算着色器实现GPU四叉树划分和视锥剔除，地形高度图支持虚拟纹理（使用Sparse Texture实现）。虚拟纹理的换入换出策略使用LRU算法。
- 支持预计算的大气渲染。在此基础上实现基于物理的、预计算的水体渲染，包含多次散射，能够通过调整水体成分（如有机物、叶绿素浓度）改变水体颜色。
- 支持海面的渲染，使用计算着色器进行快速傅里叶变换，生成海面位移贴图和法线贴图，应用于海面着色。
- 支持双精度世界坐标，在渲染时世界坐标会变换到Relative to Eye的坐标。
## TODO
- 水下散射
- 全局光照
- 屏幕空间反射
- TAA
- ......