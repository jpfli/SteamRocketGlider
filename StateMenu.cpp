
#include <cstdint>
#include <Pokitto.h>
#include <tasui>
#include "StateMenu.h"
#include "StatePlaying.h"
#include "Game.h"
#include "Scene.h"
#include "Airplane.h"

StateMenu StateMenu::_instance;

void StateMenu::enter() {
    _drawTopBar();
    _drawMenu();
}

void StateMenu::update() {
    static bool pressed = false;
    
    if(Pokitto::Buttons::pressed(BTN_A)) {
        pressed = true;
    }
    else if(pressed && Pokitto::Buttons::released(BTN_A)) {
        pressed = false;
        Game::setState(&StatePlaying::instance());
        return;
    }
    
    std::int32_t camera_x = Game::airplane().locationX() + 11;
    camera_x = (camera_x < 110) ? 0 : camera_x - 110;
    std::int32_t camera_y = Game::airplane().locationY() + 11;
    camera_y = (camera_y < 88) ? 0 : ((camera_y > POK_TILE_H * Scene::MAP_HEIGHT - 88) ? (camera_y = POK_TILE_H * Scene::MAP_HEIGHT - 176) : (camera_y - 88));
    
    Game::scene().update(camera_x, camera_y);
    Game::scene().draw();
    
    Game::airplane().draw(camera_x, camera_y);
}

void StateMenu::_drawTopBar() {
    using Pokitto::UI;
    
    UI::setCursor(0, 0);
    UI::printString("STEAM");
    UI::drawGauge(5, 10, 0, Game::airplane().steam(), 100);
    
    UI::setCursor(14, 0);
    UI::printString("DIST:");
    UI::printInteger(Game::airplane().distance());
    UI::printChar('m');
    
    UI::setCursor(26, 0);
    UI::printString("HI:");
    UI::printInteger(Game::getHighscore());
    UI::printChar('m');
}

void StateMenu::_drawMenu() {
    using Pokitto::UI;
    
    UI::drawBox(7, 4, 28, 13);
    UI::setCursor(9, 6);
    UI::printString("CONTROLS:");
    UI::setCursor(9, 8);
    UI::printString("A: Thrust");
    UI::setCursor(9, 9);
    UI::printString("LEFT, RIGHT: Turn");
    UI::setCursor(9, 11);
    UI::printString("Press A to Launch!");
}
