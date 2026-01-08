#pragma once

#include "axmol.h"
#include "GameObject/GameObject.h"

class GameObjectFactory
{
public:
    static GameObject* create(GameObjectManager* manager,
                              GameObject::GameObjectType gameObjectType,
                              GameObject::CampType campType,
                              const std::string& templateName,
                              const ax::Vec2& position);

private:
    static int unique_id_counter_;
};