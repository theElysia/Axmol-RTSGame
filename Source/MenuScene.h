#pragma once

#include "axmol.h"
#include "ui/axmol-ui.h"

class MenuScene : public ax::Scene
{
public:
    bool init() override;

    MenuScene() { AXLOGD("MenuScene Create"); }
    ~MenuScene() override { AXLOGD("MenuScene Destroy"); }

private:
    // void initSettingDialog();
    void initMainMenu();
    void initDeveloperDialog();

    void onGameStart(ax::Object* sender);
    void onGameExit(ax::Object* sender);
    void onDeveloperDialogOpen(ax::Object* sender);
    void onDeveloperDialogClose(ax::Object* sender);
    // void onDeveloperDialogClose(ax::Object* sender, ax::ui::Widget::TouchEventType type);

    // void onMusicVolumeSliderMoved(Object* sender, Slider::EventType sliderType);
    // void onEffectVolumeSliderMoved(Object* sender, Slider::EventType sliderType);

    ax::Menu* main_menu_              = nullptr;
    ax::ui::Layout* developer_dialog_ = nullptr;
};