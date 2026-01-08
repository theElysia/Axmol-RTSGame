#include "GameObject/GameRole.h"
#include "GameObject/GameResourceHandler.h"

using namespace ax;
using CommandType = GameCommand::CommandType;

void GameRole::logState()

{
    int state = 0;
    if (dynamic_cast<GameRoleStateIdle*>(current_state_))
        state = 1;
    else if (dynamic_cast<GameRoleStateMove*>(current_state_))
        state = 2;
    else if (dynamic_cast<GameRoleStateFight*>(current_state_))
        state = 3;
    else if (dynamic_cast<GameRoleStateDead*>(current_state_))
        state = 4;
    AXLOGD("Game Role id {},  state {}, tick{}, target {}", this->unique_id_, state, this->state_tick_,
           this->target_obj_id_);
}

bool GameRole::init(GameObjectManager* manager,
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
    h_atk_.init(obj_template);
    h_select_.init(obj_template, "test");
    h_move_.init(obj_template);

    anim_idle_   = RepeatForever::create(obj_template.anim_action_["idle"]->clone());
    anim_run_    = RepeatForever::create(obj_template.anim_action_["run"]->clone());
    anim_attack_ = obj_template.anim_action_["attack"]->clone();
    anim_die_    = obj_template.anim_action_["death"]->clone();
    anim_idle_->retain();
    anim_run_->retain();
    anim_attack_->retain();
    anim_die_->retain();
    anim_idle_->setTag(ActionTag::ANIMATION);
    anim_run_->setTag(ActionTag::ANIMATION);
    anim_attack_->setTag(ActionTag::ANIMATION);
    anim_die_->setTag(ActionTag::ANIMATION);

    role_sprite_ = ax::Sprite::createWithSpriteFrame(obj_template.initial_sprite_frame_);
    // role_sprite_->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);
    role_sprite_->setAnchorPoint(Vec2(0.5f, 0.25f));

    this->addChild(role_sprite_, 1);
    this->addChild(h_hp_.getHpBar(), 2);
    auto select_tip = h_select_.getSelectTip();
    select_tip->setAnchorPoint(Vec2(0.5f, 0.25f));
    this->addChild(select_tip, 0);

    auto func1 = [=](AttributeHandler_FaceDirection d) -> void {
        if (d == AttributeHandler_FaceDirection::EAST)
            role_sprite_->setFlippedX(false);
        else if (d == AttributeHandler_FaceDirection::WEST)
            role_sprite_->setFlippedX(true);
    };

    h_atk_.bindSetFaceDirectionFunc(func1);
    h_move_.bindSetFaceDirectionFunc(func1);

    current_state_ = GameRoleStateIdle::getInstance();
    this->idleAction();

    return true;
}

GameRole::~GameRole()
{
    role_sprite_->stopAllActions();
    this->stopAllActions();
    anim_idle_->release();
    anim_run_->release();
    anim_attack_->release();
    anim_die_->release();
}

void GameRole::idleAction()
{
    this->stopActionByTag(ActionTag::MOVE);

    role_sprite_->stopActionByTag(ActionTag::ANIMATION);
    role_sprite_->runAction(anim_idle_);
}

void GameRole::dieAction()
{
    this->stopActionByTag(ActionTag::MOVE);

    role_sprite_->stopActionByTag(ActionTag::ANIMATION);

    auto seq = Sequence::create(this->anim_die_, CallFunc::create([=]() {
        role_sprite_->stopAllActions();
        this->stopAllActions();
        this->setReadyToRemove();
    }),
                                nullptr);

    role_sprite_->runAction(seq);
}

bool GameRole::isMoveActionValid(int moveStrategy)
{
    return current_move_strategy_ == -1 || moveStrategy <= current_move_strategy_;
}

void GameRole::moveAction(ax::Sequence* actionSeq, int moveStrategy)
{
    this->stopActionByTag(ActionTag::MOVE);
    current_move_strategy_ = moveStrategy;
    auto seq               = Sequence::create(actionSeq, CallFunc::create([=]() {
        this->idleAction();
        this->current_move_strategy_ = -1;
    }),
                                              nullptr);
    seq->setTag(ActionTag::MOVE);
    this->runAction(seq);

    role_sprite_->stopActionByTag(ActionTag::ANIMATION);
    role_sprite_->runAction(anim_run_);
}

void GameRole::attackAction(GameMessage* msg)
{
    this->stopActionByTag(ActionTag::MOVE);

    role_sprite_->stopActionByTag(ActionTag::ANIMATION);
    auto seq = Sequence::create(this->anim_attack_, CallFunc::create([=]() {
        this->sendMessage(msg);
        role_sprite_->runAction(anim_idle_);
    }),
                                nullptr);
    seq->setTag(ActionTag::ANIMATION);
    role_sprite_->runAction(seq);
}

void GameRole::hurtAction(float duration)
{
    if (is_hurting_)
        return;

    original_color_ = this->getColor();
    is_hurting_     = true;
    role_sprite_->stopActionByTag(static_cast<int>(ActionTag::HURT));

    auto tintRed     = TintTo::create(0.08f, 255, 50, 50);
    auto tintOrange  = TintTo::create(0.08f, 255, 150, 100);
    auto tintRecover = TintTo::create(duration - 0.16f, original_color_.r, original_color_.g, original_color_.b);

    auto seq = Sequence::create(tintRed, tintOrange, tintRecover, CallFunc::create([this]() { is_hurting_ = false; }),
                                nullptr);

    seq->setTag(static_cast<int>(ActionTag::HURT));
    role_sprite_->runAction(seq);
}

void GameRole::stopHurtAction()
{
    is_hurting_ = false;
    this->stopActionByTag(static_cast<int>(ActionTag::HURT));
    this->setColor(original_color_);
}

void GameRoleState::update(GameRole* role)
{
    // if (role->getStateTick() % 16 == 0)
    //     role->logState();
    role->h_hp_.updateHpBar();
    if (role->h_hp_.isHpZero())
    {
        role->setState(GameRoleStateDead::getInstance());
        role->dieAction();
    }
}

void GameRoleState::handleCommand(GameRole* role, GameCommand* cmd)
{

    switch (cmd->getType())
    {
    case CommandType::SELECT:
    {
        role->h_select_.setSelected();
    }
    break;
    case CommandType::DESELECT:
    {
        role->h_select_.setDeSelected();
    }
    break;
    case CommandType::SETTARGET:
    {
        auto command = static_cast<CommandSetTarget*>(cmd);
        auto data    = command->getData();
        auto tid     = data.new_target;
        role->setTargetObject(tid);
    }
    break;

    case CommandType::GET_ATTACK:
    {
        auto command = static_cast<CommandGetAttack*>(cmd);
        auto data    = command->getData();
        auto dmg     = data.damage;
        role->hurtAction();
        role->h_hp_.reduceHp(dmg);
    }
    break;

    default:
        break;
    }
}

void GameRoleStateIdle::update(GameRole* role)
{
    GameRoleState::update(role);
    if (role->isReadyToRemove())
        return;
    if (role->getStateTick() % GameRole::idle_enermy_alert_tick == 0)
    {
        auto target = role->getTargetObject();
        if (role->h_atk_.isTargetWithinAlertRange(role, target))
            role->setState(GameRoleStateFight::getInstance());
        else
        {
            auto msg = role->h_atk_.findEnermy(role);
            role->sendMessage(msg);
        }
    }
    // if (role->getCampType() == GameObject::CampType::PLAYER && role->getStateTick() % GameRole::idle_wander_tick ==
    // 0)
    // {
    //     auto msg = new MessageRequestRandomMove(role->getUniqueID());
    //     role->sendMessage(msg);
    // }
    if (role->getStateTick() % GameRole::idle_wander_tick == 0)
    {
        auto msg = new MessageRequestRandomMove(role->getUniqueID());
        role->sendMessage(msg);
    }
}

void GameRoleStateIdle::handleCommand(GameRole* role, GameCommand* cmd)
{
    GameRoleState::handleCommand(role, cmd);

    switch (cmd->getType())
    {
    case CommandType::GET_ATTACK:
    {
        auto command = static_cast<CommandGetAttack*>(cmd);
        auto data    = command->getData();
        auto sid     = data.source_id;
        role->setTargetObject(sid);

        role->setState(GameRoleStateFight::getInstance());
    }
    break;

    case CommandType::MOVE:
    {
        auto command  = static_cast<CommandMove*>(cmd);
        auto data     = command->getData();
        auto strategy = data.strategy;
        auto action   = role->h_move_.getMoveAction(role->getPosition(), data.path);
        if (role->isMoveActionValid(strategy))
        {
            ax::Sequence* t = ax::Sequence::create(
                action, CallFunc::create([=]() { role->setState(GameRoleStateIdle::getInstance()); }), nullptr);
            role->moveAction(t, strategy);
            role->setState(GameRoleStateMove::getInstance());
        }
    }
    break;

    default:
        break;
    }
}

void GameRoleStateMove::update(GameRole* role)
{
    GameRoleState::update(role);
    // if (role->isReadyToRemove())
    //     return;
    // if (role->h_move_.isMoveFinish())
    // {
    //     role->idleAction();
    //     role->setState(GameRoleStateIdle::getInstance());
    // }
}

void GameRoleStateMove::handleCommand(GameRole* role, GameCommand* cmd)
{
    GameRoleState::handleCommand(role, cmd);

    switch (cmd->getType())
    {
    case CommandType::GET_ATTACK:
    {
        auto command = static_cast<CommandGetAttack*>(cmd);
        auto data    = command->getData();
        auto sid     = data.source_id;
        if (role->getCurrentMoveStrategy() > 1)
        {
            role->setTargetObject(sid);
            role->setState(GameRoleStateFight::getInstance());
        }
    }
    break;

    case CommandType::MOVE:
    {
        auto command  = static_cast<CommandMove*>(cmd);
        auto data     = command->getData();
        auto strategy = data.strategy;
        auto action   = role->h_move_.getMoveAction(role->getPosition(), data.path);
        if (role->isMoveActionValid(strategy))
        {
            auto cur_state  = role->getCurrentState();
            ax::Sequence* t = ax::Sequence::create(
                action, CallFunc::create([=]() { role->setState(GameRoleStateIdle::getInstance()); }), nullptr);
            role->moveAction(t, strategy);
        }
    }
    break;

    default:
        break;
    }
}

void GameRoleStateFight::update(GameRole* role)
{
    GameRoleState::update(role);
    if (role->isReadyToRemove())
        return;
    auto target = role->getTargetObject();
    if (target == nullptr)
    {
        role->setState(GameRoleStateIdle::getInstance());
    }
    else if (role->h_atk_.isTargetWithinAtkRange(role, target))
    {
        if (role->h_atk_.isReadyToAttack(role->getStateTick()))
        {
            role->resetStateTick();
            auto msg = role->h_atk_.launchAttack(role, target);
            role->attackAction(msg);
        }
    }
    else if (role->getStateTick() % GameRole::fight_chase_step_tick == 0)
    {
        if (role->h_atk_.isTargetWithinAlertRange(role, target))
        {
            auto msg = new MessageRequestChase(role->getUniqueID(), role->getTargetId());
            role->sendMessage(msg);
        }
        else
        {
            role->setTargetObject(0);
            role->setState(GameRoleStateIdle::getInstance());
        }
    }
}

void GameRoleStateFight::handleCommand(GameRole* role, GameCommand* cmd)
{
    GameRoleState::handleCommand(role, cmd);

    switch (cmd->getType())
    {
    case CommandType::GET_ATTACK:
    {
        auto command = static_cast<CommandGetAttack*>(cmd);
        auto data    = command->getData();
        auto sid     = data.source_id;
        auto target  = role->getTargetObject();

        if (!role->h_atk_.isTargetWithinAtkRange(role, target))
            role->setTargetObject(sid);

        role->setState(GameRoleStateFight::getInstance());
    }
    break;

    case CommandType::MOVE:
    {
        auto command  = static_cast<CommandMove*>(cmd);
        auto data     = command->getData();
        auto strategy = data.strategy;
        auto action   = role->h_move_.getMoveAction(role->getPosition(), data.path);
        if (role->isMoveActionValid(strategy) && strategy <= 1)
        {
            auto cur_state  = role->getCurrentState();
            ax::Sequence* t = ax::Sequence::create(
                action, CallFunc::create([=]() { role->setState(GameRoleStateFight::getInstance()); }), nullptr);
            role->moveAction(t, strategy);
            role->setState(GameRoleStateMove::getInstance());
        }
    }
    break;

    default:
        break;
    }
}

void GameRoleStateDead::update(GameRole* role) {}

void GameRoleStateDead::handleCommand(GameRole* role, GameCommand* cmd) {}