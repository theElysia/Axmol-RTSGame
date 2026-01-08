# Axmol RTS Game

#### author: Zevick

> A Real-Time Strategy game built with C++ and Axmol Engine, serving as a practical project for implementing and mastering software design patterns.

> 孤岂欲开发游戏邪？但当以此为基，实践**设计模式**耳。

### Related Tools

+ 游戏开发
  + 引擎：[Axmol Engine v2.10.0](https://github.com/axmolengine/axmol)
  + 教程：自行查阅仓库，或者入门可以先参考[cocos2d-x manual](https://docs.cocos.com/cocos2d-x/v4/manual/zh/)了解游戏开发基本概念
  + Windows Visual Studio 2026
  + 编程语言：C++23 (应该是至少用到了C++17的特性)
  + 补充：与Cocos2d-x引擎相似，你也可以选择使用该引擎开发。
+ 角色动画制作`.plist`
  + 工具：[TexturePacker](https://www.codeandweb.com/texturepacker)
  + 教程：[https://www.codeandweb.com/texturepacker/tutorials/animations-and-spritesheets-in-axmol-engine](https://www.codeandweb.com/texturepacker/tutorials/animations-and-spritesheets-in-axmol-engine)
+ 地图制作`.tmx`
  + 工具：[Tiled](https://www.mapeditor.org/)
  + 教程：[https://www.bilibili.com/video/BV1ubjZzqEPN](https://www.bilibili.com/video/BV1ubjZzqEPN)
+ 主要美术素材来源
  + [https://www.bilibili.com/video/BV1idNJz5Exu](https://www.bilibili.com/video/BV1idNJz5Exu)
  + [https://www.bilibili.com/video/BV14T4y1Z7np](https://www.bilibili.com/video/BV14T4y1Z7np)
  

---

### setup

请提前已安装Axmol与Visual Studio
创建项目后，确保可以用Visual Studio打开并运行（打开`.sln`或`.slnx`文件，`F5`运行）
注意下载Visual Studio时把游戏开发(UE/Cocos)的那一项选上

```bash
axmol new -p dev.axmol.mygame -d project/mygame -l cpp mygame
cd project/mygame
mkdir build
cd build
cmake ..
```

在成功运行HelloWorld示例后，把本仓库的Source与Content替换，重新cmake后即可运行
```bash
cd build
cmake ..
```

---

### 游戏基本内容

![游戏画面](https://github.com/theElysia/Axmol-RTSGame/blob/main/pics/GameScene.jpg)

点击右侧`Button`消耗一定点数部署角色。按住`ctrl`后滑动鼠标选中角色（或`ctrl+A`以全选），松开`ctrl`后鼠标点击地块引导角色移动。击破敌方大本营后游戏胜利。

本游戏用了标准的配置驱动模式，使用excel配置角色动画以及数据，如下

![角色配置信息](https://github.com/theElysia/Axmol-RTSGame/blob/main/pics/character_stats.png)

---

### 使用的设计模式

+ 重要
  + 单例模式
  + 建造者模式
  + 观察者模式
  + 策略模式
  + 工厂模式
  + 蝇量/享元模式
  + 状态模式
  + 责任链模式
  + 命令模式
  + 原型模式
+ 次要
  + 装饰器模式
  + 代理模式
  + 组合模式
  + 空对象模式
  + 模板方法模式
  + 迭代器模式
+ 未实现的缺憾
  + 备忘录模式（未实现存档）
  

### 代码简易文档


[代码文档](https://github.com/theElysia/Axmol-RTSGame/blob/main/code_explanation.md)
