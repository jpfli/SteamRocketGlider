
#pragma once

#include <cstdint>
#include <Tilemap.hpp>
#include "FixedPoint/Fixed.h"
#include "SimplexNoise.h"

#include "images/tile_00.h"
#include "images/tile_01.h"
#include "images/tile_02.h"
#include "images/tile_03.h"
#include "images/tile_04.h"
#include "images/tile_10.h"
#include "images/tile_11.h"
#include "images/tile_12.h"
#include "images/tile_13.h"
#include "images/tile_14.h"
#include "images/tile_20.h"
#include "images/tile_21.h"
#include "images/tile_22.h"
#include "images/tile_23.h"
#include "images/tile_24.h"
#include "images/tile_30.h"
#include "images/tile_31.h"
#include "images/tile_32.h"
#include "images/tile_33.h"
#include "images/tile_34.h"
#include "images/tile_40.h"
#include "images/tile_41.h"
#include "images/tile_42.h"
#include "images/tile_43.h"
#include "images/tile_44.h"
#include "images/groundtile.h"

#include "sprites/launchpad.h"

class Scene {
    
    public:
    
        static constexpr std::uint32_t MAP_WIDTH = 2 * 14;
        static constexpr std::uint32_t MAP_HEIGHT = 80;
    
        Scene() : _tilemap(), _map{}, _camera_x(0), _camera_y(0) {
            constexpr unsigned NUM_TILES = 5 * 5 + 1;
            constexpr const std::uint8_t* tiles[NUM_TILES] = {
                tile_00, tile_01, tile_02, tile_03, tile_04,
                tile_10, tile_11, tile_12, tile_13, tile_14,
                tile_20, tile_21, tile_22, tile_23, tile_24,
                tile_30, tile_31, tile_32, tile_33, tile_34,
                tile_40, tile_41, tile_42, tile_43, tile_44,
                groundtile
            };
            for(std::size_t idx = 0; idx < NUM_TILES; ++idx) {
                _tilemap.setTile(idx, POK_TILE_W, POK_TILE_H, tiles[idx] + 2);
            }
            
            _tilemap.set(MAP_WIDTH, MAP_HEIGHT, _map);
            
            _fillMap(true);
        }
        
        inline void _fillMap(bool clear=false) {
            const std::int32_t first_col = (MAP_WIDTH / 2) * (_camera_x / (POK_TILE_W * (MAP_WIDTH / 2)));
            for(std::size_t row = 0; row < MAP_HEIGHT; ++row) {
                for(std::size_t col = 0; col < MAP_WIDTH; ++col) {
                    if(clear && row < MAP_HEIGHT - 1) {
                        _mapWrite(col, row, 2 * 5 + 2);
                    }
                    else {
                        _mapWrite(col, row, _getTile(first_col + col, row));
                    }
                }
            }
        }
        
        ~Scene() = default;
        
        struct FlowVector {
            constexpr FlowVector(std::int32_t x, std::int32_t y) : xy((x & 0xff) | (y << 8)) {}
            
            constexpr std::int32_t x() const { return static_cast<std::int8_t>(xy); }
            constexpr std::int32_t y() const { return static_cast<std::int8_t>(xy >> 8); }
            
            std::uint16_t xy;
        };
        
        inline FlowVector flowVectorAt(std::int32_t x, std::int32_t y) const {
            std::int32_t col = x / POK_TILE_W - (MAP_WIDTH / 2) * (_camera_x / (POK_TILE_W * (MAP_WIDTH / 2)));
            std::int32_t row = y / POK_TILE_H;
            
            if(col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
                return FlowVector(0, 0);
            }
            
            snapToFlowMap(col, row);
            
            std::uint8_t tile_id = _mapRead(col, row);
            return FlowVector((tile_id) % 5 - 2, 2 - tile_id / 5);
        }
        
        inline void update(std::int32_t camera_x, std::int32_t camera_y) {
            std::int32_t x = _camera_x / POK_TILE_W;
            while(x < camera_x / POK_TILE_W) {
                ++x;
                std::int32_t mod_x1 = ((x < 1) ? (MAP_WIDTH / 2) : 0) + (x - 1) % (MAP_WIDTH / 2);
                std::int32_t mod_x2 = ((x < 0) ? (MAP_WIDTH / 2) : 0) + x % (MAP_WIDTH / 2);
                for(std::size_t y = 0; y < MAP_HEIGHT; ++y) {
                    _mapWrite(mod_x1, y, _mapRead(mod_x1 + (MAP_WIDTH / 2), y));
                    _mapWrite(mod_x2 + (MAP_WIDTH / 2), y, _getTile(x + (MAP_WIDTH / 2), y));
                }
            }
            while(x > camera_x / POK_TILE_W) {
                --x;
                std::int32_t mod_x1 = ((x < -1) ? (MAP_WIDTH / 2) : 0) + (x + 1) % (MAP_WIDTH / 2);
                std::int32_t mod_x2 = ((x < 0) ? (MAP_WIDTH / 2) : 0) + x % (MAP_WIDTH / 2);
                for(std::size_t y = 0; y < MAP_HEIGHT; ++y) {
                    _mapWrite(mod_x1 + (MAP_WIDTH / 2), y, _mapRead(mod_x1, y));
                    _mapWrite(mod_x2, y, _getTile(x, y));
                }
            }
            
            _camera_x = camera_x;
            _camera_y = camera_y;
        }
        
        inline void draw() {
            _drawLaunchPad();
            
            _tilemap.draw(-(_camera_x % (POK_TILE_W * (MAP_WIDTH / 2))), -_camera_y);
        }
        
        inline void reset(std::uint32_t seed) {
            _camera_x = 0;
            _camera_y = 0;
            SimplexNoise::setSeed(seed);
            _fillMap();
        }
    
    private:
    
        static constexpr void snapToFlowMap(std::int32_t& col, std::int32_t& row) {
            row &= ~1;
            col = (col | 1) - ((row >> 1) & 1);
        }
        
        static constexpr std::uint8_t _getTile(std::int32_t col, std::int32_t row) {
            if(row < MAP_HEIGHT - 1) {
                std::int32_t x_val = 2;
                std::int32_t y_val = 2;
                
                const std::int32_t col_orig = col;
                const std::int32_t row_orig = row;
                snapToFlowMap(col, row);
                
                if(col_orig == col && row_orig == row) {
                    FixedPoint::Fixed<24> noise = SimplexNoise::noise_fixp(FixedPoint::Fixed<10>(0.01)*col, FixedPoint::Fixed<10>(0.03)*row);
                    x_val = (2.5 * (FixedPoint::Fixed<24>(1) + noise)).integer();
                    
                    noise = SimplexNoise::noise_fixp(FixedPoint::Fixed<10>(-0.03)*row, FixedPoint::Fixed<10>(0.01)*col);
                    y_val = (2.5 * (FixedPoint::Fixed<24>(1) + noise)).integer();
                }
                return 5 * y_val + x_val;
            }
            else {
                return 25;
            }
        }
        
        inline void _drawLaunchPad() {
            constexpr bool flip = false;
            constexpr bool mirror = false;
            constexpr std::uint8_t recolor = 0;
            Pokitto::Display::drawSprite(96 - _camera_x, POK_TILE_H * MAP_HEIGHT - 64 - _camera_y, _sprite, flip, mirror, recolor);
        }
        
        inline std::uint8_t _mapRead(std::size_t x, std::size_t y) const {
            return _map[(y * MAP_WIDTH + x)];
        }
        
        inline void _mapWrite(std::size_t x, std::size_t y, std::uint8_t tile_id) {
            _map[(y * MAP_WIDTH + x)] = tile_id;
        }
        
        static constexpr const std::uint8_t* _sprite = launchpad;
        
        Tilemap _tilemap;
        std::uint8_t _map[MAP_WIDTH * MAP_HEIGHT];
        
        std::int32_t _camera_x;
        std::int32_t _camera_y;
};
