#pragma once

#include "axmol.h"
#include "GameObject/AttributeHandler.h"

class GameObjectTemplate
{
public:
    std::string template_name_;

    int hp_max_             = 0;
    int atk_                = 0;
    int atk_interval_       = 0;  // tick
    float atk_range_tile_   = 0;  // tile
    float atk_radius_tile_  = 0;  // tile
    float alert_range_tile_ = 0;  // tile
    float move_speed_tile_  = 0;  // tiles per second
    float atk_range_2_      = 0;  // pixel^2
    float atk_radius_2_     = 0;  // pixel^2
    float alert_range_2_    = 0;  // pixel^2
    float move_speed_       = 0;  // pixels per second

    AttributeHandler_AtkType atk_type_ = AttributeHandler_AtkType::SINGLE;

    std::string anim_plist_str_;

    struct animInfoType
    {
        std::string str;
        int frames;
        float duration;
        animInfoType(std::string s, int frame, float d) : str(s), frames(frame), duration(d) {}
    };

    //"idle","run","attack","death"
    std::unordered_map<std::string, ax::Animate*> anim_action_;
    std::unordered_map<std::string, animInfoType> anim_info_;

    ax::SpriteFrame* initial_sprite_frame_ = nullptr;

    ~GameObjectTemplate()
    {
        for (auto& it : anim_action_)
        {
            it.second->release();
        }
        if (initial_sprite_frame_)
            initial_sprite_frame_->release();
    }
};