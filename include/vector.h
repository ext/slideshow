/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
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

#ifndef __BLUEFLOWER_VECTOR_H
#define __BLUEFLOWER_VECTOR_H

// Derived from blueflower (c) David Sveningsson 2009-2010

#include "mathutils.h"

/* While waiting for make_signed to be part of the standard...
 * It takes a typename and if possible gives the signed version of the same
 * type, eg unsigned int to signed int. */
template <typename T>
struct make_signed {
	typedef T type;
};

template <>
struct make_signed<unsigned int> {
	typedef int type;
};

template <class T>
class Vector2 {
	public:
		typedef typename make_signed<T>::type difference_T;

		Vector2(): x(0), y(0){}
		Vector2(T x, T y): x(x), y(y){}

		template <class inner>
		Vector2<T>(const Vector2<inner>& v): x(v.x), y(v.y){}

		Vector2& operator=(const Vector2& v){
			x = v.x;
			y = v.y;
			return *this;
		}

		Vector2 operator+(Vector2 rhs) const {
			return Vector2(x + rhs.x, y + rhs.y);
		}

		Vector2 operator+=(const Vector2& rhs) const {
			return Vector2(x + rhs.x, y + rhs.y);
		}

		Vector2<difference_T> operator-(Vector2 rhs) const {
			return Vector2<difference_T>(x - rhs.x, y - rhs.y);
		}

		Vector2 operator*(T scalar) const {
			return Vector2(x * scalar, y * scalar);
		}

		Vector2 operator/(T scalar) const {
			return Vector2(x / scalar, y / scalar);
		}

		T length() const {
			return sqrt(x*x + y*y);
		}

		T lengthSquared() const {
			return x*x + y*y;
		}

		Vector2 normalize() const {
			T leninv = 1 / length();
			return Vector2(x*leninv, y*leninv);
		}

		Vector2 invert() const {
			return Vector2(-x, -y);
		}

		static Vector2 cross(Vector2 const &v, Vector2 const &w){
			return Vector2(
				v.y*w.z - v.z*w.y,
				v.z*w.x - v.x*w.z
			);
		}

		static T dot(Vector2 const &v, Vector2 const &w){
			return (v.x*w.x + v.y*w.y);
		}

		static T distance(const Vector2& v, const Vector2& w){
			T dx = w.x - v.x;
			T dy = w.y - v.y;
			return sqrt( dx*dx + dy*dy );
		}

		static T distanceSquared(const Vector2& v, const Vector2& w){
			T dx = w.x - v.x;
			T dy = w.y - v.y;
			return ( dx*dx + dy*dy );
		}

		Vector2 clamp( T min_x, T min_y, T max_x, T max_y ){
			return Vector2(
					math::clamp(x, min_x, max_x),
					math::clamp(y, min_y, max_y)
			);
		}

		Vector2 clamp( const Vector2& min, const Vector2& max ){
			return Vector2(
					math::clamp(x, min.x, max.x),
					math::clamp(y, min.y, max.y)
			);
		}

		float ratio() const { return static_cast<float>(x) / static_cast<float>(y); }

		const union {
			struct {
				T x;
				T y;
			};
			struct {
				T width;
				T height;
			};
			T v[2];
		};
};

template<>
inline double Vector2<double>::length() const {
	return sqrt(x*x + y*y);
}

typedef Vector2<unsigned int> Vector2ui;
typedef Vector2<float> Vector2f;

#endif /* VECTOR_H */
