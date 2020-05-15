
#pragma once

#include "Fixed.h"

namespace FixedPoint {

template<unsigned Q>
constexpr Fixed<Q> abs(const Fixed<Q>& value) {
    return (value < FixedPoint::Fixed<Q>(0)) ? -value : value;
}

template<unsigned Q>
constexpr typename Fixed<Q>::IntegerType floor(const Fixed<Q>& value) {
    return value.internal() >> Q;
}

template<unsigned Q>
constexpr typename Fixed<Q>::IntegerType sign(const Fixed<Q>& value) {
    return (value > FixedPoint::Fixed<Q>(0)) - (value < FixedPoint::Fixed<Q>(0));
}

template<unsigned Q>
constexpr Fixed<Q> sin(Fixed<Q> radians) {
    constexpr const Fixed<Q>& pi = Fixed<Q>::Pi;
    
    Fixed<Q> angle = radians + pi / 2;
    if(2 * pi <= angle) {
        radians -= 2 * pi * (angle / (2 * pi)).integer();
    }
    else if(Fixed<Q>(0) > angle) {
        radians += 2 * pi * (1 - (angle / (2 * pi)).integer());
    }
    
    // Mirror angles larger than 90 degrees
    if(pi/2 < radians) {
        radians = pi - radians;
    }
    
    // For angles close to -90 and 90 degrees use cosine approximation
    constexpr const Fixed<Q> LIMIT = 52.65 * pi / 180;
    if(LIMIT <= radians) {
        radians -= pi / 2;
        Fixed<Q> angle_sq = radians * radians;
        return Fixed<Q>(1) - angle_sq * (Fixed<Q>(0.5) - angle_sq * 0.041672);
    }
    else if(-LIMIT >= radians) {
        radians += pi / 2;
        Fixed<Q> angle_sq = radians*radians;
        return angle_sq * (Fixed<Q>(0.5) - angle_sq * 0.041672) - Fixed<Q>(1);
    }
    
    // For all other angles use sine approximation
    Fixed<Q> angle_sq = radians*radians;
    return radians * (Fixed<Q>(1) - angle_sq * (Fixed<Q>(0.1666718) - angle_sq * 0.0083313));
}

template<unsigned Q>
inline constexpr Fixed<Q> cos(const Fixed<Q>& radians) { return sin(radians + Fixed<Q>::Pi / 2); }

template<unsigned Q>
inline constexpr Fixed<Q> tan(const Fixed<Q>& radians) { return sin(radians) / cos(radians); }

template<unsigned Q>
constexpr Fixed<Q> atan2(const Fixed<Q>& y, const Fixed<Q>& x) {
	const Fixed<Q> abs_y = (y < Fixed<Q>(0)) ? -y : y;
    if(x >= Fixed<Q>(0)) {
        Fixed<Q> r = (x - abs_y) / (abs_y + x);
        Fixed<Q> angle = (0.19629 * r * r - Fixed<Q>(0.98169)) * r + Fixed<Q>::Pi / 4;
        return (y < Fixed<Q>(0)) ? -angle : angle;
    }
    else {
        Fixed<Q> r = (x + abs_y) / (abs_y - x);
        Fixed<Q> angle = (0.19629 * r * r - Fixed<Q>(0.98169)) * r + 3 * Fixed<Q>::Pi / 4;
        return (y < Fixed<Q>(0)) ? -angle : angle;
    }
}

} // namespace FixedPoint
