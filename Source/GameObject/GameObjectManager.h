#pragma once

#include "axmol.h"
#include "GameObject/GameObject.h"
#include <memory>

class RTSCommandPool;
class GameWorld;
class GameMessage;
class GameMapManager;

class GameObjectManager
{
public:
    GameObjectManager()  = default;
    ~GameObjectManager() = default;

public:
    void init(GameWorld* gameWorld,
              ax::Node* gameObjectLayer,
              RTSCommandPool* commandPool,
              GameMapManager* gameMapManager);

    void update(float delta);

    void setBaseCamp(GameObject* baseCampObj, GameObject::CampType campType);
    ax::Vec2 getBaseCampPos(GameObject::CampType campType);

    GameObject* createGameObject(GameObject::GameObjectType gameObjectType,
                                 GameObject::CampType campType,
                                 const std::string& templateName,
                                 const ax::Vec2& position);

    void removeGameObjectBy(int uniqueID);
    void removeAllGameObjects();

    void addReadyToRemoveGameObject(int uniqueID);
    void removeAllReadyToRemoveGameObjects();

    GameObject* getGameObjectBy(int uniqueID);
    const std::unordered_map<int, GameObject*>& getGameObjectPool() { return game_object_pool_; }

    void selectAll(GameObject::GameObjectType gameObjectType = GameObject::GameObjectType::ROLE,
                   GameObject::CampType campType             = GameObject::CampType::PLAYER);
    void selectWithRect(const ax::Vec2& cursorPoint1,
                        const ax::Vec2& cursorPoint2,
                        GameObject::GameObjectType gameObjectType = GameObject::GameObjectType::ROLE,
                        GameObject::CampType campType             = GameObject::CampType::PLAYER);
    void cancelAllSelected();
    size_t getSelectedObjCount() { return selected_object_.size(); }

    void moveSelectedObjTo(const ax::Vec2& position);

    void handleMessage(GameMessage* msg);

private:
    void sendSelectCommand(bool select = true);
    // void sendMessage(GameMessage* msg);

private:
    GameWorld* game_world_            = nullptr;
    ax::Node* game_object_layer_      = nullptr;
    RTSCommandPool* command_pool_     = nullptr;
    GameMapManager* game_map_manager_ = nullptr;

    int tick_ = 0;

    std::unordered_map<GameObject::CampType, int> base_camp_;
    std::unordered_map<GameObject::CampType, bool> base_camp_alive_;

    std::unordered_map<int, GameObject*> game_object_pool_;

    std::unordered_set<int> selected_object_;
    std::vector<int> ready_to_remove_object_;
};
