
#pragma once

#include <cstdint>
#include <Pokitto.h>
#include <tasui>
#include "State.h"

class StateMenu final: public State {
    
    public:
    
        StateMenu(const StateMenu&) = delete;
        StateMenu& operator =(const StateMenu&) = delete;
        
        static constexpr StateMenu& instance() { return _instance; }
        
        inline void init() override {}
        
        void enter() override;
        
        inline void leave() override { Pokitto::UI::clear(); }
        
        void update() override;
    
    private:
    
        StateMenu() = default;
        virtual ~StateMenu() = default;
        
        void _drawTopBar();
        
        void _drawMenu();
        
        static StateMenu _instance;
};
