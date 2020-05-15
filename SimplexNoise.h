
#pragma once

#include <cstdio>
#include <array>
#include "FixedPoint/Fixed.h"
#include "Random.h"

class SimplexNoise {
    
    public:
    
        SimplexNoise() = default;
        ~SimplexNoise() = default;
        
        static inline void setSeed(std::uint32_t seed) {
            for(std::size_t idx = 0; idx < 256; ++idx) {
                _perm[idx] = _perm_orig[idx];
            }
            if(seed != 0) {
                _random.setSeed(seed);
                _shuffle(_perm);
            }
        }
        
        static FixedPoint::Fixed<24> noise_fixp(const FixedPoint::Fixed<10>& x, const FixedPoint::Fixed<10>& y) {
            constexpr const unsigned Q = 24;
            
            // Skewing/Unskewing factors for 2D
            constexpr const FixedPoint::Fixed<Q> F2(0.366025403);  // F2 = (sqrt(3) - 1) / 2
            constexpr const FixedPoint::Fixed<Q> G2(0.211324865);  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)
            
            // Skew the input space to determine which simplex cell we're in
            const std::int64_t s_Q10 = ((static_cast<int64_t>(x.internal()) + static_cast<int64_t>(y.internal())) * F2.internal()) >> Q;
            const std::int32_t i = (x.internal() + s_Q10) >> 10;
            const std::int32_t j = (y.internal() + s_Q10) >> 10;
            
            // Unskew the cell origin back to (x,y) space
            const std::int64_t t_Q24 = (i + j) * static_cast<std::int64_t>(G2.internal());
            const FixedPoint::Fixed<Q> x0 = FixedPoint::Fixed<Q>::fromInternal((static_cast<std::int64_t>(x.internal()) << (Q - 10)) - (static_cast<std::int64_t>(i) << Q) + t_Q24);
            const FixedPoint::Fixed<Q> y0 = FixedPoint::Fixed<Q>::fromInternal((static_cast<std::int64_t>(y.internal()) << (Q - 10)) - (static_cast<std::int64_t>(j) << Q) + t_Q24);
            
            // For the 2D case, the simplex shape is an equilateral triangle.
            // Determine which simplex we are in.
            const std::int32_t i1 = (x0 > y0);  // Offsets for second (middle) corner of simplex in (i,j) coords
            const std::int32_t j1 = (x0 <= y0);
            
            // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
            // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where c = (3-sqrt(3))/6
            const FixedPoint::Fixed<Q> x1 = x0 - FixedPoint::Fixed<Q>(i1) + G2;      // Offsets for middle corner in (x,y) unskewed coords
            const FixedPoint::Fixed<Q> y1 = y0 - FixedPoint::Fixed<Q>(j1) + G2;
            const FixedPoint::Fixed<Q> x2 = x0 - FixedPoint::Fixed<Q>(1) + 2 * G2;   // Offsets for last corner in (x,y) unskewed coords
            const FixedPoint::Fixed<Q> y2 = y0 - FixedPoint::Fixed<Q>(1) + 2 * G2;
            
            FixedPoint::Fixed<Q> n0, n1, n2;
            
            // Calculate the contribution from the first corner
            FixedPoint::Fixed<Q> t0 = FixedPoint::Fixed<Q>(0.5) - x0*x0 - y0*y0;
            if(t0 > FixedPoint::Fixed<Q>(0)) {
                t0 *= t0;
                n0 = t0 * t0 * _dot(_grad(_hash(i + _hash(j))), x0, y0);
            }
            
            // Calculate the contribution from the second corner
            FixedPoint::Fixed<Q> t1 = FixedPoint::Fixed<Q>(0.5) - x1*x1 - y1*y1;
            if(t1 > FixedPoint::Fixed<Q>(0)) {
                t1 *= t1;
                n1 = t1 * t1 * _dot(_grad(_hash(i + i1 + _hash(j + j1))), x1, y1);
            }
            
            // Calculate the contribution from the third corner
            FixedPoint::Fixed<Q> t2 = FixedPoint::Fixed<Q>(0.5) - x2*x2 - y2*y2;
            if(t2 > FixedPoint::Fixed<Q>(0)) {
                t2 *= t2;
                n2 = t2 * t2 * _dot(_grad(_hash(i + 1 + _hash(j + 1))), x2, y2);
            }
            
            // Add contributions from each corner to get the final noise value.
            // The result is scaled to return values in the interval [-1000,1000].
            return 70 * (n0 + n1 + n2);
        }
    
    private:
    
        struct Grad2 {
            constexpr Grad2(std::int32_t x, std::int32_t y) : _x(x), _y(y) {}
            
            constexpr std::int32_t x() const { return _x; }
            constexpr std::int32_t y() const { return _y; }
            
            std::int32_t _x;
            std::int32_t _y;
        };
        
        static inline void _shuffle(std::array<std::uint8_t, 256>& arr) {
            for(std::size_t idx = 0; idx < 256; ++idx) {
                std::uint8_t& ref = arr[idx + _random.nextInt(256 - idx)];
                std::uint8_t acc = ref;
                ref = arr[idx];
                arr[idx] = acc;
            }
        }
        
        static inline Grad2 _grad(std::int32_t hash) {
            std::int32_t h = hash & 7;
            std::int32_t i = (h & 3) ? ((h < 4) ? 1 : -1) : 0;
            
            h = (hash - 2) & 7;
            std::int32_t j = (h & 3) ? ((h < 4) ? 1 : -1) : 0;
            
            return Grad2(i, j);
        }
        
        template<unsigned Q>
        static inline FixedPoint::Fixed<Q> _dot(const Grad2& grad, const FixedPoint::Fixed<Q>& x, const FixedPoint::Fixed<Q>& y) {
            return grad.x() * x + grad.y() * y;
        }
        
        static inline std::int32_t _hash(std::uint8_t idx) {
            return _perm[idx];
        }
        
        // Numbers 0 to 255 in random order
        static constexpr const std::uint8_t _perm_orig[256] = {
            151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225, 
            140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148, 
            247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32, 
             57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175, 
             74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122, 
             60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54, 
             65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169, 
            200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64, 
             52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212, 
            207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213, 
            119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9, 
            129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104, 
            218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241, 
             81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157, 
            184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93, 
            222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180
        };
        
        static inline std::array<std::uint8_t, 256> _perm = []{
            std::array<std::uint8_t, 256> arr;
            for(std::size_t idx = 0; idx < arr.size(); ++idx) {
                arr[idx] = _perm_orig[idx];
            }
            return arr;
        }();
        
        static inline Random _random;
};
