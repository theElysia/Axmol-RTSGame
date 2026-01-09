#pragma once

#include "GameMessage/GameMessage.h"
#include <memory>

class RTSCommandPool;
class GameObjectManager;
class GameMapManager;

class GameMessageHandler
{
public:
    enum class ResultType
    {
        CONSUMED,
        UNHANDLED,
        REJECTED,
    };

public:
    virtual ~GameMessageHandler() = default;

    void setNext(std::shared_ptr<GameMessageHandler> next) { next_ = next; }

    ResultType handle(GameMessage* msg)
    {
        GameMessageHandler* current = this;

        while (current)
        {
            if (current->canHandle(msg))
            {
                return current->process(msg);
            }
            current = current->next_.get();
        }

        return ResultType::UNHANDLED;
    }

protected:
    virtual bool canHandle(GameMessage* msg) const = 0;
    virtual ResultType process(GameMessage* msg)   = 0;

private:
    std::shared_ptr<GameMessageHandler> next_;
};

class GameMessageHandler_Attack : public GameMessageHandler
{
public:
    GameMessageHandler_Attack(RTSCommandPool* commandPool,
                              GameObjectManager* gameObjectManager,
                              GameMapManager* gameMapManager)
        : command_pool_(commandPool), game_object_manager_(gameObjectManager), game_map_manager_(gameMapManager)
    {}

protected:
    bool canHandle(GameMessage* msg) const override
    {
        return msg->getType() == GameMessage::MessageType::REQUEST_ATTACK_SINGLE ||
               msg->getType() == GameMessage::MessageType::REQUEST_ATTACK_AOE;
    }
    ResultType process(GameMessage* msg) override;

private:
    ResultType processSingle(MessageRequestAttackSingle* msg);
    ResultType processAoe(MessageRequestAttackAoe* msg);

    RTSCommandPool* command_pool_           = nullptr;
    GameObjectManager* game_object_manager_ = nullptr;
    GameMapManager* game_map_manager_       = nullptr;
};

class GameMessageHandler_Move : public GameMessageHandler
{
public:
    GameMessageHandler_Move(RTSCommandPool* commandPool,
                            GameObjectManager* gameObjectManager,
                            GameMapManager* gameMapManager)
        : command_pool_(commandPool), game_object_manager_(gameObjectManager), game_map_manager_(gameMapManager)
    {}

protected:
    bool canHandle(GameMessage* msg) const override
    {
        return msg->getType() == GameMessage::MessageType::REQUEST_CHASE ||
               msg->getType() == GameMessage::MessageType::REQUEST_RANDOMMOVE ||
               msg->getType() == GameMessage::MessageType::REQUEST_FINDENERMY;
    }
    ResultType process(GameMessage* msg) override;

private:
    ResultType processChase(MessageRequestChase* msg);
    ResultType processRandomMove(MessageRequestRandomMove* msg);
    ResultType processFindEnermy(MessageRequestFindEnermy* msg);

    RTSCommandPool* command_pool_           = nullptr;
    GameObjectManager* game_object_manager_ = nullptr;
    GameMapManager* game_map_manager_       = nullptr;
};