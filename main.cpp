
#include "Game.h"
#include "StateMenu.h"

int main() {
    Game::init();
    Game::run(&StateMenu::instance());
    
    return 0;
}
