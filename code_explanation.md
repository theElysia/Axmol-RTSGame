# Axmol RTS Game Code Explanation

#### author: Zevick

### 项目结构

![代码结构](https://github.com/theElysia/Axmol-RTSGame/blob/main/pics/CodeStructure.png)

+ GameMap
  + GameMapManager，管理场景地图
  + PathFinderStrategy，寻路算法策略模式，已实现适合RTS的流场算法
+ GameMessage
  + GameCommand，GameObject接受的命令
  + RTSCommandPool，管理内存池并缓冲命令
  + GameMessage，GameObject发出的消息
  + GameMessageHandler，责任链模式处理消息
+ GameObject
  + GameObject为抽象游戏对象类，子类具体对象包含GameBuilding与GameRole
  + GameBuilding，简单设计
  + GameRole，使用状态机，比较完善的设计
  + GameObjectFactory，简单工厂
  + AttributeHandler，具体游戏业务逻辑，用于组合设计
  + GameObjectManager，享元模式，统一管理游戏对象
  + GameResourceHandler，用于预加载游戏资源
  + GameObjectTemplate，原型模式
+ Scenes
  + 测试场景，用于测试各项功能，实际不被使用
+ AppDelegate
  + 程序入口，基本资源设置
  + MenuScene
    + 基本菜单
  + GameScene
    + 游戏场景
    + GameUI
    + GameWorld

---

### 设计模式讲解

部分次要的就不讲了，只讲一些重要的。

##### 工厂模式(Factory)

最明显的就是手动写的GameObjectFactory，用来自动选择多态子类。

实际更多用的是静态工厂方法与二段构造方法，类似如下代码。

```cpp
class A{
public:
    static A* create();
    virtual bool init();
};
```

静态工厂就不多说了。二段构造主要是将内存分配与初始化分离。我们知道c++构造函数没有返回值，用这种方法可以避免内存申请失败错误。使用时通常如下：

```cpp
auto game_role = ax::utils::createInstance<GameRole>(&GameRole::init, args...);

template <typename T, typename F, typename... Ts>
inline T* createInstance(F&& finit, Ts&&... args)
{
    T* pRet = new T();
    if (std::mem_fn(finit)(pRet, std::forward<Ts>(args)...))
        pRet->autorelease();
    else
    {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}
```

##### 建造者模式(Builder)

这与游戏开发基本概念中的`导演(Director)`相关，参考[cocos2d-x manual](https://docs.cocos.com/cocos2d-x/v4/manual/zh/)。
游戏由导演控制`场景(Scene)`替换和转换，本例中场景就是`MenuScene`和`GameScene`。这里场景的生成就使用了建造者模式，当然具体类生成是用静态工厂方法。

当然不同层级通过GameObjectManager，再经由GameObjectFactory生成游戏对象也体现了建造者模式，只是没那么明显。

##### 单例模式(Singleton)

很明显`导演(Director)`或者`GameResourceHandler`等是需要单例实现。导演的单例由引擎实现，自己写的单例参考`BaseTemplates.h`中的宏实现。

```cpp
#define DECLARE_SINGLETON(ClassName)                  \
private:                                              \
    ClassName()                            = default; \
    ~ClassName()                           = default; \
    ClassName(const ClassName&)            = delete;  \
    ClassName& operator=(const ClassName&) = delete;  \
    ClassName(ClassName&&)                 = delete;  \
    ClassName& operator=(ClassName&&)      = delete;  \
                                                      \
public:                                               \
    static ClassName& instance()                      \
    {                                                 \
        static ClassName inst;                        \
        return inst;                                  \
    }                                                 \
    static ClassName* getInstance()                   \
    {                                                 \
        return &instance();                           \
    }
```

当然在开发过程中发现单例其实很容易滥用（这就相当于一个全局变量），其生命周期以及可见性会产生问题，所以还是尽量避免使用。

例如通过精心设计，`GameObjectManager`与`GameMapManager`最终没有采用单例模式，这样可以明确其可见性，便于维护。


##### 观察者模式(Observer)

游戏需要响应玩家的输入（键盘，鼠标等）。通过向这些监视器注册观察者（回调函数），就可以实现各种功能。比如`GameScene`中的代码：

```cpp
keyboard_listener_                = EventListenerKeyboard::create();
keyboard_listener_->onKeyReleased = AX_CALLBACK_2(GameScene::onKeyReleased, this);
keyboard_listener_->onKeyPressed  = AX_CALLBACK_2(GameScene::onKeyPressed, this);
_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener_, this);
```


##### 策略模式(Strategy)

设置一个统一的接口，当有需求时方便扩展与替换算法。比如寻路算法抽象基类接口设置如下（实际上寻路应该用引擎自带的类似工具navmesh）：

```cpp
class PathFinderStrategy
{
public:
    virtual ~PathFinderStrategy() = default;

    virtual bool init(const std::vector<bool>& grid, int x, int y) = 0;

    virtual void setDest(const Point& dst) = 0;

    virtual std::optional<std::vector<Point>> findPath(const Point& src) = 0;
};
```

##### 蝇量/享元模式(Flyweight)

RTS游戏中有大量游戏对象，而且要实现选中移动等功能（对玩家输入做出响应）。若每个对象分别作为一个观察者注册到事件列表中，那么显然性能问题严重。这时候就需要有统一管理的`GameObjectManager`，同时一些选中状态/大本营管理等业务逻辑也可以由其完成。

##### 状态模式(State)

随着游戏进行，一个游戏对象会有多种状态，比如战斗与空闲，行为上说就是不同状态下会有不同的基础逻辑与响应动作，那么这样就需要状态模式来介入，提供可扩展性与可维护性。

当然，为了进一步提升性能，避免过多子类，我使用了静态状态，`Object`和`Context`相当于每次作为参数提供。

```cpp
class GameObject : public ax::Sprite
{
public:
    virtual void handleCommand(GameCommand* cmd) = 0;

    void sendMessage(GameMessage* msg);
};

class GameRole : public GameObject
{
public:
    void setState(GameRoleState* newState) { current_state_ = newState; }
    GameRoleState* getCurrentState() { return current_state_; }

    void update(float delta) override { current_state_->update(this); }
    void handleCommand(GameCommand* cmd) override { current_state_->handleCommand(this, cmd); }
}；
```

```cpp
class GameRoleState
{
public:
    virtual ~GameRoleState() = default;

    virtual void update(GameRole* role);

    virtual void handleCommand(GameRole* role, GameCommand* cmd);
};
```

##### 装饰模式(Decorator)

动作`Action`中使用的`Sequence`与`Spawn`修饰符，可以将原本的基础动作修饰附带上其他动作，并保持接口不变。

##### 责任链模式(Chain of Responsibility)

`GameMessage`由`GameObject`产生并向上传播，接受这样的消息并处理很自然想到用责任链模式。就是设计多个handler，逐次调用handler来处理一个消息，每个handler可以只处理部分消息，未处理的沿着链往后传递。

```cpp
class GameMessageHandler
{
public:
    enum class ResultType
    {
        CONSUMED,
        UNHANDLED,
        REJECTED,
    };

public:
    virtual ~GameMessageHandler() = default;

    void setNext(std::shared_ptr<GameMessageHandler> next) { next_ = next; }

    ResultType handle(GameMessage* msg);

protected:
    virtual bool canHandle(GameMessage* msg) const = 0;
    virtual ResultType process(GameMessage* msg)   = 0;

private:
    std::shared_ptr<GameMessageHandler> next_;
};


ResultType GameMessageHandler::handle(GameMessage* msg)
{
    GameMessageHandler* current = this;

    while (current)
    {
        if (current->canHandle(msg))
        {
            return current->process(msg);
        }
        current = current->next_.get();
    }

    return ResultType::UNHANDLED;
}
```

##### 命令模式(Command)

显然`GameCommand`描述的就是这种模式，当然这也需要`GameObject`，`RTSCommandPool`的配合。为了适应RTS高并发命令的场景，`RTSCommandPool`的设计还是较为复杂的，目前实现了
+ 内存池管理，所有命令均存放于固定内存池中
+ 双缓冲结构，缓冲相邻帧的命令
+ 批处理模式，批量处理上一帧的命令，优化手段有并行和分组（提高缓存命中）。

```cpp
class RTSCommandPool
{
public:
    RTSCommandPool() = default;

    RTSCommandPool(const RTSCommandPool&)            = delete;
    RTSCommandPool& operator=(const RTSCommandPool&) = delete;

    // 添加命令到当前帧缓冲区，并自动分组
    template <typename T, typename... Args>
    void addCommand(Args&&... args);

    void processPreviousFrame(GameObjectManager* manager);
    void swapBuffers();
    void clearAll();
};


template <typename T, typename... Args>
void RTSCommandPool::addCommand(Args&&... args)
{
    static_assert(std::is_base_of_v<GameCommand, T>, "T must be derived from GameCommand");

    FrameBuffer& currentBuffer = frameBuffers[currentBufferIndex];

    auto cmd = this->createCommand<T>(currentBuffer, std::forward<Args>(args)...);

    GameCommand::CommandType cmdType = cmd->getType();
    auto& commandGroup               = currentBuffer.getCommandGroup(cmdType);

    commandGroup.push_back(std::move(cmd));
    currentBuffer.totalCommands++;

    if (currentBuffer.totalCommands >= MAX_COMMANDS_PER_FRAME)
    {
        handleCommandOverflow();
    }
}
```

##### 原型模式(Prototype)

游戏会有大量重复`GameObject`对象，通过提前设置一个原型可以极大降低创建游戏对象的开销,在创建新对象时使用`clone`即可。实际上这是与`.plist`制作结合的，需要提前将动画帧读取到内存中，并根据动画配置信息制作基本动画。

```cpp
anim_idle_   = RepeatForever::create(obj_template.anim_action_["idle"]->clone());
anim_idle_->retain();
anim_idle_->setTag(ActionTag::ANIMATION);
```
