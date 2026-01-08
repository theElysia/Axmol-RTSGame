#pragma once

#include "axmol.h"
#include "GameMessage/GameCommand.h"
#include "GameMessage/GameMessage.h"

class GameObjectManager;

class GameObject : public ax::Sprite
{
public:
    enum class GameObjectType
    {
        UNDEF,

        ROLE,
        BUILDING,
    };

    enum class CampType
    {
        UNDEF,

        PLAYER,
        ENERMY1,

        TOTAL,
    };

public:
    virtual bool init(GameObjectManager* manager,
                      GameObjectType gameObjectType,
                      CampType campType,
                      const std::string& templateName,
                      const ax::Vec2& position,
                      int uniqueID);

    virtual void handleCommand(GameCommand* cmd) = 0;

    GameObject() { AXLOGD("GameObject Create"); }
    virtual ~GameObject() override { AXLOGD("GameObject Destroy"); }

    int getUniqueID() { return unique_id_; }
    GameObjectType getGameObjectType() { return game_object_type_; }
    CampType getCampType() { return camp_type_; }
    const std::string& getTemplateName() { return template_name_; }

    bool isReadyToRemove() { return is_ready_to_remove_; }
    void setReadyToRemove();

    void sendMessage(GameMessage* msg);

protected:
    int unique_id_                          = 0;
    GameObjectType game_object_type_        = GameObjectType::UNDEF;
    CampType camp_type_                     = CampType::UNDEF;
    GameObjectManager* game_object_manager_ = nullptr;
    bool is_ready_to_remove_                = false;
    std::string template_name_;
};
