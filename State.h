
#pragma once

#include <cstdint>

class State {
    
    public:
    
        explicit State() = default;
        virtual ~State() {}
        
        State(const State&) = delete;
        State& operator =(const State&) = delete;
        
        virtual void init() = 0;
        virtual void enter() = 0;
        virtual void leave() = 0;
        virtual void update() = 0;
};
