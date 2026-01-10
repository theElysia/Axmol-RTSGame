#include "GameObject/GameObjectFactory.h"
#include "GameObject/GameBuilding.h"
#include "GameObject/GameRole.h"

int GameObjectFactory::unique_id_counter_ = 1000;

using GameObjectType = GameObject::GameObjectType;
using CampType       = GameObject::CampType;

GameObject* GameObjectFactory::create(GameObjectManager* manager,
                                      GameObject::GameObjectType gameObjectType,
                                      GameObject::CampType campType,
                                      const std::string& templateName,
                                      const ax::Vec2& position)
{
    // GameObject* gameObject = nullptr;

    switch (gameObjectType)
    {
    case GameObjectType::ROLE:
    {
        unique_id_counter_++;
        // auto gameObject = new GameRole;
        // gameObject->init(manager, gameObjectType, campType, templateName, position, unique_id_counter_);
        auto game_role = ax::utils::createInstance<GameRole>(&GameRole::init, manager, gameObjectType, campType,
                                                             templateName, position, unique_id_counter_);
        return game_role;
    }
    break;
    case GameObjectType::BUILDING:
    {
        unique_id_counter_++;
        // auto game_building = new GameBuilding;
        // game_building->init(manager, gameObjectType, campType, templateName, position, unique_id_counter_);
        auto game_building = ax::utils::createInstance<GameBuilding>(
            &GameBuilding::init, manager, gameObjectType, campType, templateName, position, unique_id_counter_);
        return game_building;
    }
    break;
    default:
        break;
    }

    return nullptr;
}