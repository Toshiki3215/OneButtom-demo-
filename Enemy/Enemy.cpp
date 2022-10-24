#include "Enemy.h"
#include "Player/Player.h"
#include <GameScene.h>
#include <cmath>

void Enemy::Initialize(Model* model, uint32_t textureHandle, Vector3 vector3) {
	// NULLポインタチェック
	assert(model);
	model_ = model;
	textureHandle_ = textureHandle;

	//シングルインスタンスを取得する
	input_ = Input::GetInstance();
	debugText_ = DebugText::GetInstance();

	//ワールド変換の初期化
	worldTransform_.Initialize();

	//初期座標をセット
	worldTransform_.translation_ = vector3;
}

void Enemy::Update() {

	//敵の移動の速さ
	const float kCharacterSpeed = 0.09f;
	const float kCharacterSpeed2 = 0.1f;

	//行列更新
	AffinTrans::affin(worldTransform_);

	worldTransform_.TransferMatrix();

	//移動(ベクトルを加算)
	if (isChangeFlag == 0) {
		worldTransform_.translation_ += {kCharacterSpeed, 0, 0};
	}
	else if (isChangeFlag == 1) {
		worldTransform_.translation_ += {-kCharacterSpeed2, 0, 0};
	}

	if (worldTransform_.translation_.x >= 8.0f && isChangeFlag == 0) {
		isChangeFlag = 1;
	}
	else if (worldTransform_.translation_.x <= -8.0f && isChangeFlag == 1) {
		isChangeFlag = 0;
	}

	debugText_->SetPos(50, 250);
	debugText_->Printf(
	  "Enemytranslation : %f,%f,%f", worldTransform_.translation_.x, worldTransform_.translation_.y,
	  worldTransform_.translation_.z);
}

void Enemy::Draw(ViewProjection viewProjection_) {
	model_->Draw(worldTransform_, viewProjection_, textureHandle_);
}

Vector3 Enemy::GetWorldPosition() { 
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void Enemy::OnCollision() { isDead_ = true; }

bool Enemy::Life() { return isDead_; }
