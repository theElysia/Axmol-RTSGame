#pragma once

#include "axmol.h"
#include "ui/axmol-ui.h"
#include <string>

class GameWorld;

class GameUI : public ax::Node
{
public:
    bool init(GameWorld* game_world);

    GameUI() { AXLOGD("GameUI Create"); }
    ~GameUI() override {}

    void update(float delta) override;

private:
    ax::ui::Button* initButton(const std::string& btnFile, const std::string& templateName, const ax::Vec2& pos);

    GameWorld* game_world_  = nullptr;
    ax::Label* token_label_ = nullptr;
    int player_token_       = 0;
    int tick_count_         = 0;
};