#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>
#include <fstream>

Matrix4 ScaleMatrix4(Matrix4 matWorld, Vector3 scale);
Matrix4 RotationYMatrix4(Matrix4 matWorld, Vector3 rotation);
Matrix4 MoveMatrix4(Matrix4 matWorld, Vector3 translation);
Vector3 HalfwayPoint(Vector3 A, Vector3 B, Vector3 C, Vector3 D, float t);

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
	delete player_;
	delete modelSkydome_;
	delete debugCamera_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();
	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	textureHandle_ = TextureManager::Load("mario.jpg");
	textureHandle2_ = TextureManager::Load("enemy.jpg");
	model_ = Model::Create();

	// ----- ベジエ用 ----- //

	bezierMode = false;

	boomerang_.rotation_ = { 0.0f,0.0f,0.0f };
	boomerang_.translation_ = { 0.0f,0.0f,0.0f };
	boomerang_.scale_ = { 1.0f,0.3f,0.3f };
	boomerang_.Initialize();

	//ブーメラン横幅
	targetSide1.rotation_ = { 0.0f,0.0f,0.0f };
	targetSide1.translation_ = { 0.0f,0.0f,0.0f };
	targetSide1.scale_ = { 1.0f,1.0f,1.0f };
	targetSide1.Initialize();
	targetSide1.matWorld_ = MathUtility::Matrix4Identity();

	targetSide2.rotation_ = { 0.0f,0.0f,0.0f };
	targetSide2.translation_ = { 0.0f,0.0f,0.0f };
	targetSide2.scale_ = { 1.0f,1.0f,1.0f };
	targetSide2.Initialize();
	targetSide2.matWorld_ = MathUtility::Matrix4Identity();

	// ----- ベジエ用 ----- //

	//カメラターゲット
	target_.rotation_ = { 0.0f,0.0f,0.0f };
	target_.translation_ = { 0.0f,0.0f,0.0f };
	target_.scale_ = { 1.0f,1.0f,1.0f };
	target_.Initialize();
	target_.matWorld_ = MathUtility::Matrix4Identity();

	//カメラ
	viewProjection_.eye = { 0.0f, 0.0f, -10.0f };
	viewProjection_.target = target_.translation_;

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();

	//自キャラの生成
	player_ = new Player();
	//自キャラの初期化
	player_->Initialize(model_, textureHandle_);

	//スカイドームの生成
	skydome_ = new Skydome();
	//3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	//スカイドームの初期化
	skydome_->Initialize(modelSkydome_);

	//ファイルの読み込み
	LoadEnemyPopData();
}

void GameScene::Update() {

	const float rSpeed = 0.02f;

	if (input_->PushKey(DIK_D)) {
		target_.rotation_.y += rSpeed;
	}
	else if (input_->PushKey(DIK_A)) {
		target_.rotation_.y += -rSpeed;
	}

	//単位ベクトルの向き
	Vector3 direction = { sinf(target_.rotation_.y), 0, cosf(target_.rotation_.y) };

	//ブーメラン横幅調整用変数
	float boomerangWidth = 0.8f;

	// ----- ベクトル ----- //
	Vector3 frontV = Vector3(
		viewProjection_.eye.x - viewProjection_.target.x,
		viewProjection_.eye.y - viewProjection_.target.y,
		viewProjection_.eye.z - viewProjection_.target.z);

	Vector3 sideV = Vector3(
		boomerangWidth * frontV.z, 0, -boomerangWidth * frontV.x);

	//カメラターゲット
	target_.matWorld_ = MathUtility::Matrix4Identity();

	target_.matWorld_ = RotationYMatrix4(target_.matWorld_, target_.rotation_);

	//ブーメラン横幅
	targetSide1.translation_ = { -sideV.x, target_.translation_.y + 3,-sideV.z };
	targetSide2.translation_ = { sideV.x ,target_.translation_.y + 3,sideV.z };

	targetSide1.matWorld_ = MathUtility::Matrix4Identity();
	targetSide1.matWorld_ = MoveMatrix4(targetSide1.matWorld_, targetSide1.translation_);

	targetSide2.matWorld_ = MathUtility::Matrix4Identity();
	targetSide2.matWorld_ = MoveMatrix4(targetSide2.matWorld_, targetSide2.translation_);

	//カメラ
	viewProjection_.eye.x = -direction.x * 80 + target_.translation_.x;

	viewProjection_.eye.z = -direction.z * 80 + target_.translation_.z;

	viewProjection_.target = { direction.x * 5 + target_.translation_.x, 0, direction.z * 5 + target_.translation_.z };

	////デスフラグの立った弾の削除
	//enemybullets_.remove_if([](std::unique_ptr<EnemyBullet>& bullet) { return bullet->InDead(); });

	//自キャラの更新
	player_->Update();

	//敵発生
	UpdataEnemyPopCommands();

	//敵の更新
	for (std::unique_ptr<Enemy>& enemy_ : enemys_) {
		enemy_->SetGameScene(this);
		enemy_->Update();
	}

	////弾更新
	////複数
	//for (std::unique_ptr<EnemyBullet>& bullet : enemybullets_) {
	//	bullet->Update();
	//}

	// ----- ベジエ ここから ----- //

	// BP = Boomerang Point
	Vector3 BP1 = { viewProjection_.eye.x, viewProjection_.eye.y,viewProjection_.eye.z };

	//通常ブーメラン
	Vector3 BP2 = targetSide1.translation_;
	Vector3 BP3 = targetSide2.translation_;

	//戻りブメ
	/*Vector3 BP2 = { viewProjection_.target.x, viewProjection_.target.y + 3, viewProjection_.target.z};
	Vector3 BP3 = BP2;*/

	Vector3 BP4 = BP1;

	if (input_->PushKey(DIK_SPACE))
	{
		bezierMode = TRUE;
	}
	if (bezierMode == TRUE)
	{
		timer++;
		t = (1.0 / splitNum) * timer;
		boomerang_.rotation_.y++;
		if (timer >= splitNum)
		{
			boomerang_.rotation_.y = 0;
			timer = 0;
			bezierMode = FALSE;
		}
	}

	//ベジェ関数
	boomerang_.translation_ = HalfwayPoint(BP1, BP2, BP3, BP4, t);

	//ブーメランの行列
	boomerang_.matWorld_ = MathUtility::Matrix4Identity();
	boomerang_.matWorld_ = ScaleMatrix4(boomerang_.matWorld_, boomerang_.scale_);
	boomerang_.matWorld_ = RotationYMatrix4(boomerang_.matWorld_, boomerang_.rotation_);
	boomerang_.matWorld_ = MoveMatrix4(boomerang_.matWorld_, boomerang_.translation_);

	boomerang_.TransferMatrix();

	target_.TransferMatrix();

	targetSide1.TransferMatrix();
	targetSide2.TransferMatrix();

	// ---- ベジエ関係 ここまで ---- //

	//行列の再計算
	viewProjection_.UpdateMatrix();

	CheckAllCollisions();

	BoomerangCollision(boomerang_);

	//デバッグ用表示
#pragma region debugText
	debugText_->SetPos(50, 70);
	debugText_->Printf(
		"target:(%f,%f,%f)", target_.rotation_.x, target_.rotation_.y,
		target_.rotation_.z);

	debugText_->SetPos(50, 90);
	debugText_->Printf(
		"up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);

	debugText_->SetPos(50, 110);
	debugText_->Printf("%d", enemys_.size());

	debugText_->SetPos(200, 0);
	debugText_->Printf("%d", Hit);

#pragma endregion
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	skydome_->Draw(viewProjection_);
	for (std::unique_ptr<Enemy>& enemy_ : enemys_) {
		if (enemy_->Life() == FALSE)
		{
			enemy_->Draw(viewProjection_);
		}
	}

	model_->Draw(boomerang_, viewProjection_, textureHandle_);

	/*for (int i = 0; i < 100; i++)
	{
		model_->Draw(route_[i], viewProjection_, textureHandle2_);
	}*/

	//ベジェ(点2と点3の描画)目安がわかるよう
	model_->Draw(targetSide1, viewProjection_, textureHandle_);
	model_->Draw(targetSide2, viewProjection_, textureHandle_);

	/*for (std::unique_ptr<EnemyBullet>& bullet : enemybullets_) {
		bullet->Draw(viewProjection_);
	}*/

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::CheckAllCollisions() {

	//判定対象AとBの座標
	Vector3 posA, posB;

	//自弾リストの取得
	const std::list<std::unique_ptr<PlayerBullet>>& playerBullets = player_->GetBullets();
	//敵弾リストの取得
	//const std::list<std::unique_ptr<EnemyBullet>>& enemyBullets = GetBullets();

#pragma region 自キャラと敵弾の当たり判定
	//自キャラの座標
	posA = player_->GetWorldPosition2();

	//コメントアウト
	{
		//自キャラと敵弾すべての当たり判定
		//for (const std::unique_ptr<EnemyBullet>& bullet : enemyBullets) {
		//	//敵弾の座標
		//	posB = bullet->GetWorldPosition();

		//	float x = posB.x - posA.x;
		//	float y = posB.y - posA.y;
		//	float z = posB.z - posA.z;

		//	float cd = sqrt(x * x + y * y + z * z);

		//	if (cd <= playerRadius + enemyBulletRadius) {
		//		//自キャラの衝突時コールバックを呼び出す
		//		player_->OnCollision();
		//		//敵弾の衝突時コールバックを呼び出す
		//		bullet->OnCollision();
		//	}
		//}
	}
#pragma endregion

#pragma region 自弾と敵キャラの当たり判定
	//敵キャラの座標
	for (std::unique_ptr<Enemy>& enemy_ : enemys_) {
		posA = enemy_->GetWorldPosition();

		//自弾と敵キャラの当たり判定
		for (const std::unique_ptr<PlayerBullet>& bullet : playerBullets) {
			//自弾の座標
			posB = bullet.get()->GetWorldPosition();

			float x = posB.x - posA.x;
			float y = posB.y - posA.y;
			float z = posB.z - posA.z;

			float cd = sqrt(x * x + y * y + z * z);

			if (cd <= playerRadius + enemyBulletRadius) {
				//敵キャラの衝突時コールバックを呼び出す
				enemy_->OnCollision();
				//自弾の衝突時コールバックを呼び出す
				bullet->OnCollision();
			}
		}
	}

#pragma endregion

#pragma region 自弾と敵弾の当たり判定
	{
		//自キャラと敵弾すべての当たり判定
		//for (const std::unique_ptr<PlayerBullet>& bullet : playerBullets) {
		//	for (const std::unique_ptr<EnemyBullet>& bullet2 : enemyBullets) {

		//		//自弾の座標
		//		posA = bullet.get()->GetWorldPosition();

		//		//敵弾の座標
		//		posB = bullet2.get()->GetWorldPosition();

		//		float x = posB.x - posA.x;
		//		float y = posB.y - posA.y;
		//		float z = posB.z - posA.z;

		//		float cd = sqrt(x * x + y * y + z * z);

		//		if (cd <= playerBulletRadius + enemyBulletRadius) {
		//			//自キャラの衝突時コールバックを呼び出す
		//			bullet->OnCollision();
		//			//敵弾の衝突時コールバックを呼び出す
		//			bullet2->OnCollision();
		//		}
		//	}
		//}
	}
#pragma endregion
}

void GameScene::AddEnemyBullet(std::unique_ptr<EnemyBullet>& enemyBullet) {
	//リストに登録する
	//enemybullets_.push_back(std::move(enemyBullet));
}

void GameScene::LoadEnemyPopData() {

	//ファイルを開く
	std::ifstream file;
	file.open("Resources/enemyPop2.csv");

	assert(file.is_open());

	//ファイルの内容を文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	//ファイルを閉じる
	file.close();

}

void GameScene::UpdataEnemyPopCommands() {

	//待機処理
	if (isStand_) {
		standTime_--;
		if (standTime_ <= 0) {
			//待機完了
			isStand_ = false;
		}
		return;
	}
	// 1行分の文字列を入れる変数
	std::string line;

	//コマンド実行ループ
	while (getline(enemyPopCommands, line)) {
		// 1行分の文字数をストリームに変換して解折しやすくなる
		std::istringstream line_stream(line);

		std::string word;
		//,区切りで行の先頭文字を取得
		getline(line_stream, word, ',');

		//"//"から始まる行はコメント
		if (word.find("//") == 0) {
			//コメント行を飛ばす
			continue;
		}
		// POPコマンド
		if (word.find("POP") == 0) {
			// X座標
			std::getline(line_stream, word, ',');
			float x = static_cast<float>(std::atof(word.c_str()));

			// Y座標
			std::getline(line_stream, word, ',');
			float y = static_cast<float>(std::atof(word.c_str()));

			// Z座標
			std::getline(line_stream, word, ',');
			float z = static_cast<float>(std::atof(word.c_str()));

			GenerEnemy(Vector3(x, y, z));
		}
		// WAITコマンド
		else if (word.find("WAIT") == 0) {
			std::getline(line_stream, word, ',');

			//待ち時間
			int32_t waitTime = std::atoi(word.c_str());

			//待機開始
			isStand_ = true;
			standTime_ = waitTime;

			//ループを抜ける
			break;
		}
	}

}

void GameScene::GenerEnemy(Vector3 EnemyPos) {
	//敵キャラの生成
	std::unique_ptr<Enemy> newEnemy = std::make_unique<Enemy>();
	//敵キャラの初期化
	newEnemy->Initialize(model_, textureHandle2_, EnemyPos);

	//敵キャラにアドレスを渡す
	newEnemy->SetPlayer(player_);

	//リストに登録する
	enemys_.push_back(std::move(newEnemy));
}

Vector3 GameScene::vector3(float x, float y, float z) { return Vector3(x, y, z); }

Vector4 GameScene::vector4(int x, int y, int z, int w) { return Vector4(x, y, z, w); }

//行列計算用関数
Matrix4 ScaleMatrix4(Matrix4 matWorld, Vector3 scale)
{
	Matrix4 matScale = MathUtility::Matrix4Identity();

	matScale.m[0][0] = scale.x;
	matScale.m[1][1] = scale.y;
	matScale.m[2][2] = scale.z;

	return matWorld *= matScale;
}

Matrix4 RotationXMatrix4(Matrix4 matWorld, Vector3 rotation)
{
	Matrix4 matRotX = MathUtility::Matrix4Identity();

	matRotX.m[1][1] = cosf(rotation.x);
	matRotX.m[1][2] = sinf(rotation.x);
	matRotX.m[2][1] = -sinf(rotation.x);
	matRotX.m[2][2] = cosf(rotation.x);

	return matWorld *= matRotX;
}

Matrix4 RotationYMatrix4(Matrix4 matWorld, Vector3 rotation)
{
	Matrix4 matRotY = MathUtility::Matrix4Identity();

	matRotY.m[0][0] = cosf(rotation.y);
	matRotY.m[0][2] = -sinf(rotation.y);
	matRotY.m[2][0] = sinf(rotation.y);
	matRotY.m[2][2] = cosf(rotation.y);

	return matWorld *= matRotY;
}

Matrix4 RotationZMatrix4(Matrix4 matWorld, Vector3 rotation)
{
	Matrix4 matRotZ = MathUtility::Matrix4Identity();

	matRotZ.m[0][0] = cosf(rotation.z);
	matRotZ.m[0][1] = sinf(rotation.z);
	matRotZ.m[1][0] = -sinf(rotation.z);
	matRotZ.m[1][1] = cosf(rotation.z);

	return matWorld *= matRotZ;
}

Matrix4 MoveMatrix4(Matrix4 matWorld, Vector3 translation)
{
	Matrix4 matTrans = MathUtility::Matrix4Identity();

	matTrans.m[3][0] = translation.x;
	matTrans.m[3][1] = translation.y;
	matTrans.m[3][2] = translation.z;

	return matWorld *= matTrans;
}

//ベジェ計算用関数
Vector3 HalfwayPoint(Vector3 A, Vector3 B, Vector3 C, Vector3 D, float t)
{
	// ----- 第1中間点 ----- //

	Vector3 AB = { 0, 0, 0 };

	AB.x = ((1 - t) * A.x + t * B.x);
	AB.y = ((1 - t) * A.y + t * B.y);
	AB.z = ((1 - t) * A.z + t * B.z);

	Vector3 BC = { 0, 0, 0 };

	BC.x = ((1 - t) * B.x + t * C.x);
	BC.y = ((1 - t) * B.y + t * C.y);
	BC.z = ((1 - t) * B.z + t * C.z);

	Vector3 CD = { 0, 0, 0 };

	CD.x = ((1 - t) * C.x + t * D.x);
	CD.y = ((1 - t) * C.y + t * D.y);
	CD.z = ((1 - t) * C.z + t * D.z);

	// ----- 第2中間点 ----- //

	Vector3 AC = { 0, 0, 0 };

	AC.x = ((1 - t) * AB.x + t * BC.x);
	AC.y = ((1 - t) * AB.y + t * BC.y);
	AC.z = ((1 - t) * AB.z + t * BC.z);

	Vector3 BD = { 0, 0, 0 };

	BD.x = ((1 - t) * BC.x + t * CD.x);
	BD.y = ((1 - t) * BC.y + t * CD.y);
	BD.z = ((1 - t) * BC.z + t * CD.z);

	// ----- ベジエ本体座標 ----- //

	Vector3 AD = { 0, 0, 0 };

	AD.x = ((1 - t) * AC.x + t * BD.x);
	AD.y = ((1 - t) * AC.y + t * BD.y);
	AD.z = ((1 - t) * AC.z + t * BD.z);

	return AD;

}

void GameScene::BoomerangCollision(WorldTransform boomerang)
{
	//判定対象AとBの座標
	Vector3 posA, posB;

	//ワールド座標を入れる変数
	Vector3 wBoomerangPos;

	//ワールド行列の平行移動成分
	wBoomerangPos.x = boomerang.matWorld_.m[3][0];
	wBoomerangPos.y = boomerang.matWorld_.m[3][1];
	wBoomerangPos.z = boomerang.matWorld_.m[3][2];

	//敵キャラの座標
	for (std::unique_ptr<Enemy>& enemy_ : enemys_) {
		posA = enemy_->GetWorldPosition();

		//自弾の座標
		posB = wBoomerangPos;

		float x = posB.x - posA.x;
		float y = posB.y - posA.y;
		float z = posB.z - posA.z;

		float cd = sqrt(x * x + y * y + z * z);

		if (cd <= 5 + enemyBulletRadius) {
			//敵キャラの衝突時コールバックを呼び出す
			enemy_->OnCollision();
			Hit = 1;
		}
	}
}