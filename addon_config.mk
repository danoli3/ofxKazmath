meta:
	ADDON_NAME = ofxKazmath
	ADDON_DESCRIPTION = openFrameworks wrapper for kazmath, a lightweight 3D math library in C (vectors, matrices, quaternions, planes, rays, AABBs).
	ADDON_AUTHOR = Dan Rosser
	ADDON_TAGS = "math" "3d" "vectors" "matrices" "quaternions"
	ADDON_URL = https://github.com/danoli3/ofxKazmath

common:
	# The kazmath sources use same-directory quoted includes (e.g.
	# #include "utility.h" from src/*.c and #include "matrix.h" from
	# src/GL/*.c), so every level of the split include tree must be
	# visible. Without include/kazmath and include/kazmath/GL here the
	# Makefile build resolves those headers to unrelated matrix.h files
	# from other addons and fails with unknown type name 'kmGLEnum'.
	ADDON_INCLUDES += libs/kazmath/include
	ADDON_INCLUDES += libs/kazmath/include/kazmath
	ADDON_INCLUDES += libs/kazmath/include/kazmath/GL
