#pragma once
#include "DebugText.h"
#include "Input.h"
#include "ViewProjection.h"
#include <Model.h>
#include <WorldTransform.h>
#include <affin.h>
#include <cassert>
#include "EnemyBullet.h"
#include <list>
class Player;

class GameScene;

class Enemy {
  public:
	///< summary>
	///初期化
	///</summary>
	void Initialize(Model* model, uint32_t textureHandle, Vector3 vector3);

	///< summary>
	///初期化
	///</summary>
	void Update();

	///< summary>
	///初期化
	///</summary>
	void Draw(ViewProjection viewProjection_);


	//発射隔離
	static const int kFireInterval = 60;
	
	void SetPlayer(Player* player) { player_ = player;}

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	////ワールド座標を取得
	Vector3 GetWorldPosition();

	//衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	bool Life();

  private:
	//ワールド変換データ
	WorldTransform worldTransform_;
	//モデル
	Model* model_ = nullptr;
	//テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	Input* input_ = nullptr;
	DebugText* debugText_ = nullptr;

	//自キャラ
	Player* player_ = nullptr;

	//デスフラグ
	bool isDead_ = false;

	//発射タイマー
	int32_t shotTimer = 0;

	//ゲームシーン
	GameScene* gameScene_ = nullptr;

	//敵の挙動フラグ
	int isChangeFlag = 0;

};
