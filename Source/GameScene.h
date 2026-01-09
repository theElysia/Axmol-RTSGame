#pragma once

#include "axmol.h"
#include "GameWorld.h"
#include "GameUI.h"

class GameScene : public ax::Scene
{
public:
    bool init() override;

    GameScene() { AXLOGD("GameScene Create"); }

    ~GameScene() override
    {
        AXLOGD("GameScene Destroy");

        if (keyboard_listener_)
            _eventDispatcher->removeEventListener(keyboard_listener_);
        if (touch_listener_)
            _eventDispatcher->removeEventListener(touch_listener_);
    }

    void update(float delta) override;
    void switchKeyCrtl() { key_ctrl_pressed_ = !key_ctrl_pressed_; }

private:
    // void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event);
    // Touch
    bool onTouchBegin(ax::Touch* touch, ax::Event* event);
    void onTouchMoved(ax::Touch* touch, ax::Event* event);
    void onTouchEnded(ax::Touch* touch, ax::Event* event);

    void makeFinalEd(GameWorld::GameResultType result);
    void onGameBack(ax::Object* sender);

    GameWorld* game_world_ = nullptr;
    GameUI* gameUI_        = nullptr;

    ax::Rect client_rect_;
    ax::Rect game_world_rect_;

    ax::EventListenerKeyboard* keyboard_listener_   = nullptr;
    ax::EventListenerTouchOneByOne* touch_listener_ = nullptr;

    bool game_ended_ = false;

private:
    // game world control
    bool key_ctrl_pressed_ = false;
    bool key_l_switch_     = false;  // 切换控制阵营
    bool key_k_switch_     = true;   // 自动生成敌人
    ax::Vec2 touch_point_pressed_;
    ax::Vec2 touch_point_released_;
    bool touch_point_moved_ = false;

    GameObject::CampType keyL() { return key_l_switch_ ? GameObject::CampType::ENERMY1 : GameObject::CampType::PLAYER; }

    void enermyActionLogic();
    int tick_count_ = 0;

    void selectGameObjectWithCursor();
    void selectGameObjectAll();
    void cancelSelectGameObject();
    void moveSelectedObjectTo();

    void initBaseCamp();
};