# Nilou
一个自娱自乐的实时渲染引擎  
![](figures/nilou.png)
_她真的好美_
## Overview

原先是我的毕业设计，毕业设计中实现了pbr（部分）、GPU Driven的地形系统（还没搞RVT）、大气渲染（还有bug）、FFT海面（这个倒是很完整）  
毕业项目的链接：https://pan.baidu.com/s/104S4944MNkbFUYGAdGSm5A?pwd=y4ws  
提取码：y4ws  
后来我觉得这个还有再继续做下去的价值，可以变成我试验各种图形学算法的游乐场，但是我发现这个项目当时的架构根本就没什么可扩展性，因此开始了长达4个月的重构之旅。  
重构选用了xmake作为构建工具。在重构中我尽量尝试模仿虚幻引擎的Actor和组件机制以及渲染体系，虽然依然很丑，但是已经有了那么一点样子。在妮露池的最后一天终于差不多重构完了。
## 运行
```sh
xmake build -v Nilou  
xmake run Nilou
```
## 相较于毕设多了哪些
- 在类上打上UCLASS()的标记会自动生成类的信息（比如继承关系）
- vertex factory、shader和material的分离
- 像样的actor和component机制
- 像样的渲染资源管理与渲染流程控制
## 相较于毕设还少了哪些
- FFT海面
- 地形系统
## TODO
- 完整的地形系统、pbr
- 屏幕空间反射
- TAA
- ......