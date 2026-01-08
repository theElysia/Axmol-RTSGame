#pragma once

#include "axmol.h"
#include "GameObject/GameObject.h"
#include "GameObject/AttributeHandler.h"

class GameBuilding : public GameObject
{
public:
    GameBuilding() {};
    ~GameBuilding() override {};

    bool init(GameObjectManager* manager,
              GameObjectType gameObjectType,
              CampType campType,
              const std::string& templateName,
              const ax::Vec2& position,
              int uniqueID) override;

    void update(float delta) override;
    void handleCommand(GameCommand* cmd) override;

public:
    // 业务逻辑
    AttributeHandler_Hp h_hp_;

private:
    ax::Sprite* building_sprite_ = nullptr;
};