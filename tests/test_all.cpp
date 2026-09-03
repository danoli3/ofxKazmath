// ofxKazmath all-in-one test: core kazmath sanity + ofx converter round-trips.
//
// Build from the addon root (OF_ROOT = your openFrameworks checkout).
// The kazmath C sources must be compiled as C (GL/matrix.c relies on
// implicit void* conversion), everything else as C++:
//   mkdir -p /tmp/kazobjs
//   for f in libs/kazmath/src/*.c libs/kazmath/src/GL/*.c; do
//     cc -c -I libs/kazmath/include -I libs/kazmath/include/kazmath \
//        -I libs/kazmath/include/kazmath/GL \
//        -o /tmp/kazobjs/$(basename $f .c).o $f
//   done
//   c++ -std=c++14 tests/test_all.cpp /tmp/kazobjs/*.o \
//       $OF_ROOT/libs/openFrameworks/math/ofMatrix3x3.cpp \
//       $OF_ROOT/libs/openFrameworks/math/ofMatrix4x4.cpp \
//       $OF_ROOT/libs/openFrameworks/math/ofQuaternion.cpp \
//       $OF_ROOT/libs/openFrameworks/math/ofVec2f.cpp \
//       $OF_ROOT/libs/openFrameworks/math/ofVec4f.cpp \
//       -I src -I libs/kazmath/include \
//       -I $OF_ROOT/libs/openFrameworks/math -I $OF_ROOT/libs/glm/include \
//       -o /tmp/test_all && /tmp/test_all

#include "ofxKazmath.h"

#include <cmath>
#include <cstdio>

static int checks = 0;
static int failures = 0;

#define CHECK(cond) do { \
	++checks; \
	if(!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } \
} while(0)

#define CHECK_CLOSE(a, b) CHECK(fabs((double)(a) - (double)(b)) < 1e-5)

static kmVec3 makeVec3(kmScalar x, kmScalar y, kmScalar z) {
	kmVec3 v;
	v.x = x; v.y = y; v.z = z;
	return v;
}

// ---- core kazmath ----

static void testUtility() {
	CHECK_CLOSE(kmDegreesToRadians(180), M_PI);
	CHECK_CLOSE(kmRadiansToDegrees((kmScalar)M_PI), 180);
}

static void testMat4() {
	kmMat4 m;
	CHECK(kmMat4Identity(&m) != NULL);
	CHECK(kmMat4IsIdentity(&m));

	kmMat4 t;
	CHECK(kmMat4Translation(&t, 5, 6, 7) != NULL);
	CHECK_CLOSE(t.mat[12], 5);
	CHECK_CLOSE(t.mat[13], 6);
	CHECK_CLOSE(t.mat[14], 7);
	CHECK_CLOSE(t.mat[15], 1);

	kmMat4 inv;
	CHECK(kmMat4Inverse(&inv, &t) != NULL);
	kmMat4 prod;
	kmMat4Multiply(&prod, &t, &inv);
	CHECK(kmMat4IsIdentity(&prod));

	// axis-angle and quaternion rotations must agree (mirrors upstream test)
	kmVec3 yAxis = makeVec3(0, 1, 0);
	kmQuaternion quat;
	kmQuaternionRotationAxisAngle(&quat, &yAxis, kmDegreesToRadians(90));
	kmMat4 fromAxis, fromQuat;
	kmMat4RotationAxisAngle(&fromAxis, &yAxis, kmDegreesToRadians(90));
	kmMat4RotationQuaternion(&fromQuat, &quat);
	CHECK(kmMat4AreEqual(&fromAxis, &fromQuat));

	kmMat4 id, id2;
	kmMat4Identity(&id);
	kmMat4Identity(&id2);
	CHECK(kmMat4AreEqual(&id, &id2));
	CHECK(!kmMat4AreEqual(&id, &t));
}

static void testMat3() {
	kmMat3 m;
	CHECK(kmMat3Identity(&m) != NULL);
	CHECK_CLOSE(m.mat[0], 1);
	CHECK_CLOSE(m.mat[4], 1);
	CHECK_CLOSE(m.mat[8], 1);
	CHECK_CLOSE(m.mat[1], 0);

	kmMat4 id4;
	kmMat4Identity(&id4);
	kmMat3 rot;
	kmMat4ExtractRotationMat3(&id4, &rot);
	CHECK_CLOSE(rot.mat[0], 1);
	CHECK_CLOSE(rot.mat[4], 1);
	CHECK_CLOSE(rot.mat[8], 1);
}

static void testVecs() {
	kmVec2 a2;
	a2.x = 3; a2.y = 4;
	CHECK_CLOSE(kmVec2Length(&a2), 5);
	kmVec2 b2;
	b2.x = 1; b2.y = 0;
	CHECK_CLOSE(kmVec2Dot(&a2, &b2), 3);

	kmVec3 x = makeVec3(1, 0, 0);
	kmVec3 y = makeVec3(0, 1, 0);
	kmVec3 z;
	kmVec3Cross(&z, &x, &y);
	CHECK_CLOSE(z.x, 0); CHECK_CLOSE(z.y, 0); CHECK_CLOSE(z.z, 1);

	kmVec3 v = makeVec3(0, 0, 5);
	kmVec3 n;
	kmVec3Normalize(&n, &v);
	CHECK_CLOSE(kmVec3Length(&n), 1);
	CHECK_CLOSE(kmVec3Dot(&v, &n), 5);

	kmVec4 p, q, sum;
	p.x = 1; p.y = 2; p.z = 3; p.w = 4;
	q.x = 4; q.y = 3; q.z = 2; q.w = 1;
	kmVec4Add(&sum, &p, &q);
	CHECK_CLOSE(sum.x, 5); CHECK_CLOSE(sum.y, 5);
	CHECK_CLOSE(sum.z, 5); CHECK_CLOSE(sum.w, 5);
}

static void testQuaternion() {
	kmQuaternion q;
	CHECK(kmQuaternionIdentity(&q) != NULL);
	CHECK_CLOSE(q.w, 1);

	kmQuaternion r, prod;
	kmVec3 yAxis = makeVec3(0, 1, 0);
	kmQuaternionRotationAxisAngle(&r, &yAxis, kmDegreesToRadians(90));
	kmQuaternionMultiply(&prod, &q, &r);
	CHECK_CLOSE(prod.x, r.x); CHECK_CLOSE(prod.y, r.y);
	CHECK_CLOSE(prod.z, r.z); CHECK_CLOSE(prod.w, r.w);
}

static void testPlaneAabbRay() {
	kmPlane plane; // plane y = 5
	kmPlaneFill(&plane, 0, 1, 0, -5);
	kmVec3 above = makeVec3(0, 10, 0);
	kmVec3 below = makeVec3(0, 0, 0);
	kmVec3 on = makeVec3(0, 5, 0);
	CHECK(kmPlaneClassifyPoint(&plane, &above) == POINT_INFRONT_OF_PLANE);
	CHECK(kmPlaneClassifyPoint(&plane, &below) == POINT_BEHIND_PLANE);
	CHECK(kmPlaneClassifyPoint(&plane, &on) == POINT_ON_PLANE);

	kmAABB3 box;
	box.min = makeVec3(-1, -1, -1);
	box.max = makeVec3(1, 1, 1);
	kmVec3 origin = makeVec3(0, 0, 0);
	kmVec3 outside = makeVec3(2, 0, 0);
	CHECK(kmAABB3ContainsPoint(&box, &origin));
	CHECK(!kmAABB3ContainsPoint(&box, &outside));

	kmRay3 ray;
	ray.start = makeVec3(0, 0, 5);
	// dir is treated as a segment: its length must reach the triangle
	ray.dir = makeVec3(0, 0, -10);
	kmVec3 v0 = makeVec3(-1, -1, 0);
	kmVec3 v1 = makeVec3(1, -1, 0);
	kmVec3 v2 = makeVec3(0, 1, 0);
	kmVec3 hit, normal;
	kmScalar dist = 0;
	CHECK(kmRay3IntersectTriangle(&ray, &v0, &v1, &v2, &hit, &normal, &dist));
	CHECK_CLOSE(dist, 5);
}

// ---- ofx converters: oF types are row-major, kazmath is column-major ----

static void testConverterMat4() {
	ofMatrix4x4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
	kmMat4 k = toKM(m);
	for(int r = 0; r < 4; ++r)
		for(int c = 0; c < 4; ++c)
			CHECK_CLOSE(k.mat[c * 4 + r], r * 4 + c + 1);
	ofMatrix4x4 back = toOF(k);
	for(int r = 0; r < 4; ++r)
		for(int c = 0; c < 4; ++c)
			CHECK_CLOSE(back(r, c), m(r, c));

	// independent oracle: oF translation matrix must equal kmMat4Translation
	ofMatrix4x4 t(1, 0, 0, 5,
				  0, 1, 0, 6,
				  0, 0, 1, 7,
				  0, 0, 0, 1);
	kmMat4 kt = toKM(t), ref;
	kmMat4Translation(&ref, 5, 6, 7);
	for(int i = 0; i < 16; ++i)
		CHECK_CLOSE(kt.mat[i], ref.mat[i]);
}

static void testConverterMat3() {
	ofMatrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
	kmMat3 k = toKM(m);
	for(int r = 0; r < 3; ++r)
		for(int c = 0; c < 3; ++c)
			CHECK_CLOSE(k.mat[c * 3 + r], r * 3 + c + 1);
	ofMatrix3x3 back = toOF(k);
	const float *e = &back.a;
	for(int i = 0; i < 9; ++i)
		CHECK_CLOSE(e[i], i + 1);
}

static void testConverterVecQuat() {
	ofVec2f v2(1, 2);
	ofVec2f b2 = toOF(toKM(v2));
	CHECK_CLOSE(b2.x, 1); CHECK_CLOSE(b2.y, 2);

	ofVec3f v3(1, 2, 3);
	ofVec3f b3 = toOF(toKM(v3));
	CHECK_CLOSE(b3.x, 1); CHECK_CLOSE(b3.y, 2); CHECK_CLOSE(b3.z, 3);

	ofVec4f v4(1, 2, 3, 4);
	ofVec4f b4 = toOF(toKM(v4));
	CHECK_CLOSE(b4.x, 1); CHECK_CLOSE(b4.y, 2);
	CHECK_CLOSE(b4.z, 3); CHECK_CLOSE(b4.w, 4);

	ofQuaternion q(0.1f, 0.2f, 0.3f, 0.9f);
	ofQuaternion bq = toOF(toKM(q));
	CHECK_CLOSE(bq.x(), q.x()); CHECK_CLOSE(bq.y(), q.y());
	CHECK_CLOSE(bq.z(), q.z()); CHECK_CLOSE(bq.w(), q.w());
}

int main() {
	testUtility();
	testMat4();
	testMat3();
	testVecs();
	testQuaternion();
	testPlaneAabbRay();
	testConverterMat4();
	testConverterMat3();
	testConverterVecQuat();

	printf("%d checks, %d failures\n", checks, failures);
	if(failures == 0) printf("ALL TESTS PASS\n");
	return failures != 0;
}
