#include "GameMessage/GameMessageHandler.h"
#include "GameMessage/RTSCommandPool.h"
#include "GameObject/GameObjectManager.h"
#include "GameMap/GameMapManager.h"
#include "GameObject/GameObject.h"

using ResultType  = GameMessageHandler::ResultType;
using MessageType = GameMessage::MessageType;
using CampType    = GameObject::CampType;

ResultType GameMessageHandler_Attack::process(GameMessage* msg)
{
    switch (msg->getType())
    {
    case MessageType::REQUEST_ATTACK_SINGLE:
        return processSingle(static_cast<MessageRequestAttackSingle*>(msg));
    case MessageType::REQUEST_ATTACK_AOE:
        return processAoe(static_cast<MessageRequestAttackAoe*>(msg));
    default:
        break;
    }
    return ResultType::REJECTED;
}

ResultType GameMessageHandler_Attack::processSingle(MessageRequestAttackSingle* msg)
{
    auto sid  = msg->getSenderId();
    auto sobj = game_object_manager_->getGameObjectBy(sid);
    auto tid  = msg->getData().target_id;
    auto tobj = game_object_manager_->getGameObjectBy(tid);
    if (!sobj || !tobj)
        return ResultType::REJECTED;
    command_pool_->addCommand<CommandGetAttack>(tid, sid, msg->getData().damage);
    return ResultType::CONSUMED;
}

ResultType GameMessageHandler_Attack::processAoe(MessageRequestAttackAoe* msg)
{
    auto sid  = msg->getSenderId();
    auto sobj = game_object_manager_->getGameObjectBy(sid);
    auto tid  = msg->getData().target_id;
    auto tobj = game_object_manager_->getGameObjectBy(tid);
    if (!sobj || !tobj)
        return ResultType::REJECTED;

    auto pool = game_object_manager_->getGameObjectPool();
    auto pos  = tobj->getPosition();
    auto r2   = msg->getData().radius2;

    CampType tcamp = CampType::UNDEF;
    if (sobj->getCampType() == CampType::PLAYER)
        tcamp = CampType::ENERMY1;
    else
        tcamp = CampType::PLAYER;
    for (auto& it : pool)
    {
        if (it.second->getCampType() == tcamp)
        {
            auto rr = pos.distanceSquared(it.second->getPosition());
            if (rr < r2)
            {
                command_pool_->addCommand<CommandGetAttack>(it.first, sid, msg->getData().damage);
            }
        }
    }

    return ResultType::CONSUMED;
}

ResultType GameMessageHandler_Move::process(GameMessage* msg)
{
    switch (msg->getType())
    {
    case MessageType::REQUEST_CHASE:
        return processChase(static_cast<MessageRequestChase*>(msg));
    case MessageType::REQUEST_RANDOMMOVE:
        return processRandomMove(static_cast<MessageRequestRandomMove*>(msg));
    case MessageType::REQUEST_FINDENERMY:
        return processFindEnermy(static_cast<MessageRequestFindEnermy*>(msg));
    default:
        break;
    }
    return ResultType::REJECTED;
}

ResultType GameMessageHandler_Move::processChase(MessageRequestChase* msg)
{
    auto sid  = msg->getSenderId();
    auto sobj = game_object_manager_->getGameObjectBy(sid);
    auto tid  = msg->getData().target_id;
    auto tobj = game_object_manager_->getGameObjectBy(tid);
    if (!sobj || !tobj)
        return ResultType::REJECTED;
    auto path = GameMapManager::getInstance()->getSinglePath(sobj->getPosition(), tobj->getPosition());
    if (path.has_value())
        command_pool_->addCommand<CommandMove>(sid, path.value(), 1);
    return ResultType::CONSUMED;
}

ResultType GameMessageHandler_Move::processRandomMove(MessageRequestRandomMove* msg)
{
    auto sid  = msg->getSenderId();
    auto sobj = game_object_manager_->getGameObjectBy(sid);
    AXLOGD("hand smove{}", sid);
    if (!sobj)
        return ResultType::REJECTED;
    std::vector<ax::Vec2> path;
    path.push_back(GameMapManager::getInstance()->getRandomNearbyIdleTilePos(sobj->getPosition()));
    command_pool_->addCommand<CommandMove>(sid, path, 3);
    AXLOGD("hand smove{}  {}  {}", sid, path[0].x, path[0].y);
    return ResultType::CONSUMED;
}

ResultType GameMessageHandler_Move::processFindEnermy(MessageRequestFindEnermy* msg)
{
    auto sid  = msg->getSenderId();
    auto sobj = game_object_manager_->getGameObjectBy(sid);
    if (!sobj)
        return ResultType::REJECTED;
    auto pool = game_object_manager_->getGameObjectPool();
    auto pos  = sobj->getPosition();
    auto r2   = msg->getData().radius2;

    CampType tcamp = CampType::UNDEF;
    if (sobj->getCampType() == CampType::PLAYER)
        tcamp = CampType::ENERMY1;
    else
        tcamp = CampType::PLAYER;
    for (auto& it : pool)
    {
        if (it.second->getCampType() == tcamp)
        {
            auto rr = pos.distanceSquared(it.second->getPosition());
            if (rr < r2)
            {
                command_pool_->addCommand<CommandSetTarget>(sid, it.first);
                return ResultType::CONSUMED;
            }
        }
    }

    return ResultType::CONSUMED;
}