/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <cstdlib>

#ifndef M_PI
#	define M_PI           3.14159265358979323846 /* pi */
#endif

#ifndef M_PI_2
#	define M_PI_2         1.57079632679489661923 /* pi/2 */
#endif

#ifndef M_E
#	define M_E            2.7182818284590452354   /* e */
#endif

#ifndef M_LOG2E
#	define M_LOG2E        1.4426950408889634074   /* log_2 e */
#endif

#ifndef M_LOG10E
#	define M_LOG10E       0.43429448190325182765  /* log_10 e */
#endif

#ifndef M_LN2
#	define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif

#ifndef M_LN10
#	define M_LN10         2.30258509299404568402  /* log_e 10 */
#endif

#ifndef M_PI_4
#	define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif

#ifndef M_1_PI
#	define M_1_PI         0.31830988618379067154  /* 1/pi */
#endif

#ifndef M_2_PI
#	define M_2_PI         0.63661977236758134308  /* 2/pi */
#endif

#ifndef M_2_SQRTPI
#	define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#endif

#ifndef M_SQRT2
#	define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#	define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifndef M_2PI
#	define M_2PI          6.283185307179586232    /* 2*pi  */
#endif

extern const float epsilon;

template <typename T>
inline T clamp(T value, T min, T max){
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*inline Vector3f calc_normal(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2){
	Vector3f u = p0 - p1;
	Vector3f v = p1 - p2;
	return Vector3f::cross(u, v).normalized();
}*/

inline float randf(float lower, float upper){
	float pivot = upper - lower;
	float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return lower + r * pivot;
}

static const float PI = static_cast<float>(M_PI);
static const float PI_2 = static_cast<float>(M_PI/2);

inline float rad_to_deg(float a){
	return a * ( 180 / static_cast<float>(M_PI) );
}

inline float deg_to_rad(float a){
	return a * ( static_cast<float>(M_PI) / 180 );
}

//bool intersect(const Sphere& sphere, const AABB& aabb, bool ignore_y_axis = false);
//bool intersect(const Ray& ray, const Frustum& frustum);

template <class T> inline T min(T a, T b){
	return b + ((a - b) & -(a < b));
}

template <> inline float min<float>(float a, float b){
	return a < b ? a : b;
}

template <class T> inline T max(T a, T b){
	return a - ((a - b) & -(a < b));
}

template <> inline float max<float>(float a, float b){
	return a > b ? a : b;
}

inline bool is_pow2(unsigned int x){
	return x > 0 ? ( x & ( x - 1 ) ) == 0 : false;
}

extern const unsigned next_pow2_dd3_bit[0x100];

// Based upon code "DD3" by DigitalDelusion from a discussion at gamedev
// http://www.gamedev.net/community/forums/topic.asp?topic_id=229831
inline unsigned next_pow2(unsigned x){
	--x;
	int shift = 0;
	if( x & 0xFFFF0000)
	{
		x >>= 16;
		shift = 16;
	}
	if( x & 0xFF00)
	{
		x >>= 8;
		shift += 8;
	}
	return next_pow2_dd3_bit[x] << shift;
}

template <class T>
int truncate(T value);

#endif /* MATH_H */
