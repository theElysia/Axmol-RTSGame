#include "GameWorld.h"

using namespace ax;

bool GameWorld::init()
{
    if (!Node::init())
    {
        return false;
    }

    game_map_manager_ = new GameMapManager;
    auto map_sprite   = game_map_manager_->init(this, "map3.tmx");
    this->addChild(map_sprite, 0, 0);

    game_resource_ = GameResourceHandler::getInstance();
    game_resource_->init("animations_config.csv", "characters_stats.csv");
    game_resource_->setTileSize(game_map_manager_->getTileSize());

    command_pool_ = new RTSCommandPool;

    game_object_layer_   = ax::Node::create();
    game_object_manager_ = new GameObjectManager;
    game_object_manager_->init(this, game_object_layer_, command_pool_, game_map_manager_);
    this->addChild(game_object_layer_, 1, 1);

    auto msg_handler1 =
        std::make_shared<GameMessageHandler_Attack>(command_pool_, game_object_manager_, game_map_manager_);
    auto msg_handler2 =
        std::make_shared<GameMessageHandler_Move>(command_pool_, game_object_manager_, game_map_manager_);
    msg_handler1->setNext(msg_handler2);
    game_message_handler_ = msg_handler1;

    game_state_ = GameStateType::INITIAL;

    return true;
}

void GameWorld::setGameEnded(GameResultType result)
{
    game_state_  = GameStateType::END;
    game_result_ = result;
}

void GameWorld::update(float delta)
{
    static GameStateType lass_ = GameStateType::PAUSE;
    if (game_state_ != lass_)
    {
        AXLOGD("GameState {}", static_cast<int>(game_state_));
        lass_ = game_state_;
    }
    if (game_state_ == GameStateType::RUNNING)
    {
        game_object_manager_->update(delta);
        command_pool_->processPreviousFrame(game_object_manager_);
        command_pool_->swapBuffers();
    }
}

void GameWorld::selectObject(const ax::Vec2& cursorPoint1,
                             const ax::Vec2& cursorPoint2,
                             GameObject::GameObjectType gameObjectType,
                             GameObject::CampType campType)
{
    game_object_manager_->selectWithRect(cursorPoint1, cursorPoint2, gameObjectType, campType);
}

void GameWorld::selectAll(GameObject::GameObjectType gameObjectType, GameObject::CampType campType)
{
    game_object_manager_->selectAll(gameObjectType, campType);
}

void GameWorld::cancelSelect()
{
    game_object_manager_->cancelAllSelected();
}

void GameWorld::moveSelectedObjTo(const ax::Vec2& cursorPoint)
{
    game_object_manager_->moveSelectedObjTo(cursorPoint);
}

void GameWorld::safeSwitchPauseRun()
{
    if (game_state_ == GameStateType::PAUSE)
        game_state_ = GameStateType::RUNNING;
    else if (game_state_ == GameStateType::RUNNING)
        game_state_ = GameStateType::PAUSE;
}

void GameWorld::setupBaseCamp(GameObject::CampType campType, const std::string& templateName, const ax::Vec2& position)
{
    auto obj =
        game_object_manager_->createGameObject(GameObject::GameObjectType::BUILDING, campType, templateName, position);
    game_object_manager_->setBaseCamp(obj, campType);
}

void GameWorld::deployGameRole(GameObject::CampType campType, const std::string& templateName)
{
    Vec2 pos = game_object_manager_->getBaseCampPos(campType);
    game_object_manager_->createGameObject(GameObject::GameObjectType::ROLE, campType, templateName, pos);
}

void GameWorld::letAllRoleRushTo(GameObject::CampType campType, const Vec2& pos)
{
    auto pool = game_object_manager_->getGameObjectPool();
    game_map_manager_->setDestForMassivePath(pos);
    for (auto& it : pool)
    {
        auto obj = it.second;
        if (obj->getCampType() == campType && obj->getGameObjectType() == GameObject::GameObjectType::ROLE)
        {
            auto path = game_map_manager_->getMassivePath(obj->getPosition());
            if (path.has_value())
                command_pool_->addCommand<CommandMove>(it.first, path.value(), 2);
        }
    }
}

void GameWorld::letAllEnermyRush()
{
    auto pos = game_object_manager_->getBaseCampPos(GameObject::CampType::PLAYER);
    letAllRoleRushTo(GameObject::CampType::ENERMY1, pos);
}

void GameWorld::handleMessage(GameMessage* msg)
{
    if (game_message_handler_)
    {
        // AXLOGD("process msg {}  from {}", static_cast<int>(msg->getType()), msg->getSenderId());
        game_message_handler_->handle(msg);
    }
    delete msg;
}