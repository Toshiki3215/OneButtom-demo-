#pragma once

#include <math/Matrix4.h>
#include <math/Vector3.h>
#include <3d/WorldTransform.h>

namespace AffinTrans {

Matrix4 Initialize();
Matrix4 Scale(Vector3 scale);
Matrix4 Rotation(Vector3 rotation, int X_1_Y_2_Z_3_XYZ_6);
Matrix4 Move(Vector3 move);
void  affin(WorldTransform& affin);

Vector3 MatVector(const Vector3 vector3, const Matrix4 matrix4);

Vector3 GetWorldtransform(const Matrix4 matrix4);
};

