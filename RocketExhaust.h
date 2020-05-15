
#pragma once

#include "sprites.h"
#include "FixedPoint/Fixed.h"
#include "FixedPoint/FixedMath.h"

class RocketExhaust {
    
    public:
    
        RocketExhaust(): _cur_idx(-1) {}
        
        ~RocketExhaust() = default;
        
        inline void draw(std::int32_t x, std::int32_t y, const FixedPoint::Fixed<10>& pitch, std::int32_t camera_x, std::int32_t camera_y) {
            bool mirror = false;
            bool flip = false;
            
            std::uint8_t anim_idx = (pitch * (FixedPoint::Fixed<10>(12) / FixedPoint::Fixed<10>::Pi) + FixedPoint::Fixed<10>(0.5)).integer();
            
            x -= (20 * FixedPoint::cos(anim_idx * FixedPoint::Fixed<10>::Pi / 12)).integer();
            y += (20 * FixedPoint::sin(anim_idx * FixedPoint::Fixed<10>::Pi / 12)).integer();
            
            if(anim_idx >= 18) {
                flip = true;
                anim_idx = 24 - anim_idx;
            }
            else if(anim_idx >= 12) {
                flip = true;
                mirror = true;
                anim_idx = anim_idx - 12;
            }
            else if(anim_idx > 6) {
                mirror = true;
                anim_idx = 12 - anim_idx;
            }
            
            if(anim_idx != _cur_idx) {
                _cur_idx = anim_idx;
                ExhaustSprite::Animation anim;
                switch(anim_idx) {
                    case 0: anim = ExhaustSprite::E; break;
                    case 1: anim = ExhaustSprite::NEEE; break;
                    case 2: anim = ExhaustSprite::NEE; break;
                    case 3: anim = ExhaustSprite::NE; break;
                    case 4: anim = ExhaustSprite::NNE; break;
                    case 5: anim = ExhaustSprite::NNNE; break;
                    case 6: anim = ExhaustSprite::N; break;
                }
                _sprite.play(exhaustSprite, anim);
            }
            
            _sprite.draw(x-8 - camera_x, y-8 - camera_y, flip, mirror);
        }
    
    private:
    
        RocketExhaust(const RocketExhaust&) = delete;
        RocketExhaust& operator =(const RocketExhaust&) = delete;
        
        Sprite _sprite;
        std::uint8_t _cur_idx;
};
