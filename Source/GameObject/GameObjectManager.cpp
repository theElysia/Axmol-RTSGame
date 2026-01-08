#include "GameObject/GameObjectManager.h"
#include "GameObject/GameObjectFactory.h"
#include "GameMap/GameMapManager.h"
#include "GameMessage/RTSCommandPool.h"
#include "GameWorld.h"
#include "GameMessage/GameMessageHandler.h"

using namespace ax;

void GameObjectManager::init(GameWorld* gameWorld, ax::Node* gameObjectLayer, RTSCommandPool* commandPool)
{
    game_world_        = gameWorld;
    game_object_layer_ = gameObjectLayer;
    command_pool_      = commandPool;
}

void GameObjectManager::handleMessage(GameMessage* msg)
{
    game_world_->handleMessage(msg);
}

void GameObjectManager::update(float delta)
{
    tick_++;
    // if (tick_ % GameWorld::state_check_interval_tick == 0)
    for (auto& it : base_camp_)
    {
        auto obj = this->getGameObjectBy(it.second);
        if (!obj)
        {
            AXLOGD("camp destory id: {}", static_cast<int>(it.first));
            base_camp_alive_[it.first] = false;
        }
    }
    if (base_camp_alive_[GameObject::CampType::ENERMY1] == false)
    {
        AXLOGD("camp destory {}", static_cast<int>(GameObject::CampType::ENERMY1));
        game_world_->setGameEnded(GameWorld::GameResultType::VICTORY);
    }
    if (base_camp_alive_[GameObject::CampType::PLAYER] == false)
    {
        AXLOGD("camp destory {}", static_cast<int>(GameObject::CampType::PLAYER));
        game_world_->setGameEnded(GameWorld::GameResultType::DEFEATED);
    }
    this->removeAllReadyToRemoveGameObjects();
    for (auto& it : game_object_pool_)
    {
        it.second->update(delta);
    }
}

void GameObjectManager::setBaseCamp(GameObject* baseCampObj, GameObject::CampType campType)
{
    base_camp_[campType]       = baseCampObj->getUniqueID();
    base_camp_alive_[campType] = true;
}

GameObject* GameObjectManager::createGameObject(GameObject::GameObjectType gameObjectType,
                                                GameObject::CampType campType,
                                                const std::string& templateName,
                                                const ax::Vec2& position)
{
    auto obj = GameObjectFactory::create(this, gameObjectType, campType, templateName, position);

    game_object_pool_[obj->getUniqueID()] = obj;
    game_object_layer_->addChild(obj, 0, obj->getUniqueID());
    AXLOGD("make game object {}", obj->getUniqueID());
    return obj;
}

void GameObjectManager::removeGameObjectBy(int uniqueID)
{
    auto obj = this->getGameObjectBy(uniqueID);
    if (obj)
    {
        game_object_pool_.erase(uniqueID);
        game_object_layer_->removeChildByTag(uniqueID);
    }
}

void GameObjectManager::removeAllGameObjects()
{
    for (auto& it : game_object_pool_)
    {
        auto obj = it.second;
        game_object_layer_->removeChildByTag(obj->getUniqueID());
        // delete obj;
    }
    game_object_pool_.clear();
}

void GameObjectManager::addReadyToRemoveGameObject(int uniqueID)
{
    ready_to_remove_object_.push_back(uniqueID);
}

void GameObjectManager::removeAllReadyToRemoveGameObjects()
{
    for (auto id : ready_to_remove_object_)
    {
        auto obj = this->getGameObjectBy(id);
        if (obj)
        {
            game_object_pool_.erase(id);
            game_object_layer_->removeChildByTag(id);
        }
    }
    ready_to_remove_object_.clear();
}

GameObject* GameObjectManager::getGameObjectBy(int uniqueID)
{
    auto it = game_object_pool_.find(uniqueID);
    if (it != game_object_pool_.end())
        return it->second;
    else
        return nullptr;
}

void GameObjectManager::selectAll(GameObject::GameObjectType gameObjectType, GameObject::CampType campType)
{
    for (auto& it : game_object_pool_)
    {
        auto obj = it.second;
        if (obj->getGameObjectType() == gameObjectType && obj->getCampType() == campType)
        {
            selected_object_.insert(it.first);
        }
    }
    sendSelectCommand(true);
}

void GameObjectManager::selectWithRect(const ax::Vec2& cursorPoint1,
                                       const ax::Vec2& cursorPoint2,
                                       GameObject::GameObjectType gameObjectType,
                                       GameObject::CampType campType)
{
    ax::Vec2 pos1(std::min<float>(cursorPoint1.x, cursorPoint2.x), std::min<float>(cursorPoint1.y, cursorPoint2.y));
    ax::Vec2 pos2(std::max<float>(cursorPoint1.x, cursorPoint2.x), std::max<float>(cursorPoint1.y, cursorPoint2.y));
    for (auto& it : game_object_pool_)
    {
        auto obj = it.second;
        if (obj->getGameObjectType() == gameObjectType && obj->getCampType() == campType)
        {
            auto pos = obj->getPosition();
            if (pos.x > pos1.x && pos.x < pos2.x && pos.y > pos1.y && pos.y < pos2.y)
                selected_object_.insert(it.first);
        }
    }
    sendSelectCommand(true);
}

void GameObjectManager::cancelAllSelected()
{
    sendSelectCommand(false);
    selected_object_.clear();
}

void GameObjectManager::moveSelectedObjTo(const ax::Vec2& position)
{
    auto _manager = GameMapManager::getInstance();
    _manager->setDestForMassivePath(position);
    for (auto id : selected_object_)
    {
        auto obj = this->getGameObjectBy(id);
        if (obj)
        {
            auto pos  = obj->getPosition();
            auto path = _manager->getMassivePath(pos);
            if (path.has_value())
                command_pool_->addCommand<CommandMove>(id, std::move(path.value()), 0);
        }
    }
}

void GameObjectManager::sendSelectCommand(bool select)
{
    for (auto it : selected_object_)
    {
        if (select)
            command_pool_->addCommand<CommandSelect>(it);
        else
            command_pool_->addCommand<CommandDeSelect>(it);
    }
}

ax::Vec2 GameObjectManager::getBaseCampPos(GameObject::CampType campType)
{
    auto it1 = base_camp_.find(campType);
    if (it1 == base_camp_.end())
        return Vec2::ZERO;
    auto it2 = this->getGameObjectBy(it1->second);
    if (it2 == nullptr)
        return Vec2::ZERO;
    return it2->getPosition();
}