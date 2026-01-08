#pragma once

#include "axmol.h"
#include "ui/axmol-ui.h"
#include <functional>

class GameObject;
class GameObjectTemplate;
class GameMessage;

// 业务逻辑类

enum class AttributeHandler_AtkType
{
    SINGLE,
    AOE,
    DOT,
};

enum class AttributeHandler_FaceDirection
{
    UNDEF,
    EAST,
    WEST,
};

using SetFaceDirectionFunc = std::function<void(AttributeHandler_FaceDirection)>;

class AttributeHandler_Hp
{
public:
    void init(const GameObjectTemplate& objTemplate, const std::string& hpBarType);
    void showHpBar();
    void hideHpBar();
    void reduceHp(int reduceHpAmount);
    void addHp(int addHpAmount);
    void updateHpBar();
    bool isHpZero() { return hp_ == 0; }
    ax::Node* getHpBar();

protected:
    int hp_                     = 0;
    int hp_max_                 = 0;
    bool update_                = false;
    ax::ui::LoadingBar* hp_bar_ = nullptr;
};

class AttributeHandler_Atk
{
public:
    void init(const GameObjectTemplate& objTemplate);
    bool isReadyToAttack(int stateTick);
    bool isTargetWithinAtkRange(GameObject* selfObj, GameObject* targetObj);
    bool isTargetWithinAlertRange(GameObject* selfObj, GameObject* targetObj);
    GameMessage* launchAttack(GameObject* selfObj, GameObject* targetObj);
    GameMessage* findEnermy(GameObject* selfObj);
    void bindSetFaceDirectionFunc(SetFaceDirectionFunc func) { face_func_ = func; }

protected:
    int atk_             = 0;
    float atk_range_2_   = 0;  // pixel^2
    float atk_radius_2_  = 0;  // pixel^2
    float alert_range_2_ = 0;  // pixel^2
    int atk_interval_    = 0;  // tick

    SetFaceDirectionFunc face_func_ = nullptr;

    AttributeHandler_AtkType atk_type_ = AttributeHandler_AtkType::SINGLE;
};

class AttributeHandler_Selectable
{
public:
    void init(const GameObjectTemplate& objTemplate, const std::string& selectTipType);
    void setSelected();
    void setDeSelected();
    ax::Node* getSelectTip();

protected:
    bool is_selected_ = false;

    ax::Sprite* select_tip_ = nullptr;
};

class AttributeHandler_Movable
{
public:
    void init(const GameObjectTemplate& objTemplate);
    ax::Sequence* getMoveAction(const ax::Vec2& src, const std::vector<ax::Vec2>& path);
    void bindSetFaceDirectionFunc(SetFaceDirectionFunc func) { face_func_ = func; }

protected:
    float move_speed_ = 0;

    SetFaceDirectionFunc face_func_ = nullptr;
};