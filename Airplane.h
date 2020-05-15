
#pragma once

#include <Pokitto.h>
#include "FixedPoint/Fixed.h"
#include "FixedPoint/FixedMath.h"
#include "RocketExhaust.h"
#include "Scene.h"

#include "sprites/airplaneS.h"
#include "sprites/airplaneSSSE.h"
#include "sprites/airplaneSSE.h"
#include "sprites/airplaneSE.h"
#include "sprites/airplaneSEE.h"
#include "sprites/airplaneSEEE.h"
#include "sprites/airplaneE.h"
#include "sprites/airplaneNEEE.h"
#include "sprites/airplaneNEE.h"
#include "sprites/airplaneNE.h"
#include "sprites/airplaneNNE.h"
#include "sprites/airplaneNNNE.h"
#include "sprites/airplaneN.h"

#include "sounds/rocket11k.h"
#include "sounds/crash11k.h"

class Airplane {
    
    public:
    
        Airplane(): _state(State::Launch), _pos_x(_INIT_X), _pos_y(_INIT_Y), _pitch(FixedPoint::Fixed<10>::Pi / 4), _steam(0), _distance(0) {}
        
        ~Airplane() = default;
        
        inline void pitchControl(std::int8_t amount) { _pitch_input = amount; }
        
        inline void throttleControl(std::int8_t amount) {
            if(amount > 0 && _steam > FixedPoint::Fixed<10>(0)) {
                if(_throttle_input <= 0) {
                    Pokitto::Sound::loadSampleToOsc(1, (std::uint8_t*)rocket_snd, sizeof(rocket_snd));
                    
                    // Start playing osc1
                    constexpr std::uint8_t ON = 1;
                    constexpr std::uint8_t WAVETYPE_SAMPLE = MAX_WAVETYPES;
                    constexpr std::uint8_t LOOP = 1;      // 1 to loop the sample
                    constexpr std::uint8_t NOTENUM = 37;  // 37 sets playing speed to 1:1 (I think)
                    constexpr std::uint16_t VOLUME = 254;
                    setOSC(&osc1, ON, WAVETYPE_SAMPLE, LOOP, 0, 0, NOTENUM, VOLUME, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                }
            }
            else if(_throttle_input > 0) {
                // Stop playing osc1
                constexpr std::uint8_t OFF = 0;
                setOSC(&osc1, OFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            }
            
            _throttle_input = amount;
        }
        
        inline std::uint32_t steam() const { return (_steam.integer() > 0) ? _steam.integer() : 0; }
        
        inline std::uint32_t distance() const { return _distance.integer(); }
        
        inline bool landed() const {
            return (_vel_x.integer() == 0)
                && ((_state == State::OnGround) || ((_state == State::Launch) && (_steam <= FixedPoint::Fixed<10>(0))));
        }
        
        inline std::int32_t locationX() const { return (_pos_x * (FixedPoint::Fixed<10>(1) / _SCALE)).integer() - 11; }
        inline std::int32_t locationY() const { return POK_TILE_H * Scene::MAP_HEIGHT - (_pos_y * (FixedPoint::Fixed<10>(1) / _SCALE)).integer() - 11; }
        
        inline void reset() {
            _state = State::Launch;
            
            _pos_x = _INIT_X;
            _pos_y = _INIT_Y;
            _vel_x = FixedPoint::Fixed<10>(0);
            _vel_y = FixedPoint::Fixed<10>(0);
            
            _pitch = FixedPoint::Fixed<10>::Pi / 4;
            _steam = FixedPoint::Fixed<10>(100);
            _distance = FixedPoint::Fixed<10>(0);
        }
        
        inline void update(const Scene& scene) {
            // Forward and up unit vectors
            const FixedPoint::Fixed<10> fwd_x = FixedPoint::cos(_pitch);
            const FixedPoint::Fixed<10> fwd_y = FixedPoint::sin(_pitch);
            const FixedPoint::Fixed<10>& up_x = -fwd_y;
            const FixedPoint::Fixed<10>& up_y = fwd_x;
            
            // Speed relative to air
            Scene::FlowVector wind = scene.flowVectorAt(locationX() + 11, locationY() + 11);
            const FixedPoint::Fixed<10> airspeed_x = _vel_x - 5 * FixedPoint::Fixed<10>(wind.x());
            const FixedPoint::Fixed<10> airspeed_y = _vel_y - 5 * FixedPoint::Fixed<10>(wind.y());
            
            // Forward and up components of velocity
            const FixedPoint::Fixed<10> vel_fwd = fwd_x * airspeed_x + fwd_y * airspeed_y;
            const FixedPoint::Fixed<10> vel_up = up_x * airspeed_x + up_y * airspeed_y;
            
            FixedPoint::Fixed<10> dvel_fwd;
            FixedPoint::Fixed<10> dvel_up;
            
            // Thrust
            if((_throttle_input > 0) && (_steam > FixedPoint::Fixed<10>(0))) {
                _steam -= 20 * _DTIME;
                dvel_fwd = _throttle_input * _THRUST * _DTIME / _MASS;
            }
            
            // Forward drag
            constexpr const FixedPoint::Fixed<10> DRAG_COEF(0.01);
            dvel_fwd -= (0.5 * _AIR_DENSITY * vel_fwd * FixedPoint::abs(vel_fwd) * DRAG_COEF * _WING_AREA * _DTIME) / _MASS;
            
            if(_state == State::Launch) {
                // Gravitation along launch pad
                dvel_fwd -= fwd_y * FixedPoint::Fixed<10>(9.81) * _DTIME;
            }
            else if(_state == State::OnGround) {
                if(_vel_x != FixedPoint::Fixed<10>(0)) {
                    FixedPoint::Fixed<10> scale = FixedPoint::Fixed<10>(1) - 50 * (-_vel_y + FixedPoint::Fixed<10>(1)) * _DTIME / _MASS;
                    _vel_x = (scale > FixedPoint::Fixed<10>(0)) ? _vel_x * scale : FixedPoint::Fixed<10>(0);
                }
            }
            else {
                // Drag caused by air hitting the bottom surface of the wings
                dvel_up -= (0.5 * _AIR_DENSITY * _WING_AREA * vel_up * FixedPoint::abs(vel_up) * _DTIME) / _MASS;
                
                // Aerodynamic lift component perpendicular to air flow
                const FixedPoint::Fixed<10> angle_attack = -FixedPoint::atan2(vel_up, vel_fwd);
                FixedPoint::Fixed<10> airspeed_sq = airspeed_x * airspeed_x + airspeed_y * airspeed_y;
                FixedPoint::Fixed<10> dvel = (FixedPoint::Fixed<10>(0.5) * _AIR_DENSITY * _WING_AREA * _liftCoef(angle_attack) * airspeed_sq * _DTIME) / _MASS;
                
                // Total lift in up direction
                dvel_up = dvel / FixedPoint::cos(angle_attack);
                
                // Gravity
                _vel_y -= FixedPoint::Fixed<10>(9.81) * _DTIME;
                
                _pitch += _pitch_input * _DTIME * ((1.1 * vel_fwd > FixedPoint::Fixed<10>(1000))
                    ? FixedPoint::Fixed<10>(1000)
                    : 1.1 * vel_fwd)
                    * FixedPoint::Fixed<10>::Pi / 180;
                _pitch -= angle_attack * FixedPoint::Fixed<10>(1.6) * _DTIME;
                _pitch = angleIntoRange(_pitch);
            }
            
            _vel_x += dvel_up * up_x + dvel_fwd * fwd_x;
            _vel_y += dvel_up * up_y + dvel_fwd * fwd_y;
            
            // Modify position by velocity
            _pos_x += _vel_x * _DTIME;
            _pos_y += _vel_y * _DTIME;
            
            if(_pos_x - (_INIT_X + _LAUNCHPAD_LEN) > _distance) _distance = _pos_x - (_INIT_X + _LAUNCHPAD_LEN);
            
            if(_state == State::Launch) {
                if(_pos_x < _INIT_X || _pos_y < _INIT_Y) {
                    _vel_x = FixedPoint::Fixed<10>(0);
                    _vel_y = FixedPoint::Fixed<10>(0);
                    _pos_x = _INIT_X;
                    _pos_y = _INIT_Y;
                }
                else if(_pos_x >= _INIT_X + _LAUNCHPAD_LEN) {
                    _state = State::Flying;
                }
            }
            else if(_state == State::Flying) {
                if(_pos_y.integer() < (static_cast<std::int32_t>(POK_TILE_H) * _SCALE).integer()) {
                    _state = State::OnGround;
                    _pos_y = static_cast<std::int32_t>(POK_TILE_H) * _SCALE;
                    
                    Pokitto::Sound::loadSampleToOsc(1, (std::uint8_t*)crash_snd, sizeof(crash_snd));
                    
                    // Start playing osc1
                    constexpr std::uint8_t ON = 1;
                    constexpr std::uint8_t WAVETYPE_SAMPLE = MAX_WAVETYPES;
                    constexpr std::uint8_t LOOP = 0;      // 1 to loop the sample
                    constexpr std::uint8_t NOTENUM = 37;  // 37 sets playing speed to 1:1 (I think)
                    std::uint16_t volume = (((-_vel_y.integer() < 20) ? -_vel_y.integer() : 20) * 254) / 20;
                    setOSC(&osc1, ON, WAVETYPE_SAMPLE, LOOP, 0, 0, NOTENUM, volume, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                }
            }
            else {
                _pos_y = static_cast<std::int32_t>(POK_TILE_H) * _SCALE;
            }
        }
        
        // Approximates lift coefficient by sin function
        static constexpr FixedPoint::Fixed<10> _liftCoef(const FixedPoint::Fixed<10>& angle_attack) {
            constexpr const FixedPoint::Fixed<10> _MAX_LIFT = (2 * FixedPoint::Fixed<10>::Pi / 0.8660254) * (2 * _ANGLE_MAX_LIFT / 3);
            
            if(angle_attack < 2 * _ANGLE_MAX_LIFT && angle_attack > -2 * _ANGLE_MAX_LIFT) {
                return _MAX_LIFT * FixedPoint::sin(angle_attack * FixedPoint::Fixed<10>::Pi / (2 * _ANGLE_MAX_LIFT));
            }
            return FixedPoint::Fixed<10>(0);
        }
        
        inline void draw(std::int32_t camera_x, std::int32_t camera_y) {
            bool mirror = false;
            
            std::uint8_t sprite_idx = (_pitch * (FixedPoint::Fixed<10>(12) / FixedPoint::Fixed<10>::Pi) + FixedPoint::Fixed<10>(0.5)).integer();
            if(sprite_idx < 6) {
                sprite_idx += 6;
            }
            else if(sprite_idx >= 18) {
                sprite_idx -= 18;
            }
            else {
                mirror = true;
                sprite_idx = 12 - (sprite_idx - 6);
            }
            
            constexpr bool flip = false;
            constexpr std::uint8_t recolor = 0;
            Pokitto::Display::drawSprite(locationX() - camera_x, locationY() - camera_y, _sprites[sprite_idx], flip, mirror, recolor);
            
            if(_throttle_input > 0 && _steam > FixedPoint::Fixed<10>(0)) {
                _exhaust.draw(locationX() + 11, locationY() + 11, _pitch, camera_x, camera_y);
            }
        }
    
    private:
    
        enum class State: std::uint8_t { Launch, Flying, OnGround };
        
        static constexpr FixedPoint::Fixed<10> angleIntoRange(const FixedPoint::Fixed<10>& angle) {
            constexpr const FixedPoint::Fixed<10>& pi = FixedPoint::Fixed<10>::Pi;
            if(angle >= 2 * pi) {
                return angle - (angle / (2 * pi)).integer() * 2 * pi;
            }
            else if(angle < FixedPoint::Fixed<10>(0)) {
                return angle + (1 - (angle / (2 * pi)).integer()) * 2 * pi;
            }
            return angle;
        }
        
        static constexpr const std::uint8_t* _sprites[13] = {
            airplaneS, airplaneSSSE, airplaneSSE, airplaneSE, airplaneSEE, airplaneSEEE, 
            airplaneE, airplaneNEEE, airplaneNEE, airplaneNE, airplaneNNE, airplaneNNNE, airplaneN
        };
        
        static constexpr FixedPoint::Fixed<10> _DTIME = FixedPoint::Fixed<10>(1.0 / 30);
        
        static constexpr std::int32_t _MASS = 350;      // kilograms
        static constexpr std::int32_t _THRUST = 14000;  // Newtons
        static constexpr std::int32_t _WING_AREA = 13;  // square meters
        
        static constexpr FixedPoint::Fixed<10> _AIR_DENSITY = FixedPoint::Fixed<10>(1.225);
        static constexpr FixedPoint::Fixed<10> _ANGLE_MAX_LIFT = 10 * FixedPoint::Fixed<10>::Pi / 180;
        
        static constexpr FixedPoint::Fixed<10> _SCALE = FixedPoint::Fixed<10>(0.5);
        static constexpr FixedPoint::Fixed<10> _INIT_X = FixedPoint::Fixed<10>(100 * _SCALE);
        static constexpr FixedPoint::Fixed<10> _INIT_Y = FixedPoint::Fixed<10>(24 * _SCALE);
        static constexpr FixedPoint::Fixed<10> _LAUNCHPAD_LEN = FixedPoint::Fixed<10>(44 * _SCALE);
        
        State _state;
        
        FixedPoint::Fixed<10> _pitch;
        FixedPoint::Fixed<10> _pos_x;
        FixedPoint::Fixed<10> _pos_y;
        FixedPoint::Fixed<10> _vel_x;
        FixedPoint::Fixed<10> _vel_y;
        
        std::int8_t _pitch_input;
        std::int8_t _throttle_input;
        
        FixedPoint::Fixed<10> _steam;
        FixedPoint::Fixed<10> _distance;
        
        RocketExhaust _exhaust;
};
