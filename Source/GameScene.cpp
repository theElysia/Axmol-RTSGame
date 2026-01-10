#include "GameScene.h"
#include "ui/axmol-ui.h"
#include "MenuScene.h"

using namespace ax;

// EventKeyboard::KeyCode::KEY_A;

static std::vector<std::string> enermy_object = {"Slime", "Mushroom", "Ghost", "Skeleton"};

bool GameScene::init()
{
    if (!Scene::init())
    {
        return false;
    }
    keyboard_listener_                = EventListenerKeyboard::create();
    keyboard_listener_->onKeyReleased = AX_CALLBACK_2(GameScene::onKeyReleased, this);
    keyboard_listener_->onKeyPressed  = AX_CALLBACK_2(GameScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener_, this);

    touch_listener_               = EventListenerTouchOneByOne::create();
    touch_listener_->onTouchBegan = AX_CALLBACK_2(GameScene::onTouchBegin, this);
    touch_listener_->onTouchMoved = AX_CALLBACK_2(GameScene::onTouchMoved, this);
    touch_listener_->onTouchEnded = AX_CALLBACK_2(GameScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch_listener_, this);

    game_world_ = utils::createInstance<GameWorld>();
    game_world_->setAnchorPoint(Vec2::ZERO);
    gameUI_ = utils::createInstance<GameUI>(&GameUI::init, game_world_);
    gameUI_->setAnchorPoint(Vec2::ZERO);

    auto closeItem = MenuItemLabel::create(Label::createWithSystemFont("Back", "Arial", 16),
                                           AX_CALLBACK_1(GameScene::onGameBack, this));
    closeItem->setPosition(Vec2(540, 300));
    auto menu = Menu::create(closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);

    auto checkbox = ui::CheckBox::create("CheckBox_Normal.png", "CheckBox_Press.png", "CheckBoxNode_Normal.png",
                                         "CheckBox_Disable.png", "CheckBoxNode_Disable.png");
    checkbox->setPosition(Vec2(540, 20));
    checkbox->setScale(0.5f);

    checkbox->addTouchEventListener([=](ax::Object* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::BEGAN:
            break;
        case ui::Widget::TouchEventType::ENDED:
            this->cancelSelectGameObject();
            this->switchKeyCrtl();
            break;
        default:
            break;
        }
    });

    this->addChild(game_world_, 0, "gameworld");
    this->addChild(gameUI_, 1, "gameui");
    this->addChild(menu, 2, "back");
    this->addChild(checkbox, 2, "checkbox");

    game_world_rect_.origin = Vec2::ZERO;
    game_world_rect_.size   = game_world_->getMapSize();
    client_rect_            = _director->getSafeAreaRect();

    game_world_->setPosition(game_world_rect_.origin);
    gameUI_->setPosition(Vec2(game_world_rect_.origin.x + game_world_rect_.size.x, game_world_rect_.origin.y));

    initBaseCamp();

    game_world_->setGameRun();

    scheduleUpdate();

    return true;
}

void GameScene::update(float delta)
{
    if (!game_ended_ && game_world_->getGameState() != GameWorld::GameStateType::END)
    {
        gameUI_->update(delta);
        game_world_->update(delta);
        if (key_k_switch_)
            this->enermyActionLogic();
    }
    else if (!game_ended_)
    {
        game_ended_ = true;
        makeFinalEd(game_world_->getGameResult());
    }
}

void GameScene::enermyActionLogic()
{
    constexpr int enermy_interval      = 400;
    constexpr int enermy_rush_interval = 1000;
    tick_count_++;
    if (tick_count_ % enermy_interval == 0)
    {
        int t = ax::random<int>(0, 5) % 4;

        auto template_name = enermy_object[t];
        game_world_->deployGameRole(GameObject::CampType::ENERMY1, template_name);
    }

    if (tick_count_ % enermy_rush_interval == 0)
    {
        game_world_->letAllEnermyRush();
    }
}

void GameScene::makeFinalEd(GameWorld::GameResultType result)
{
    game_world_->setVisible(false);
    gameUI_->setVisible(false);

    std::string f;
    if (result == GameWorld::GameResultType::VICTORY)
        f = "victory.png";
    else
        f = "defeated.png";
    auto closeItem = MenuItemImage::create(f, f, AX_CALLBACK_1(GameScene::onGameBack, this));

    closeItem->setScale(0.2);

    auto menu = Menu::create(closeItem, NULL);
    this->addChild(menu, 3);

    menu->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    menu->setPosition(Vec2(client_rect_.getMidX(), client_rect_.getMidY()));
}

void GameScene::onGameBack(ax::Object* sender)
{
    _director->replaceScene(utils::createInstance<MenuScene>());
}

void GameScene::onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event)
{
    AXLOGD("GameScene onKeyPressed keycode: {}", static_cast<int>(code));
    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_1:
        game_world_->deployGameRole(keyL(), "Knight");
        break;
    case EventKeyboard::KeyCode::KEY_2:
        game_world_->deployGameRole(keyL(), "Wizard");
        break;
    case EventKeyboard::KeyCode::KEY_3:
        game_world_->deployGameRole(keyL(), "Ghost");
        break;
    case EventKeyboard::KeyCode::KEY_4:
        game_world_->deployGameRole(keyL(), "Skeleton");
        break;
    case EventKeyboard::KeyCode::KEY_5:
        game_world_->deployGameRole(keyL(), "Slime");
        break;
    case EventKeyboard::KeyCode::KEY_6:
        game_world_->deployGameRole(keyL(), "Mushroom");
        break;
    case EventKeyboard::KeyCode::KEY_CTRL:
        key_ctrl_pressed_ = true;

        break;
    case EventKeyboard::KeyCode::KEY_ESCAPE:
        cancelSelectGameObject();
        break;
    case EventKeyboard::KeyCode::KEY_A:
        if (key_ctrl_pressed_)
            game_world_->selectAll(GameObject::GameObjectType::ROLE, keyL());

        break;
    case EventKeyboard::KeyCode::KEY_P:
        game_world_->safeSwitchPauseRun();
        break;

    case EventKeyboard::KeyCode::KEY_L:
        key_l_switch_ = !key_l_switch_;
        break;
    case EventKeyboard::KeyCode::KEY_K:
        key_k_switch_ = !key_k_switch_;
        break;
    default:
        break;
    }
}

void GameScene::onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event)
{

    switch (code)
    {
    case EventKeyboard::KeyCode::KEY_CTRL:
        key_ctrl_pressed_ = false;
        break;

    default:
        break;
    }
}

bool GameScene::onTouchBegin(ax::Touch* touch, ax::Event* event)
{
    touch_point_pressed_ = touch->getLocation();
    touch_point_moved_   = false;
    return true;
}

void GameScene::onTouchMoved(ax::Touch* touch, ax::Event* event)
{
    touch_point_moved_ = true;
}

void GameScene::onTouchEnded(ax::Touch* touch, ax::Event* event)
{
    touch_point_released_ = touch->getLocation();
    if (key_ctrl_pressed_ && touch_point_moved_)
        this->selectGameObjectWithCursor();
    else
        this->moveSelectedObjectTo();
}

void GameScene::selectGameObjectWithCursor()
{
    if (game_world_->getGameState() != GameWorld::GameStateType::RUNNING)
        return;
    if (key_l_switch_)
        game_world_->selectObject(touch_point_pressed_, touch_point_released_, GameObject::GameObjectType::ROLE,
                                  GameObject::CampType::ENERMY1);
    else
        game_world_->selectObject(touch_point_pressed_, touch_point_released_, GameObject::GameObjectType::ROLE,
                                  GameObject::CampType::PLAYER);
}

void GameScene::selectGameObjectAll()
{
    if (game_world_->getGameState() != GameWorld::GameStateType::RUNNING)
        return;
    if (key_l_switch_)
        game_world_->selectAll(GameObject::GameObjectType::ROLE, GameObject::CampType::ENERMY1);
    else
        game_world_->selectAll(GameObject::GameObjectType::ROLE, GameObject::CampType::PLAYER);
}

void GameScene::cancelSelectGameObject()
{
    if (game_world_->getGameState() != GameWorld::GameStateType::RUNNING)
        return;
    game_world_->cancelSelect();
}

void GameScene::moveSelectedObjectTo()
{
    if (game_world_->getGameState() != GameWorld::GameStateType::RUNNING)
        return;
    if (game_world_rect_.containsPoint(touch_point_pressed_))
        game_world_->moveSelectedObjTo(touch_point_pressed_);
}

void GameScene::initBaseCamp()
{
    game_world_->setupBaseCamp(GameObject::CampType::PLAYER, "Building1", Vec2(16, 160));
    game_world_->setupBaseCamp(GameObject::CampType::ENERMY1, "Building2", Vec2(464, 160));
}