#include "GameObject/GameResourceHandler.h"
#include <fstream>
#include <sstream>

using namespace ax;

bool GameResourceHandler::init(const std::string& animation_csv, const std::string& characters_csv)
{
    obj_templates_.clear();
    auto fileUtils      = FileUtils::getInstance();
    std::string csvPath = fileUtils->fullPathForFilename(characters_csv);
    AXLOG(("try open " + csvPath).c_str());
    if (!readStats(csvPath))
        return false;
    csvPath = fileUtils->fullPathForFilename(animation_csv);
    AXLOG(("try open " + csvPath).c_str());
    if (!readAnimation(csvPath))
        return false;
    AXLOG("finish init GameResourceHandler");
    for (auto& it : obj_templates_)
    {
        AXLOG((std::string("init template ") + it.first).c_str());
    }
    return true;
}

bool GameResourceHandler::readStats(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open())
    {
        AXLOGERROR(("Cannot open file: " + filename).c_str());
        return false;
    }
    else
    {
        AXLOGD("Open file {}", filename);
    }
    std::string line, _word;
    std::vector<std::string> words;
    std::istringstream sline;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        words.clear();
        sline.clear();
        sline.str(line);
        while (std::getline(sline, _word, ','))
        {
            words.push_back(_word);
        }
        auto it = obj_templates_.find(words[0]);
        if (it == obj_templates_.end())
        {
            obj_templates_.insert(std::pair<std::string, GameObjectTemplate>(words[0], GameObjectTemplate()));
            obj_templates_[words[0]].anim_plist_str_ = words[0] + ".plist";
        }
        auto& objT           = obj_templates_[words[0]];
        objT.hp_max_         = std::stoi(words[1]);
        objT.atk_            = std::stoi(words[2]);
        objT.atk_range_tile_ = std::stof(words[3]);
        objT.atk_interval_   = std::stoi(words[4]);
        if (words[5] == "AOE")
        {
            objT.atk_type_ = AttributeHandler_AtkType::AOE;
        }
        else
        {
            objT.atk_type_ = AttributeHandler_AtkType::SINGLE;
        }
        objT.atk_radius_tile_  = std::stof(words[6]);
        objT.alert_range_tile_ = std::stof(words[7]);
        objT.move_speed_tile_  = std::stof(words[8]);
    }
    return true;
}

bool GameResourceHandler::readAnimation(const std::string& filename)
{
    auto spritecache = SpriteFrameCache::getInstance();
    for (auto& it : obj_templates_)
    {
        spritecache->addSpriteFramesWithFile(it.second.anim_plist_str_);
    }

    std::ifstream file(filename, std::ios::in);
    if (!file.is_open())
    {
        AXLOGERROR(("Cannot open file: " + filename).c_str());
        return false;
    }
    else
    {
        AXLOGD("Open file {}", filename);
    }
    std::string line, _word;
    std::vector<std::string> words;
    std::istringstream sline;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        words.clear();
        sline.clear();
        sline.str(line);
        while (std::getline(sline, _word, ','))
        {
            words.push_back(_word);
        }
        auto it = obj_templates_.find(words[0]);
        if (it == obj_templates_.end())
        {
            AXLOG(("Unknown template name: " + words[0]).c_str());
            continue;
        }
        auto& objT = obj_templates_[words[0]];

        auto& state    = words[1];
        float duration = tick_time * std::stoi(words[4]);

        ax::Vector<ax::SpriteFrame*> anim_frames;
        int t = std::stoi(words[3]) + 1;
        anim_frames.reserve(t);
        for (int i = 0; i < t; i++)
        {
            std::string str = words[2] + "/" + std::to_string(i) + ".png";
            anim_frames.pushBack(spritecache->getSpriteFrameByName(str));
        }

        ax::Animate* action = Animate::create(Animation::createWithSpriteFrames(anim_frames, duration / t));
        action->retain();
        objT.anim_action_[state] = action;

        objT.anim_info_.insert(std::pair<std::string, GameObjectTemplate::animInfoType>(
            words[1], GameObjectTemplate::animInfoType(words[2], t, duration)));

        if (state == "idle")
        {
            objT.initial_sprite_frame_ = anim_frames[0];
            objT.initial_sprite_frame_->retain();
        }
    }
    return true;
}

ax::Sprite* GameResourceHandler::createHpBar(const std::string& type)
{
    ax::ui::LoadingBar* hpbar = nullptr;
    if (type == "green")
    {
        hpbar = ui::LoadingBar::create("HPBarGreen.png");
    }
    else
    {
        hpbar = ui::LoadingBar::create("HPBarRed.png");
    }

    hpbar->setAnchorPoint(Vec2::ZERO);
    hpbar->setPercent(100.0f);

    auto hpbar_background = Sprite::create("HPBarBackground.png");
    hpbar_background->setCascadeOpacityEnabled(true);
    hpbar_background->addChild(hpbar, 0, 0);
    hpbar_background->setScale(0.2f);
    hpbar_background->setVisible(true);
    hpbar_background->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);

    return hpbar_background;
}

ax::Sprite* GameResourceHandler::createSelectTip(const std::string& type)
{
    auto tip = Sprite::create("SelectedTip.png");
    tip->setVisible(true);
    tip->setAnchorPoint(Vec2::ANCHOR_MIDDLE_BOTTOM);

    return tip;
}

GameObjectTemplate& GameResourceHandler::getObjTemplate(const std::string& templateName)
{
    auto it = obj_templates_.find(templateName);
    if (it != obj_templates_.end())
    {
        return it->second;
    }
    else
    {
        AXLOGERROR("GameResourceHandler unknown template name: {}", templateName);
        return obj_null_template_;
    }
}

void GameResourceHandler::setTileSize(const ax::Vec2& tileSize)
{
    float s = tileSize.x * tileSize.y;
    for (auto& it : obj_templates_)
    {
        it.second.atk_radius_2_  = it.second.atk_radius_tile_ * it.second.atk_radius_tile_ * s;
        it.second.atk_range_2_   = it.second.atk_range_tile_ * it.second.atk_range_tile_ * s;
        it.second.alert_range_2_ = it.second.alert_range_tile_ * it.second.alert_range_tile_ * s;
        it.second.move_speed_    = it.second.move_speed_tile_ * tileSize.x;
    }
}