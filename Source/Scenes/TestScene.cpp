#include "TestScene.h"
#include "MenuScene.h"

using namespace ax;

static Vector<SpriteFrame*> getAnimation(const char* format, int count)
{
    auto spritecache = SpriteFrameCache::getInstance();
    Vector<SpriteFrame*> animFrames(count);
    char str[100];
    for (int i = 0; i < count; i++)
    {
        snprintf(str, sizeof(str), format, i);
        animFrames.pushBack(spritecache->getSpriteFrameByName(str));
    }
    return animFrames;
}

bool TestScene::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();
    auto origin      = _director->getVisibleOrigin();
    auto safeArea    = _director->getSafeAreaRect();
    auto safeSize    = safeArea.size;
    auto safeOrigin  = safeArea.origin;

    //////////////////////////////
    // 2. add the menu item for back to main menu
    auto label    = Label::createWithSystemFont("Back", "Arial", 24);
    auto menuItem = MenuItemLabel::create(label);
    menuItem->setCallback([&](ax::Object* sender) { _director->replaceScene(utils::createInstance<MenuScene>()); });

    auto menu = Menu::create(menuItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    menuItem->setPosition(Vec2(safeOrigin.x + safeArea.size.width - 40, safeArea.origin.y + safeArea.size.height - 32));
    this->addChild(menu, 1);

    auto spritecache = SpriteFrameCache::getInstance();
    spritecache->addSpriteFramesWithFile("Knight.plist");
    spritecache->addSpriteFramesWithFile("Ghost.plist");
    auto frame1 = spritecache->getSpriteFrameByName("Knight_Attack01/0.png");

    Vector<SpriteFrame*> frames = getAnimation("Knight_Attack01/%d.png", 9);
    // test sprite
    auto sprite1 = Sprite::createWithSpriteFrame(frames[0]);
    sprite1->setAnchorPoint(Vec2::ZERO);
    sprite1->setPosition(safeOrigin + Vec2(safeSize.x / 2, safeSize.y / 2));
    this->addChild(sprite1, 2, 3);
    auto sprite2 = Sprite::createWithSpriteFrame(frames[0]);
    sprite2->setAnchorPoint(Vec2::ZERO);
    sprite2->setPosition(safeOrigin + Vec2(safeSize.x / 4, safeSize.y / 4));
    this->addChild(sprite2, 2, 4);

    _animation["attack"] = Animation::createWithSpriteFrames(getAnimation("Knight_Attack01/%d.png", 9), 1.0f / 9);
    _animation["idle"]   = Animation::createWithSpriteFrames(getAnimation("Ghost_Idle/%d.png", 6), 1.0f / 6);
    _animation["run"]    = Animation::createWithSpriteFrames(getAnimation("Knight_Run/%d.png", 8), 1.0f / 8);
    _animation["attack"]->retain();
    _animation["idle"]->retain();
    _animation["run"]->retain();
    _action["attack"] = Animate::create(_animation["attack"]);
    _action["idle"]   = RepeatForever::create(Animate::create(_animation["idle"]));
    _action["run"]    = RepeatForever::create(Animate::create(_animation["run"]));
    _action["attack"]->retain();
    _action["idle"]->retain();
    _action["run"]->retain();
    // auto action1   = RepeatForever::create(Animate::create(animation));
    // sprite1->runAction(action1);
    sprite1->runAction(RepeatForever::create(Animate::create(_animation["idle"])));

    // k->runAction();

    //////////////////////////////
    // 3. TileMap
    _map = TMXTiledMap::create("map3.tmx");
    _map->setPosition(Vec2::ZERO);
    this->addChild(_map, 0);

    _keyboardListener               = EventListenerKeyboard::create();
    _keyboardListener->onKeyPressed = AX_CALLBACK_2(TestScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_keyboardListener, this);

    _touchListener               = EventListenerTouchOneByOne::create();
    _touchListener->onTouchBegan = AX_CALLBACK_2(TestScene::onTouchBegin, this);
    _touchListener->onTouchEnded = AX_CALLBACK_2(TestScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);

    _hpBar = ui::LoadingBar::create("HpBarGreen.png");
    _hpBar->setAnchorPoint(Vec2::ZERO);
    _hpBar->setPercent(100.0f);

    auto hpBarBackground = Sprite::create("HpBarBackground.png");
    hpBarBackground->setCascadeOpacityEnabled(true);
    hpBarBackground->addChild(_hpBar);
    hpBarBackground->setScale(0.2f);
    hpBarBackground->setVisible(true);
    hpBarBackground->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
    // hpBarBackground->setPosition(safeOrigin + Vec2(safeSize.x / 4, safeSize.y / 4));

    // auto sp = Sprite::create(hpBarBackground);

    sprite2->addChild(hpBarBackground);

    // addChild(hpBarBackground, 1);
    _director->getFrameRate();
    scheduleUpdate();

    return true;
}

void TestScene::update(float delta)
{
    // AXLOGD("update: {}", delta);
}

bool TestScene::onTouchBegin(ax::Touch* touch, ax::Event* event)
{
    return true;
}

void TestScene::onTouchEnded(ax::Touch* touch, ax::Event* event)
{
    auto layer     = _map->getLayer("layer0");
    auto pos       = touch->getLocation();
    auto tile_size = layer->getMapTileSize();
    auto view_size = _director->getWinSize();
    auto map_size  = _map->getMapSize();

    int tileX = pos.x / tile_size.x;
    int tileY = (view_size.y - pos.y) / tile_size.y;

    int id = layer->getTileGIDAt(Vec2(tileX, tileY));
    // int id1 = layer->getTileGIDAt(pos);
    // int id2 = layer->getTileGIDAt(Vec2(pos.x, view_size.y - pos.y));

    Sprite* tileSprite = layer->getTileAt(Vec2(tileX, tileY));
    tileSprite->setVisible(false);
    // layer->removeChild(tileSprite);

    AXLOGD("Scene: onTouchEnded, keycode:x {} y {}", pos.x, pos.y);
    AXLOGD("tile x {} y {}", tileX, tileY);
    // AXLOGD("tile id {}  id1 {}  id2 {}", id, id1, id2);
    AXLOGD("tile id {}", id);
    AXLOGD("map size {}   {} ", map_size.x, map_size.y);
}

void TestScene::onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event)
{
    AXLOGD("framerate {}", _director->getFrameRate());
    AXLOGD("delta {}", _director->getDeltaTime());
    AXLOGD("Scene: onKeyPressed, keycode: {}", static_cast<int>(code));
    ax::Sprite* sprite  = static_cast<ax::Sprite*>(this->getChildByTag(3));
    ax::Sprite* sprite2 = static_cast<ax::Sprite*>(this->getChildByTag(4));
    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_A:
        AXLOGD("Scene: onKeyPressed, A keycode: {}", static_cast<int>(code));  // 3. 检查特定动画是否存在
        if (_animation.find("attack") == _animation.end())
        {
            AXLOGE("ERROR: Attack animation not found!");
            return;
        }

        if (!_animation["attack"])
        {
            AXLOGE("ERROR: Attack animation is null!");
            return;
        }

        // 4. 调试输出动画信息
        AXLOGD("Attack animation address: {}", (void*)_animation["attack"]);
        AXLOGD("Animation frame count: {}", _animation["attack"]->getFrames().size());
        /* code */
        sprite->stopAllActions();
        // sprite->runAction(Animate::create(_animation["attack"]));
        sprite->runAction(_action["attack"]->clone());
        sprite2->stopAllActions();
        // sprite2->runAction(Animate::create(_animation["attack"]));
        // sprite2->runAction();
        // sprite2->runAction(_action["attack"]->clone());
        sprite2->runAction(Sequence::create(_action["attack"]->clone(),
                                            CallFunc::create([=]() { sprite2->runAction(_action["run"]->clone()); }),
                                            nullptr));
        // sprite2->runAction(_action["attack"]->clone());
        // sprite2->runAction(_animation["attack"]);
        break;
    case EventKeyboard::KeyCode::KEY_B:
        AXLOGD("Scene: onKeyPressed, b keycode: {}", static_cast<int>(code));
        /* code */
        sprite->stopAllActions();
        // sprite->runAction(RepeatForever::create(Animate::create(_animation["idle"])));
        sprite->runAction(_action["idle"]);
        // sprite2->stopAllActions();
        // // sprite2->runAction(RepeatForever::create(Animate::create(_animation["idle"])));
        // sprite2->runAction(_action["idle"]->clone());
        break;
    case EventKeyboard::KeyCode::KEY_C:
        AXLOGD("Scene: onKeyPressed, c keycode: {}", static_cast<int>(code));
        /* code */
        sprite->stopAllActions();
        // sprite->runAction(RepeatForever::create(Animate::create(_animation["run"])));
        sprite->runAction(_action["run"]->clone());
        break;
    case EventKeyboard::KeyCode::KEY_E:
        sprite2->setFlippedX(true);
        break;
    case EventKeyboard::KeyCode::KEY_R:
        sprite2->setFlippedX(false);
        break;

    case EventKeyboard::KeyCode::KEY_D:
        AXLOGD("Scene: onKeyPressed, c keycode: {}", static_cast<int>(code));
        /* code */
        sprite2->stopAllActions();
        // sprite2->runAction(RepeatForever::create(Animate::create(_animation["run"])));
        sprite2->runAction(_action["run"]->clone());
        break;
    case EventKeyboard::KeyCode::KEY_0:
        AXLOGD("Scene: onKeyPressed, keycode: {}", static_cast<int>(code));
        /* code */
        _hpBar->setPercent(10.0f);
        sprite2->setColor(ax::Color3B::WHITE);
        break;
    case EventKeyboard::KeyCode::KEY_1:
        AXLOGD("Scene: onKeyPressed, keycode: {}", static_cast<int>(code));
        /* code */
        _hpBar->setPercent(50.0f);
        sprite2->setColor(ax::Color3B::RED);
        break;
    case EventKeyboard::KeyCode::KEY_2:
        AXLOGD("Scene: onKeyPressed, keycode: {}", static_cast<int>(code));
        /* code */
        _hpBar->setPercent(100.0f);
        sprite2->setColor(ax::Color3B::BLACK);
        break;
    case EventKeyboard::KeyCode::KEY_3:
        AXLOGD("Scene: onKeyPressed, keycode: {}", static_cast<int>(code));
        /* code */
        {
            Vector<FiniteTimeAction*> vec;
            vec.pushBack(MoveBy::create(1, Vec2(50, 0)));
            vec.pushBack(MoveBy::create(1, Vec2(0, 50)));
            auto t = Sequence::create(vec);
            sprite2->runAction(t);
        }
        break;

    default:
        break;
    }

    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_I:
    {
        auto act1 = MoveBy::create(1, Vec2(0, 50));
        act1->setTag(1);
        sprite2->stopActionByTag(1);
        sprite2->runAction(act1);
    }
    break;
    case EventKeyboard::KeyCode::KEY_L:
    {
        auto act1 = MoveBy::create(1, Vec2(50, 0));
        act1->setTag(1);
        sprite2->stopActionByTag(1);
        sprite2->runAction(act1);
    }
    break;

    case EventKeyboard::KeyCode::KEY_K:
    {
        auto act1 = MoveBy::create(1, Vec2(0, -50));
        act1->setTag(1);
        sprite2->stopActionByTag(1);
        sprite2->runAction(act1);
    }
    break;

    case EventKeyboard::KeyCode::KEY_J:
    {
        auto act1 = MoveBy::create(1, Vec2(-50, 0));
        act1->setTag(1);
        sprite2->stopActionByTag(1);
        sprite2->runAction(act1);
    }
    break;

    default:
        break;
    }
}

TestScene::TestScene()
{
    AXLOGD("TestScene: ctor");
}

TestScene::~TestScene()
{
    AXLOGD("TestScene: dtor");
    if (_keyboardListener)
        _eventDispatcher->removeEventListener(_keyboardListener);
}
