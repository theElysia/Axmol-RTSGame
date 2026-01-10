#include "GameUI.h"
#include "GameWorld.h"
#include <unordered_map>
#include <vector>

using namespace ax;

static constexpr int token_increase_interval = 20;

static std::unordered_map<std::string, int> gameobject_cost = {std::pair("Knight", 5), std::pair("Wizard", 20),
                                                               std::pair("Ghost", 12), std::pair("Skeleton", 8)};

bool GameUI::init(GameWorld* game_world)
{
    if (!Node::init())
    {
        return false;
    }

    game_world_ = game_world;

    const float x1 = 5.0f;
    auto btn1      = initButton("Knight_btn.png", "Knight", Vec2(x1, 220));
    auto btn2      = initButton("Wizard_btn.png", "Wizard", Vec2(x1, 160));
    auto btn3      = initButton("Ghost_btn.png", "Ghost", Vec2(x1, 100));
    auto btn4      = initButton("Skeleton_btn.png", "Skeleton", Vec2(x1, 40));

    auto lb1 = Label::createWithSystemFont(std::to_string(gameobject_cost["Knight"]), "Arial", 24);
    auto lb2 = Label::createWithSystemFont(std::to_string(gameobject_cost["Wizard"]), "Arial", 24);
    auto lb3 = Label::createWithSystemFont(std::to_string(gameobject_cost["Ghost"]), "Arial", 24);
    auto lb4 = Label::createWithSystemFont(std::to_string(gameobject_cost["Skeleton"]), "Arial", 24);
    lb1->setPosition(Vec2(x1 + 50, 240));
    lb2->setPosition(Vec2(x1 + 50, 180));
    lb3->setPosition(Vec2(x1 + 50, 120));
    lb4->setPosition(Vec2(x1 + 50, 60));
    lb1->setColor(Color3B::ORANGE);
    lb2->setColor(Color3B::ORANGE);
    lb3->setColor(Color3B::ORANGE);
    lb4->setColor(Color3B::ORANGE);

    this->addChild(btn1, 0);
    this->addChild(btn2, 0);
    this->addChild(btn3, 0);
    this->addChild(btn4, 0);
    this->addChild(lb1, 0);
    this->addChild(lb2, 0);
    this->addChild(lb3, 0);
    this->addChild(lb4, 0);

    token_label_ = Label::createWithSystemFont("My Label Text", "Arial", 16);
    token_label_->setString(std::to_string(player_token_));
    token_label_->setAnchorPoint(Vec2::ZERO);
    token_label_->setPosition(Vec2(x1, 280));
    this->addChild(token_label_);

    return true;
}

void GameUI::update(float delta)
{
    tick_count_++;
    token_label_->setString(std::to_string(player_token_));
    if (tick_count_ % token_increase_interval == 0)
        player_token_++;
}

ax::ui::Button* GameUI::initButton(const std::string& btnFile, const std::string& templateName, const ax::Vec2& pos)
{
    auto btn = ui::Button::create(btnFile);
    btn->setAnchorPoint(Vec2::ZERO);
    btn->setPosition(pos);

    btn->addTouchEventListener([=](Object* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::BEGAN:
            break;
        case ui::Widget::TouchEventType::ENDED:
            if (player_token_ >= gameobject_cost[templateName])
            {
                player_token_ -= gameobject_cost[templateName];
                this->game_world_->deployGameRole(GameObject::CampType::PLAYER, templateName);
            }
            // AXLOGD("Button 1 clicked");
            break;
        default:
            break;
        }
    });

    return btn;
}
