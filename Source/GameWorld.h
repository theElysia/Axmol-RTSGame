#pragma once

#include "axmol.h"
#include "GameObject/GameObject.h"
#include "GameObject/GameObjectManager.h"
#include "GameObject/GameResourceHandler.h"
#include "GameMap/GameMapManager.h"
#include "GameMessage/RTSCommandPool.h"
#include "GameMessage/GameMessageHandler.h"

class GameWorld : public ax::Node
{
public:
    enum class GameStateType
    {
        INITIAL,
        RUNNING,
        PAUSE,
        END,
    };

    enum class GameResultType
    {
        VICTORY,
        DEFEATED,
    };

public:
    bool init() override;

    GameWorld() { AXLOGD("GameWorld Create"); }
    ~GameWorld() override
    {
        {
            AXLOGD("GameWorld Destroy");
            if (command_pool_)
            {
                delete command_pool_;
            }
            if (game_object_manager_)
            {
                delete game_object_manager_;
            }
            if (game_map_manager_)
            {
                delete game_map_manager_;
            }
        }
    }

    void setGameEnded(GameResultType result);

    void update(float delta) override;

    void selectObject(const ax::Vec2& cursorPoint1,
                      const ax::Vec2& cursorPoint2,
                      GameObject::GameObjectType gameObjectType = GameObject::GameObjectType::ROLE,
                      GameObject::CampType campType             = GameObject::CampType::PLAYER);
    void selectAll(GameObject::GameObjectType gameObjectType = GameObject::GameObjectType::ROLE,
                   GameObject::CampType campType             = GameObject::CampType::PLAYER);
    void cancelSelect();
    void moveSelectedObjTo(const ax::Vec2& cursorPoint);

    GameStateType getGameState() { return game_state_; }
    GameResultType getGameResult() { return game_result_; }
    // void setGameState(GameStateType newState) { game_state_ = newState; }
    void setGamePause() { game_state_ = GameStateType::PAUSE; }
    void setGameRun() { game_state_ = GameStateType::RUNNING; }
    void safeSwitchPauseRun();

    ax::Vec2 getMapSize() { return game_map_manager_->getMapSize(); }

    void setupBaseCamp(GameObject::CampType campType, const std::string& templateName, const ax::Vec2& position);
    // 放置在对应大本营附近
    void deployGameRole(GameObject::CampType campType, const std::string& templateName);

    void letAllRoleRushTo(GameObject::CampType campType, const ax::Vec2& pos);
    void letAllEnermyRush();

    void handleMessage(GameMessage* msg);

private:
    GameObjectManager* game_object_manager_ = nullptr;
    GameMapManager* game_map_manager_       = nullptr;
    RTSCommandPool* command_pool_           = nullptr;
    GameResourceHandler* game_resource_     = nullptr;

    GameStateType game_state_   = GameStateType::INITIAL;
    GameResultType game_result_ = GameResultType::VICTORY;

    ax::Node* game_object_layer_ = nullptr;

    std::shared_ptr<GameMessageHandler> game_message_handler_;
};