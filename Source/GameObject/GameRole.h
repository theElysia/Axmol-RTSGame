#pragma once

#include "axmol.h"
#include "BaseTemplates.h"
#include "GameObject/GameObject.h"
#include "GameObject/AttributeHandler.h"
#include "GameObject/GameObjectManager.h"

class GameRole;
// State Pattern & Singleton & CRTP
class GameRoleState
{
public:
    virtual ~GameRoleState() = default;

    virtual void update(GameRole* role);

    virtual void handleCommand(GameRole* role, GameCommand* cmd);
};

class GameRole : public GameObject
{
public:
    enum class StateType
    {
        IDLE,
        MOVE,
        FIGHT,
        DEAD,
    };

    enum ActionTag
    {
        MOVE,
        ANIMATION,
        HURT,
    };

    static constexpr int idle_wander_tick       = 300;
    static constexpr int idle_enermy_alert_tick = 32;
    static constexpr int fight_chase_step_tick  = 64;
    static constexpr float hurt_duration        = 0.5f;

public:
    GameRole() {};
    ~GameRole() override;

    bool init(GameObjectManager* manager,
              GameObjectType gameObjectType,
              CampType campType,
              const std::string& templateName,
              const ax::Vec2& position,
              int uniqueID) override;

    void setState(GameRoleState* newState) { current_state_ = newState; }
    GameRoleState* getCurrentState() { return current_state_; }

    void update(float delta) override
    {
        current_state_->update(this);
        state_tick_++;
    }
    void handleCommand(GameCommand* cmd) override { current_state_->handleCommand(this, cmd); }

    GameObject* getTargetObject() { return game_object_manager_->getGameObjectBy(target_obj_id_); }
    int getTargetId() { return target_obj_id_; }
    void setTargetObject(int objId) { target_obj_id_ = objId; }

    void idleAction();
    void dieAction();
    void moveAction(ax::Sequence* actionSeq, int moveStrategy = 0);
    bool isMoveActionValid(int moveStrategy);
    void attackAction(GameMessage* msg);
    void hurtAction(float duration = hurt_duration);
    void stopHurtAction();

    void resetStateTick() { state_tick_ = 0; }
    int getStateTick() { return state_tick_; }

    void logState();

    int getCurrentMoveStrategy() { return current_move_strategy_; }

public:
    // 业务逻辑
    AttributeHandler_Hp h_hp_;
    AttributeHandler_Atk h_atk_;
    AttributeHandler_Selectable h_select_;
    AttributeHandler_Movable h_move_;

public:
    // 动画
    ax::FiniteTimeAction* anim_idle_   = nullptr;  // repeat
    ax::FiniteTimeAction* anim_run_    = nullptr;  // repeat
    ax::FiniteTimeAction* anim_attack_ = nullptr;  // finite
    ax::FiniteTimeAction* anim_die_    = nullptr;  // repeat
    // ax::FiniteTimeAction* anim_hurt_   = nullptr; //受伤只变红

private:
    GameRoleState* current_state_ = nullptr;
    ax::Sprite* role_sprite_      = nullptr;

    int target_obj_id_ = 0;
    int state_tick_    = 0;

    ax::Color3B original_color_;
    bool is_hurting_ = false;

    int current_move_strategy_ = -1;
};

class GameRoleStateIdle : public GameRoleState
{
    DECLARE_SINGLETON(GameRoleStateIdle)
public:
    void update(GameRole* role) override;
    void handleCommand(GameRole* role, GameCommand* cmd) override;
};

// 快速声明状态类
#define DECLARE_GAME_ROLE_STATE(StateName)                             \
    class StateName : public GameRoleState                             \
    {                                                                  \
        DECLARE_SINGLETON(StateName)                                   \
    public:                                                            \
        void update(GameRole* role) override;                          \
        void handleCommand(GameRole* role, GameCommand* cmd) override; \
    };

// DECLARE_GAME_ROLE_STATE(Idle)
DECLARE_GAME_ROLE_STATE(GameRoleStateMove)
DECLARE_GAME_ROLE_STATE(GameRoleStateFight)
DECLARE_GAME_ROLE_STATE(GameRoleStateDead)

#undef DECLARE_GAME_ROLE_STATE