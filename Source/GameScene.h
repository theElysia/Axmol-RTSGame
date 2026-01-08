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

private:
    // void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event);
    // Touch
    bool onTouchBegin(ax::Touch* touch, ax::Event* event);
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
    bool key_l_switch_     = false;
    ax::Vec2 cursor_point_pressed_;
    ax::Vec2 cursor_point_released_;

    void selectGameObjectWithCursor();
    void selectGameObjectAll();
    void cancelSelectGameObject();
    void moveSelectedObjectTo();

    void initBaseCamp();
};