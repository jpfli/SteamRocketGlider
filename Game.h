
#pragma once

#include <cstdint>
#include <Pokitto.h>
#include <miloslav.h>
#include <tasui>
#include <puits_UltimateUtopia.h>   // Tileset for TASUI
#include "State.h"
#include "Scene.h"
#include "Airplane.h"

class Game {
    
    public:
    
        Game() = delete;
        Game(const Game&) = delete;
        Game& operator =(const Game&) = delete;
        
        static inline void init() {
            using Pokitto::UI;
            
            Pokitto::Core::begin();
            Pokitto::Display::loadRGBPalette(miloslav);
            
            // Select TASUI tileset
            UI::setTilesetImage(puits::UltimateUtopia::tileSet);
            // Show the Tilemap, the Sprites, then the UI.
            UI::showTileMapSpritesUI();
            // Align tiles so that they are horizontally and vertically centered
            UI::setOffset(2, 1);
        }
        
        static inline void run(State* state) {
            _next_state = state;
            
            while(true) {
                if(_next_state != _state ) {
                    if(_state != nullptr)
                        _state->leave();
                    _state = _next_state;
                    _state->enter();
                }
                
                while(!Pokitto::Core::update()) {}
                
                _state->update();
            }
        }
        
        static inline State* getState() { return _state; }
        static constexpr void setState(State* state) { _next_state = state; }
        
        static constexpr Scene& scene() { return _scene; }
        static constexpr Airplane& airplane() { return _airplane; }
        
        static inline std::uint32_t getHighscore() { return _highscore; }
        static constexpr void setHighscore(std::uint32_t score) { _highscore = score; }
    
    private:
    
        static inline State* _state = nullptr;
        static inline State* _next_state = nullptr;
        
        static inline Scene _scene;
        static inline Airplane _airplane;
        static inline std::uint32_t _highscore;
};
