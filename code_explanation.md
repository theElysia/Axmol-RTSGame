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

部分次要的就不讲了，太naive了。

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

template <typename T>
inline T* createInstance()
{
    return ::ax::utils::createInstance<T>(&T::init);
}

auto game_role = ax::utils::createInstance<GameRole>(&GameRole::init, args...);
auto scene = utils::createInstance<MenuScene>();
```

##### 建造者模式(Builder)

这与游戏开发基本概念中的`导演(Director)`相关，参考[cocos2d-x manual](https://docs.cocos.com/cocos2d-x/v4/manual/zh/)。
游戏由导演控制`场景(Scene)`替换和转换，本例中场景就是`MenuScene`和`GameScene`。这里场景的生成就使用了建造者模式，当然具体类生成是用静态工厂方法。

当然不同层级通过GameObjectManager，再经由GameObjectFactory生成游戏对象也体现了建造者模式，只是没那么明显。

##### 装饰模式(Decorator)

这里主要是提一下`std::bind`方法，可以快速修饰一个函数（可选固定部分参数），通常用于传递类的成员函数（将其与this绑定），比用`lambda`方便并且明确。

```cpp
std::bind(&func, fixed_arg, std::::placeholders::_1);
```

##### 单例模式(Singleton)

很明显`导演(Director)`或者`GameResourceHandler`等是需要单例实现。导演的单例由引擎实现，自己写的单例参考`BaseTemplates.h`，使用了**CRTP**来提供接口（类比java中的implements）。

```cpp
// CRTP Singleton
template <typename T>
class Singleton
{
protected:
    Singleton()  = default;
    ~Singleton() = default;

    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static T& instance()
    {
        static T inst;
        return inst;
    }

    static T* getInstance() { return &instance(); }
};
```

当然在开发过程中发现单例其实很容易滥用（这就相当于一个全局变量），其生命周期以及可见性会产生问题，所以还是尽量避免使用。例如GameObjectManager就未使用单例，但GameMapManager还是使用单例了，因为挺多地方需要用到寻路（GameWorld,MessageHandler）。


##### 观察者模式(Observer)

游戏需要响应玩家的输入（键盘，鼠标等）。通过向这些监视器注册观察者（回调函数），就可以实现各种功能。比如`GameScene`中的代码：

```cpp
keyboard_listener_                = EventListenerKeyboard::create();
keyboard_listener_->onKeyReleased = AX_CALLBACK_2(GameScene::onKeyReleased, this);
keyboard_listener_->onKeyPressed  = AX_CALLBACK_2(GameScene::onKeyPressed, this);
_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener_, this);
```


##### 策略模式(Strategy)

设置一个统一的接口，当有需求时方便扩展与替换算法。比如寻路算法抽象基类接口设置如下：

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

RTS游戏中有大量游戏对象，而且要实现选中移动等功能（对玩家输入做出响应）。若每个对象分别作为一个观察者注册到事件列表中，那么显然性能问题严重。这时候就需要有统一管理的`GameObjectManager`，同时一些选中状态/大本营管理等逻辑也可以由其完成。

##### 状态模式(State)

随着游戏进行，一个游戏对象会有多种状态，比如战斗与空闲，行为上说就是不同状态下会有不同的基础逻辑与响应动作，那么这样就需要状态模式来介入，提供可扩展性与可维护性。

当然，为了进一步提升性能，避免过多子类，我使用了静态状态，`Object`和`Context`相当于每次作为参数提供。

```cpp
// State Pattern & Singleton & CRTP
class GameRoleState
{
public:
    virtual ~GameRoleState() = default;

    virtual void update(GameRole* role);

    virtual void handleCommand(GameRole* role, GameCommand* cmd);
};

class GameRoleStateIdle : public GameRoleState, public Singleton<GameRoleStateIdle>
{
public:
    void update(GameRole* role) override;
    void handleCommand(GameRole* role, GameCommand* cmd) override;

private:
    friend class Singleton<GameRoleStateIdle>;
    GameRoleStateIdle() = default;
};
```

##### 责任链模式(Chain of Responsibility)

`GameMessage`由`GameObject`产生并向上传播，接受这样的消息并处理很自然想到用责任链模式。

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

    ResultType handle(GameMessage* msg)
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

protected:
    virtual bool canHandle(GameMessage* msg) const = 0;
    virtual ResultType process(GameMessage* msg)   = 0;

private:
    std::shared_ptr<GameMessageHandler> next_;
};
```

##### 命令模式(Command)

显然`GameCommand`描述的就是这种模式，当然这也需要`GameObject`，`RTSCommandPool`的配合。为了实现高效，`RTSCommandPool`的设计还是较为复杂的，有内存池管理，双缓冲结构，批处理模式（可选并行）。

```cpp
class GameObject : public ax::Sprite
{
public:
    virtual void handleCommand(GameCommand* cmd) = 0;

    void sendMessage(GameMessage* msg);
};
```

```cpp
class GameCommand
{
public:
    // 与执行优先级一致
    enum CommandType
    {
        UNDEF,

        MOVE,
        GET_ATTACK,

        SELECT,
        DESELECT,
        SETTARGET,

        TOTAL_NUM
    };

public:
    GameCommand(int receiverId) : receiver_id_(receiverId) {}
    virtual ~GameCommand() = default;

    virtual CommandType getType() const = 0;
    int getReceiverId() const { return receiver_id_; }

private:
    int receiver_id_ = 0;
};
```

```cpp
class RTSCommandPool
{
public:
    RTSCommandPool() = default;

    RTSCommandPool(const RTSCommandPool&)            = delete;
    RTSCommandPool& operator=(const RTSCommandPool&) = delete;

    // 添加命令到当前帧缓冲区，并自动分组
    template <typename T, typename... Args>
    void addCommand(Args&&... args)
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

    void processPreviousFrame(GameObjectManager* manager);
    void swapBuffers();
    void clearAll();
};
```

##### 原型模式(Prototype)

游戏会有大量重复`GameObject`对象，通过提前设置一个原型可以极大降低创建游戏对象的开销。实际上这是与`.plist`制作结合的，需要提前将动画帧读取到内存中，并根据动画配置信息制作基本动画。

```cpp
anim_idle_   = RepeatForever::create(obj_template.anim_action_["idle"]->clone());
anim_idle_->retain();
anim_idle_->setTag(ActionTag::ANIMATION);

```
