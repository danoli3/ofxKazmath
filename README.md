# ofxKazmath

[openFrameworks](https://openframeworks.cc) addon wrapping [kazmath](https://github.com/Kazade/kazmath) — a lightweight 3D math library aimed at game programming, by Luke Benstead (with contributions from Carsten Haubold).

It gives you plain-C math types with no dependencies, handy when you want to do your own matrix / vector / quaternion math without pulling in a bigger library.

![ofxaddons thumbnail](ofxaddons_thumbnail.png)

## Features

Bundled kazmath provides:

- Vectors: `kmVec2`, `kmVec3`, `kmVec4`
- Matrices: `kmMat3`, `kmMat4` (incl. translation, rotation, scaling, look-at, perspective / orthographic projection)
- Quaternions: `kmQuaternion` (axis-angle, rotation matrices, slerp, multiply)
- Plane: `kmPlane` (classify points, intersect)
- Axis-aligned bounding box: `kmAABB`
- Rays: `kmRay2`, `kmRay3`
- OpenGL helpers: `kmGLPushMatrix` / `kmGLPopMatrix` / `kmGLLoadMatrix` / `kmGLMultMatrix` (`kazmath/GL/matrix.h`)

## Installation

Copy this folder into your openFrameworks `addons/` directory as `ofxKazmath`, then:

- **Project Generator**: create or update your project and add `ofxKazmath` to the addons list.
- **Manual**: add `libs/kazmath/include` to your header search paths and compile all `.c` files under `libs/kazmath/src`.

No external dependencies — kazmath is bundled under `libs/kazmath` and built as C.

## Usage

```cpp
#include "ofxKazmath.h"

kmMat4 translation;
kmMat4Translation(&translation, pos.x, pos.y, pos.z);

kmMat4 rotation;
kmVec3 axis = { pos.x, pos.y, pos.z };
kmMat4RotationAxisAngle(&rotation, &axis, kmDegreesToRadians(35.0f));

ofMultMatrix(&translation.mat[0]);
ofMultMatrix(&rotation.mat[0]);
ofDrawBox(boxSize);
```

See `example/` for a full demo: a noise-driven cloud of textured boxes transformed with `kmMat4Translation` and `kmMat4RotationAxisAngle`.

### Converters

`ofxKazmath.h` also provides `toKM` / `toOF` overloads converting between openFrameworks types (`ofMatrix4x4`, `ofMatrix3x3`, `ofVec2f/3f/4f`, `ofQuaternion`) and their kazmath counterparts. oF matrices are row-major, kazmath is column-major, so matrix conversion transposes — all converters are lossless round-trips:

```cpp
ofMatrix4x4 ofMat = /* ... */;
kmMat4 kmMat = toKM(ofMat);
ofMatrix4x4 back = toOF(kmMat); // == ofMat
```

## Compatibility

Plain C89 code, should build anywhere openFrameworks builds (macOS, Windows, Linux). Tested with the bundled Xcode example project on macOS.

## License

- Addon wrapper: same terms as the project (see `LICENSE`).
- kazmath itself is released under the modified BSD license, Copyright (c) 2008 Luke Benstead. See `libs/kazmath/README.txt` for the full text.
