#include "GameObject/AttributeHandler.h"
#include "GameObject/GameObject.h"
#include "GameObject/GameObjectTemplate.h"
#include "GameObject/GameResourceHandler.h"
#include "GameMessage/GameMessage.h"

void AttributeHandler_Hp::init(const GameObjectTemplate& objTemplate, const std::string& hpBarType)
{
    auto handler          = GameResourceHandler::getInstance();
    auto hpbar_background = handler->createHpBar(hpBarType);

    hp_bar_ = static_cast<ax::ui::LoadingBar*>(hpbar_background->getChildByTag(0));

    hp_ = hp_max_ = objTemplate.hp_max_;
}

ax::Node* AttributeHandler_Hp::getHpBar()
{
    return hp_bar_->getParent();
}

void AttributeHandler_Hp::showHpBar()
{
    auto hpbar_background = hp_bar_->getParent();
    hpbar_background->setVisible(true);
}

void AttributeHandler_Hp::hideHpBar()
{
    auto hpbar_background = hp_bar_->getParent();
    hpbar_background->setVisible(false);
}

void AttributeHandler_Hp::reduceHp(int reduceHpAmount)
{
    hp_     = std::max<int>(0, hp_ - reduceHpAmount);
    update_ = true;
}

void AttributeHandler_Hp::addHp(int addHpAmount)
{
    hp_     = std::min<int>(hp_max_, hp_ + addHpAmount);
    update_ = true;
}

void AttributeHandler_Hp::updateHpBar()
{
    if (update_)
    {
        float hp_percent = (float)hp_ * 100.0f / (float)hp_max_;
        hp_bar_->setPercent(hp_percent);
        update_ = false;
    }
}

void AttributeHandler_Atk::init(const GameObjectTemplate& objTemplate)
{
    atk_           = objTemplate.atk_;
    atk_range_2_   = objTemplate.atk_range_2_;
    atk_radius_2_  = objTemplate.atk_radius_2_;
    alert_range_2_ = objTemplate.alert_range_2_;
    atk_type_      = objTemplate.atk_type_;
    atk_interval_  = objTemplate.atk_interval_;
}

bool AttributeHandler_Atk::isReadyToAttack(int stateTick)
{
    if (stateTick < atk_interval_)
        return false;
    return true;
}

bool AttributeHandler_Atk::isTargetWithinAtkRange(GameObject* selfObj, GameObject* targetObj)
{
    if (targetObj == nullptr)
        return false;
    auto pos1  = selfObj->getPosition();
    auto pos2  = targetObj->getPosition();
    auto dist2 = pos1.distanceSquared(pos2);
    if (dist2 > atk_range_2_)
        return false;
    return true;
}

bool AttributeHandler_Atk::isTargetWithinAlertRange(GameObject* selfObj, GameObject* targetObj)
{
    if (targetObj == nullptr)
        return false;
    auto pos1  = selfObj->getPosition();
    auto pos2  = targetObj->getPosition();
    auto dist2 = pos1.distanceSquared(pos2);
    if (dist2 > alert_range_2_)
        return false;
    return true;
}

GameMessage* AttributeHandler_Atk::launchAttack(GameObject* selfObj, GameObject* targetObj)
{
    if (face_func_ != nullptr)
    {
        float x = targetObj->getPosition().x - selfObj->getPosition().y;
        if (x > 0)
            face_func_(AttributeHandler_FaceDirection::EAST);
        else if (x < 0)
            face_func_(AttributeHandler_FaceDirection::WEST);
    }

    auto sid = selfObj->getUniqueID();
    auto tid = targetObj->getUniqueID();
    if (atk_type_ == AttributeHandler_AtkType::SINGLE)
    {
        return new MessageRequestAttackSingle(sid, tid, atk_);
    }
    else if (atk_type_ == AttributeHandler_AtkType::AOE)
    {
        return new MessageRequestAttackAoe(sid, tid, atk_, atk_radius_2_);
    }
    AXLOGERROR("Wrong atk_type_ with {}", static_cast<int>(atk_type_));
    return nullptr;
}

GameMessage* AttributeHandler_Atk::findEnermy(GameObject* selfObj)
{
    return new MessageRequestFindEnermy(selfObj->getUniqueID(), alert_range_2_);
}

void AttributeHandler_Selectable::init(const GameObjectTemplate& objTemplate, const std::string& selectTipType)
{
    auto handler = GameResourceHandler::getInstance();
    select_tip_  = handler->createSelectTip(selectTipType);

    this->setDeSelected();
}

ax::Node* AttributeHandler_Selectable::getSelectTip()
{
    return select_tip_;
}

void AttributeHandler_Selectable::setSelected()
{
    is_selected_ = true;
    if (select_tip_)
        select_tip_->setVisible(true);
}

void AttributeHandler_Selectable::setDeSelected()
{
    is_selected_ = false;
    if (select_tip_)
        select_tip_->setVisible(false);
}

void AttributeHandler_Movable::init(const GameObjectTemplate& objTemplate)
{
    move_speed_ = objTemplate.move_speed_;
}

ax::Sequence* AttributeHandler_Movable::getMoveAction(const ax::Vec2& src, const std::vector<ax::Vec2>& path)
{
    ax::Vector<ax::FiniteTimeAction*> vec;
    ax::Vec2 pos = src;
    if (this->face_func_ != nullptr)
        vec.reserve(path.size() * 2);
    else
        vec.reserve(path.size());
    AttributeHandler_FaceDirection last_direction = AttributeHandler_FaceDirection::UNDEF;

    for (const auto& target_pos : path)
    {
        ax::Vec2 dpos = target_pos - pos;

        float time = dpos.getLength() / move_speed_;
        pos        = target_pos;

        if (this->face_func_ != nullptr)
            if (dpos.x > 0 && last_direction != AttributeHandler_FaceDirection::EAST)
            {
                auto turn = ax::CallFunc::create([=]() { this->face_func_(AttributeHandler_FaceDirection::EAST); });
                vec.pushBack(turn);
                last_direction = AttributeHandler_FaceDirection::EAST;
            }
            else if (dpos.x < 0 && last_direction != AttributeHandler_FaceDirection::WEST)
            {
                auto turn = ax::CallFunc::create([=]() { this->face_func_(AttributeHandler_FaceDirection::WEST); });
                vec.pushBack(turn);
                last_direction = AttributeHandler_FaceDirection::WEST;
            }

        vec.pushBack(ax::MoveTo::create(time, target_pos));
    }

    return ax::Sequence::create(vec);
}