#include "BattleScene.h"

using namespace ax;

bool BattleScene::init()
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
    auto safeOrigin  = safeArea.origin;

    //////////////////////////////
    // 2. add the menu item for back to main menu
    auto label    = Label::createWithSystemFont("Back", "Arial", 32);
    auto menuItem = MenuItemLabel::create(label);
    menuItem->setCallback([&](ax::Object* sender) { _director->popScene(); });

    auto menu = Menu::create(menuItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    menuItem->setPosition(Vec2(safeOrigin.x + safeArea.size.width - 40, safeArea.origin.y + safeArea.size.height - 32));
    this->addChild(menu, 1);

    //////////////////////////////
    // 3. TileMap
    _map = TMXTiledMap::create("map2.tmx");
    _map->setPosition(Vec2::ZERO);
    // _map->setPosition(Vec2(visibleSize.width / 2 - _map->getContentSize().width / 2, visibleSize.height / 2));

    auto _touchListener = EventListenerTouchOneByOne::create();

    _touchListener->onTouchBegan = [&](Touch* touch, Event* event) {
        return true;  // consuming the event
    };

    _touchListener->onTouchEnded = [=](Touch* touch, Event* event) {
        auto layer    = _map->getLayer("layer0");
        Size viewSize = _director->getWinSize();

        Vec2 mapCordinate = _map->convertToNodeSpace(_director->convertToGL(touch->getLocationInView()));
        int tileX         = mapCordinate.x / _map->getTileSize().width;
        int tileY         = (viewSize.height - mapCordinate.y) / _map->getTileSize().height;

        int tileGID = layer->getTileGIDAt(Vec2(tileX, tileY));
        if (tileGID != 0)
        {
            auto tileSprite = layer->getTileAt(Vec2(tileX, tileY));
            // Do something with the tile
        }

        AXLOGD("Tile GID: {}, tileX: {}, tileY: {}", tileGID, tileX, tileY);
    };

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, _map);

    this->addChild(_map, 0, 99);

    return true;
}

BattleScene::BattleScene()
{
    AXLOGD("BattleScene: ctor");
}

BattleScene::~BattleScene()
{
    AXLOGD("BattleScene: dtor");

    if (_map)
        delete _map;
}
