
#pragma once

#include <cstdint>
#include "State.h"

class StatePlaying final: public State {
    
    public:
    
        StatePlaying(const StatePlaying&) = delete;
        StatePlaying& operator =(const StatePlaying&) = delete;
        
        static constexpr StatePlaying& instance() { return _instance; }
        
        inline void init() override {}
        
        void enter() override;
        
        inline void leave() override {}
        
        void update() override;
    
    private:
    
        StatePlaying() = default;
        virtual ~StatePlaying() = default;
        
        void _drawTopBar();
        
        static StatePlaying _instance;
};
