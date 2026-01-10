#include "MenuScene.h"
#include "GameScene.h"

using namespace ax;

bool MenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    initMainMenu();
    initDeveloperDialog();

    this->addChild(main_menu_, 0, "Panelmain_menu_");
    this->addChild(developer_dialog_, 1, "Panel_DeveloperIntroduce");

    return true;
}

void MenuScene::initMainMenu()
{
    main_menu_ = Menu::create();
    main_menu_->setPosition(Vec2::ZERO);

    int pos_id      = 2;
    auto safeArea   = _director->getSafeAreaRect();
    auto safeSize   = safeArea.size;
    auto safeOrigin = safeArea.origin;

    // labels
    // Start Game
    auto label = MenuItemLabel::create(Label::createWithSystemFont("Start Game", "Arial", 24));
    label->setPosition(safeOrigin + Vec2(safeSize.width / 2, safeSize.height - (++pos_id) * 40));
    label->setCallback(AX_CALLBACK_1(MenuScene::onGameStart, this));
    main_menu_->addChild(label, 0, "Label_GameStart");

    // Open Developer Dialog
    label = MenuItemLabel::create(Label::createWithSystemFont("Developer Dialog", "Arial", 24));
    label->setPosition(safeOrigin + Vec2(safeSize.width / 2, safeSize.height - (++pos_id) * 40));
    label->setCallback(AX_CALLBACK_1(MenuScene::onDeveloperDialogOpen, this));
    main_menu_->addChild(label, 0, "Label_GameDeveloper");

    // Exit
    label = MenuItemLabel::create(Label::createWithSystemFont("Exit", "Arial", 24));
    label->setPosition(safeOrigin + Vec2(safeSize.width / 2, safeSize.height - (++pos_id) * 40));
    label->setCallback(AX_CALLBACK_1(MenuScene::onGameExit, this));
    main_menu_->addChild(label, 0, "Label_GameExit");

    main_menu_->setVisible(true);
}

void MenuScene::initDeveloperDialog()
{
    developer_dialog_ = ui::Layout::create();
    developer_dialog_->setPosition(Vec2::ZERO);

    auto safeArea   = _director->getSafeAreaRect();
    auto safeSize   = safeArea.size;
    auto safeOrigin = safeArea.origin;

    // Close Developer Dialog
    auto closeItem = MenuItemLabel::create(Label::createWithSystemFont("Close", "Arial", 24));
    closeItem->setPosition(safeOrigin + Vec2(safeSize.width / 2, 20));
    closeItem->setCallback(AX_CALLBACK_1(MenuScene::onDeveloperDialogClose, this));
    auto closemenu = Menu::create(closeItem, nullptr);
    closemenu->setPosition(Vec2::ZERO);
    developer_dialog_->addChild(closemenu, 1, "Labeldeveloper_dialog_Close");

    developer_dialog_->setVisible(false);
}

void MenuScene::onGameStart(ax::Object* sender)
{
    AXLOGD("Touch onGameStart");
    // _director->replaceScene(utils::createInstance<TestScene>());
    _director->replaceScene(utils::createInstance<GameScene>());
}

void MenuScene::onGameExit(ax::Object* sender)
{
    AXLOGD("Touch onGameExit");
    _director->end();
}

void MenuScene::onDeveloperDialogOpen(ax::Object* sender)
{
    AXLOGD("Touch onOpenDevelopersDialog");
    auto safeArea   = _director->getSafeAreaRect();
    auto safeSize   = safeArea.size;
    auto safeOrigin = safeArea.origin;

    auto t         = ax::random<int>(1, 5);
    std::string t1 = "ciallo (" + std::to_string(t) + ").png";

    auto fig1 = Sprite::create(t1);
    fig1->setPosition(safeOrigin + safeSize / 2);
    this->addChild(fig1, 1, 10);

    main_menu_->setVisible(false);
    developer_dialog_->setVisible(true);
}

void MenuScene::onDeveloperDialogClose(ax::Object* sender)
{
    AXLOGD("Touch onDeveloperDialogClose");
    this->removeChildByTag(10);

    main_menu_->setVisible(true);
    developer_dialog_->setVisible(false);
}
