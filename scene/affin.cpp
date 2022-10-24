#include "affin.h"
#include <cmath>

const float PI = 3.141592f;

Matrix4 AffinTrans::Initialize() {
	Matrix4 matInitialize = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f,                
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return matInitialize;
}

Matrix4 AffinTrans::Scale(Vector3 scale) {

	//スケーリング行列を宣言
	Matrix4 matScale = {
		scale.x, 0.0f, 0.0f,    0.0f, 
		0.0f, scale.y, 0.0f, 0.0f,
	    0.0f,    0.0f, scale.z, 0.0f, 
		0.0f, 0.0f,    0.0f, 1.0f
	};

	Matrix4 scaling = Initialize();
	scaling *= matScale;

	return scaling;
}

Matrix4 AffinTrans::Rotation(Vector3 rotation, int X_1_Y_2_Z_3_XYZ_6) {
	int rotationX = 1;
	int rotationY = 2;
	int rotationZ = 3;
	int rotationXYZ = 6;

	if (X_1_Y_2_Z_3_XYZ_6 == rotationX) {
		Matrix4 matRotX = {
		  1.0f,0.0f,0.0f,0.0f,
		  0.0f,cos(rotation.x),sin(rotation.x),0.0f,
		  0.0f,-sin(rotation.x),cos(rotation.x),0.0f,
		  0.0f,0.0f,0.0f,1.0f
		};
		Matrix4 rotationX = Initialize();
		rotationX *= matRotX;

		return rotationX;
	} else if (X_1_Y_2_Z_3_XYZ_6 == rotationY) {
		Matrix4 matRotY = {
			cos(rotation.y), 0.0f, -sin(rotation.y), 0.0f, 
			0.0f, 1.0f, 0.0f, 0.0f,
		    sin(rotation.y), 0.0f, cos(rotation.y),  0.0f, 
			0.0f, 0.0f, 0.0f, 1.0f
		};
		Matrix4 rotationY = Initialize();
		rotationY *= matRotY;

		return rotationY;
	} else if (X_1_Y_2_Z_3_XYZ_6 == rotationZ) {
		Matrix4 matRotZ = {
		  cos(rotation.z),sin(rotation.z),0.0f,0.0f,
		  -sin(rotation.z),cos(rotation.z),0.0f,0.0f,
		  0.0f,0.0f,1.0f,0.0f,
		  0.0f,0.0f,0.0f,1.0f
		};
		Matrix4 rotationZ = Initialize();
		rotationZ *= matRotZ;

		return rotationZ;
	} else {
		Matrix4 matRotXYZ;
		Matrix4 matRot_X, matRot_Y, matRot_Z;
		matRot_X = {
		  1.0f,0.0f,0.0f,0.0f,
		  0.0f,cos(rotation.x),sin(rotation.x),0.0f,
		  0.0f,-sin(rotation.x),cos(rotation.x),0.0f,
		  0.0f,0.0f,0.0f,1.0f
		};
		matRot_Y = {
			cos(rotation.y), 0.0f, -sin(rotation.y), 
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		    sin(rotation.y), 0.0f, cos(rotation.y),  
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		matRot_Z = {
		  cos(rotation.z),sin(rotation.z),0.0f,0.0f,
		  -sin(rotation.z),cos(rotation.z),0.0f,0.0f,
		  0.0f,0.0f,1.0f,0.0f,
		  0.0f,0.0f,0.0f,1.0f
		};
		//各軸の回転行列を合成
		matRotXYZ = Initialize();

		matRotXYZ *= matRot_X;
		matRotXYZ *= matRot_Y;
		matRotXYZ *= matRot_Z;

		return matRotXYZ;
	}
}

Matrix4 AffinTrans::Move(Vector3 Move) {
	Matrix4 matMove = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f,   1.0f,   0.0f,   0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f, 
	    Move.x, Move.y, Move.z, 1.0f
	};
	Matrix4 move = Initialize();
	move *= matMove;

	return move;
}

void AffinTrans::affin(WorldTransform& affin) { 
	affin.matWorld_ = Initialize();
	affin.matWorld_ *= Scale(affin.scale_);
	affin.matWorld_ *= Rotation(affin.rotation_,6);
	affin.matWorld_ *= Move(affin.translation_);
}

Vector3 AffinTrans::MatVector(const Vector3 vector3, const Matrix4 matrix4) { 
	
	Vector3 retVec = {};

	retVec.x =
	  vector3.x * matrix4.m[0][0] + vector3.y * matrix4.m[1][0] + vector3.z * matrix4.m[2][0];

	retVec.y =
	  vector3.x * matrix4.m[0][1] + vector3.y * matrix4.m[1][1] + vector3.z * matrix4.m[2][1];

	retVec.z =
	  vector3.x * matrix4.m[0][2] + vector3.y * matrix4.m[1][2] + vector3.z * matrix4.m[2][2];

	return retVec;
	


}

Vector3 AffinTrans::GetWorldtransform(const Matrix4 matrix4) { 
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分
	worldPos.x = matrix4.m[3][0];
	worldPos.y = matrix4.m[3][1];
	worldPos.z = matrix4.m[3][2];

	return worldPos;
}
