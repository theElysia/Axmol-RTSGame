#pragma once

#include "axmol.h"
#include "ui/axmol-ui.h"

class TestScene : public ax::Scene
{
public:
    bool init() override;

    TestScene();
    ~TestScene() override;

    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);

    bool onTouchBegin(ax::Touch* touch, ax::Event* event);
    void onTouchEnded(ax::Touch* touch, ax::Event* event);

    void update(float delta) override;

private:
    ax::EventListenerKeyboard* _keyboardListener   = nullptr;
    ax::EventListenerTouchOneByOne* _touchListener = nullptr;
    std::unordered_map<std::string, ax::Animation*> _animation;
    std::unordered_map<std::string, ax::FiniteTimeAction*> _action;
    ax::ui::LoadingBar* _hpBar = nullptr;
    ax::FastTMXTiledMap* _map  = nullptr;
};
