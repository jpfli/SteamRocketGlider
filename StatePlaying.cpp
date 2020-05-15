
#include <Pokitto.h>
#include <tasui>
#include "StatePlaying.h"
#include "StateMenu.h"
#include "Game.h"
#include "Scene.h"
#include "Airplane.h"

StatePlaying StatePlaying::_instance;

void StatePlaying::enter() {
    using Pokitto::UI;
    
    Game::scene().reset(Pokitto::Core::getTime());
    Game::airplane().reset();
    
    _drawTopBar();
}

void StatePlaying::update() {
    using Pokitto::Buttons;
    using Pokitto::UI;
    
    _drawTopBar();
    
    if(Game::airplane().landed()) {
        Game::airplane().throttleControl(0);
        
        // Update highscore
        if(Game::airplane().distance() > Game::getHighscore()) Game::setHighscore(Game::airplane().distance());
        
        Game::setState(&StateMenu::instance());
        return;
    }
    
    Game::airplane().pitchControl(Buttons::leftBtn() - Buttons::rightBtn());
    Game::airplane().throttleControl(Buttons::aBtn());
    Game::airplane().update(Game::scene());
    
    std::int32_t camera_x = Game::airplane().locationX() + 11;
    camera_x = (camera_x < 110) ? 0 : camera_x - 110;
    std::int32_t camera_y = Game::airplane().locationY() + 11;
    camera_y = (camera_y < 88) ? 0 : ((camera_y > POK_TILE_H * Scene::MAP_HEIGHT - 88) ? (camera_y = POK_TILE_H * Scene::MAP_HEIGHT - 176) : (camera_y - 88));
    
    Game::scene().update(camera_x, camera_y);
    Game::scene().draw();
    
    Game::airplane().draw(camera_x, camera_y);
}

void StatePlaying::_drawTopBar() {
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
