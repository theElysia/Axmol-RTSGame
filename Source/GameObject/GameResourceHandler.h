#pragma once

#include "axmol.h"
#include "ui/axmol-ui.h"
#include "BaseTemplates.h"
#include "GameObject/GameObjectTemplate.h"

// Generate and Cache GameObject Templates
class GameResourceHandler
{
    DECLARE_SINGLETON(GameResourceHandler)

public:
    static constexpr float tick_time = 0.016f;

public:
    bool init(const std::string& animation_csv, const std::string& characters_csv);

    ax::Sprite* createHpBar(const std::string& type);
    ax::Sprite* createSelectTip(const std::string& type);

    GameObjectTemplate& getObjTemplate(const std::string& templateName);
    std::unordered_map<std::string, GameObjectTemplate>& getObjTemplatePool() { return obj_templates_; }

    void setTileSize(const ax::Vec2& tileSize);

private:
    bool readStats(const std::string& filename);
    bool readAnimation(const std::string& filename);

private:
    std::unordered_map<std::string, GameObjectTemplate> obj_templates_;
    GameObjectTemplate obj_null_template_;
};