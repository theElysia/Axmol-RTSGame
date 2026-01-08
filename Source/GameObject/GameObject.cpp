#include "GameObject/GameObject.h"
#include "GameObject/GameObjectManager.h"

using namespace ax;

bool GameObject::init(GameObjectManager* manager,
                      GameObjectType gameObjectType,
                      CampType campType,
                      const std::string& templateName,
                      const ax::Vec2& position,
                      int uniqueID)
{
    if (!Sprite::init())
    {
        return false;
    }

    game_object_manager_ = manager;
    unique_id_           = uniqueID;
    game_object_type_    = gameObjectType;
    camp_type_           = campType;
    template_name_       = templateName;
    this->setPosition(position);

    return true;
}

void GameObject::setReadyToRemove()
{
    is_ready_to_remove_ = true;
    game_object_manager_->addReadyToRemoveGameObject(unique_id_);
}

void GameObject::sendMessage(GameMessage* msg)
{
    game_object_manager_->handleMessage(msg);
}