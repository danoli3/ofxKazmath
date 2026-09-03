#pragma once

#include <kazmath/kazmath.h>

#include "ofMatrix3x3.h"
#include "ofMatrix4x4.h"
#include "ofQuaternion.h"
#include "ofVec2f.h"
#include "ofVec3f.h"
#include "ofVec4f.h"

// Converters between openFrameworks math types (row-major) and kazmath
// types (column-major, OpenGL order). Matrix conversion transposes; vector
// and quaternion conversion copies components. All are lossless round-trips:
// toOF(toKM(x)) == x.

inline kmMat4 toKM(const ofMatrix4x4 & m) {
	kmMat4 out;
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			out.mat[c * 4 + r] = static_cast<kmScalar>(m(r, c));
		}
	}
	return out;
}

inline ofMatrix4x4 toOF(const kmMat4 & m) {
	ofMatrix4x4 out;
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			out(r, c) = static_cast<float>(m.mat[c * 4 + r]);
		}
	}
	return out;
}

inline kmMat3 toKM(const ofMatrix3x3 & m) {
	// a..i are public and stored row-major; operator[] is non-const.
	const float elems[9] = { m.a, m.b, m.c, m.d, m.e, m.f, m.g, m.h, m.i };
	kmMat3 out;
	for(int r = 0; r < 3; ++r) {
		for(int c = 0; c < 3; ++c) {
			out.mat[c * 3 + r] = static_cast<kmScalar>(elems[r * 3 + c]);
		}
	}
	return out;
}

inline ofMatrix3x3 toOF(const kmMat3 & m) {
	ofMatrix3x3 out;
	out.set(static_cast<float>(m.mat[0]), static_cast<float>(m.mat[3]), static_cast<float>(m.mat[6]),
			static_cast<float>(m.mat[1]), static_cast<float>(m.mat[4]), static_cast<float>(m.mat[7]),
			static_cast<float>(m.mat[2]), static_cast<float>(m.mat[5]), static_cast<float>(m.mat[8]));
	return out;
}

inline kmVec2 toKM(const ofVec2f & v) {
	kmVec2 out;
	out.x = static_cast<kmScalar>(v.x);
	out.y = static_cast<kmScalar>(v.y);
	return out;
}

inline ofVec2f toOF(const kmVec2 & v) {
	return ofVec2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

inline kmVec3 toKM(const ofVec3f & v) {
	kmVec3 out;
	out.x = static_cast<kmScalar>(v.x);
	out.y = static_cast<kmScalar>(v.y);
	out.z = static_cast<kmScalar>(v.z);
	return out;
}

inline ofVec3f toOF(const kmVec3 & v) {
	return ofVec3f(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}

inline kmVec4 toKM(const ofVec4f & v) {
	kmVec4 out;
	out.x = static_cast<kmScalar>(v.x);
	out.y = static_cast<kmScalar>(v.y);
	out.z = static_cast<kmScalar>(v.z);
	out.w = static_cast<kmScalar>(v.w);
	return out;
}

inline ofVec4f toOF(const kmVec4 & v) {
	return ofVec4f(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), static_cast<float>(v.w));
}

inline kmQuaternion toKM(const ofQuaternion & q) {
	kmQuaternion out;
	out.x = static_cast<kmScalar>(q.x());
	out.y = static_cast<kmScalar>(q.y());
	out.z = static_cast<kmScalar>(q.z());
	out.w = static_cast<kmScalar>(q.w());
	return out;
}

inline ofQuaternion toOF(const kmQuaternion & q) {
	return ofQuaternion(static_cast<float>(q.x), static_cast<float>(q.y),
			static_cast<float>(q.z), static_cast<float>(q.w));
}
