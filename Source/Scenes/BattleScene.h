#pragma once

#include "axmol.h"

class BattleScene : public ax::Scene
{
public:
    bool init() override;

    BattleScene();
    ~BattleScene() override;

private:
    ax::TMXTiledMap* _map = nullptr;
};
