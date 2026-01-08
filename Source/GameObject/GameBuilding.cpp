#include "GameObject/GameBuilding.h"
#include "GameObject/GameResourceHandler.h"

using namespace ax;
using CommandType = GameCommand::CommandType;

bool GameBuilding::init(GameObjectManager* manager,
                        GameObjectType gameObjectType,
                        CampType campType,
                        const std::string& templateName,
                        const ax::Vec2& position,
                        int uniqueID)
{
    if (!GameObject::init(manager, gameObjectType, campType, templateName, position, uniqueID))
    {
        return false;
    }

    auto& obj_template = GameResourceHandler::getInstance()->getObjTemplate(templateName);

    if (campType == CampType::PLAYER)
        h_hp_.init(obj_template, "green");
    else
        h_hp_.init(obj_template, "red");

    h_hp_.getHpBar()->setScale(0.3f);

    this->addChild(h_hp_.getHpBar(), 1);

    building_sprite_ = ax::Sprite::createWithSpriteFrame(obj_template.initial_sprite_frame_);
    building_sprite_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
    building_sprite_->setScale(0.1f);
    this->addChild(building_sprite_, 0);

    return true;
}

void GameBuilding::update(float delta)
{
    if (this->isReadyToRemove())
        return;
    this->h_hp_.updateHpBar();

    if (this->h_hp_.isHpZero())
    {
        this->setReadyToRemove();
    }
}

void GameBuilding::handleCommand(GameCommand* cmd)
{
    if (this->isReadyToRemove())
        return;
    switch (cmd->getType())
    {
    case CommandType::GET_ATTACK:
    {
        auto command = static_cast<CommandGetAttack*>(cmd);
        auto data    = command->getData();
        auto dmg     = data.damage;
        this->h_hp_.reduceHp(dmg);
    }
    break;

    default:
        break;
    }
}