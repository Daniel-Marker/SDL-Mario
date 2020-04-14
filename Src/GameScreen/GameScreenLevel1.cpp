#include "GameScreenLevel1.h"
#include <iostream>
#include "Texture2D.h"
#include "Collisions.h"
#include "PowBlock.h"

//Todo
//Fix characterplayable animate constants bug
//Level 2 stuff

//Get platform to animate when you hit it 
//Clean up music and sound effect code (especially stuff in source.cpp)
//Fix bug with gravity where player can fall through the sides of blocks
//Implement another enemy (get sprite from here: https://www.spriters-resource.com/arcade/mariobros/sheet/93677/)



GameScreenLevel1::GameScreenLevel1(SDL_Renderer* renderer, GameScreenManager* manager) :
	GameScreen(renderer), mLevelMap(NULL), mManager(manager)
{
	SetLevelMap();
	SetUpLevel();

	SetUpEnemyWaves();

	scoreFont = new Font(renderer, "Fonts/Press Start 2P.png", 32, 32, ' ');
}

GameScreenLevel1::~GameScreenLevel1()
{
	delete mBackgroundTexture;
	mBackgroundTexture = NULL;

	delete mMarioCharacter;
	mMarioCharacter = NULL;

	delete mLuigiCharacter;
	mLuigiCharacter = NULL;

	delete mPowBlock;
	mPowBlock = NULL;

	delete mLevelMap;
	mLevelMap = NULL;

	for (int i = 0; i < mEnemiesAndCoins.size(); i++)
		delete mEnemiesAndCoins[i];
	mEnemiesAndCoins.clear();
}

void GameScreenLevel1::Render()
{
	//Draw the background
	//mBackgroundTexture->Render(Vector2D(0, mBackgroundYPos), SDL_FLIP_NONE);

	//Draw the enemies
	for (unsigned int i = 0; i < mEnemiesAndCoins.size(); i++)
	{
		mEnemiesAndCoins[i]->Render();
	}

	//Draw the character
	if(mMarioCharacter->GetState() != PLAYER_DEATH)
		mMarioCharacter->Render();
	if(mLuigiCharacter->GetState() != PLAYER_DEATH)
		mLuigiCharacter->Render();

	mPowBlock->Render();

	mMarioCharacter->RenderScoreAndLives(scoreFont);
	mLuigiCharacter->RenderScoreAndLives(scoreFont);

	mLevelMap->Render(mBackgroundYPos);
}

void GameScreenLevel1::Update(float deltaTime, SDL_Event e)
{
	if (mScreenshake)
	{
		mScreenshakeTime -= deltaTime;
		mWobble++;
		mBackgroundYPos = 3.0f * sin(mWobble);

		//End the shake after the duration
		if (mScreenshakeTime <= 0.0f)
		{
			mScreenshake = false;
			mBackgroundYPos = 0.0f;
		}
	}

	//Spawn in any new enemies if necessary and increment mCurrentWave if the wave is over
	HandleEnemyWave(deltaTime);

	//Update enemies and coins
	UpdateEnemiesAndCoins(deltaTime, e);

	//Update the player
	if(mMarioCharacter->GetState() != PLAYER_DEATH)
		mMarioCharacter->Update(deltaTime, e);
	if(mLuigiCharacter->GetState() != PLAYER_DEATH)
		mLuigiCharacter->Update(deltaTime, e);

	if (Collisions::Instance()->Circle(mMarioCharacter->GetCollisionCircle(), mLuigiCharacter->GetCollisionCircle()) &&
		(mMarioCharacter->GetState() != PLAYER_DEATH && mLuigiCharacter->GetState() != PLAYER_DEATH))
	{
		mMarioCharacter->OnPlayerCollision(mLuigiCharacter);
		mLuigiCharacter->OnPlayerCollision(mMarioCharacter);
	}

	UpdatePOWBlock();

	if ((mMarioCharacter->GetState() == PLAYER_DEATH && mLuigiCharacter->GetState() == PLAYER_DEATH) || mCurrentWave > mEnemyWaves.size() - 1)
	{
		ScoreManager::Instance()->SetPlayerScore(mMarioCharacter->GetScore() + mLuigiCharacter->GetScore());
		mManager->ChangeScreen(SCREEN_GAMEOVER);
	}

	SoundList::Instance()->Update();
}

void GameScreenLevel1::HandleEnemyWave(float deltaTime)
{
	std::vector<WaveComponent> current = mEnemyWaves[mCurrentWave].GetWave();
	bool allSpawned = true;
	for (int i = 0; i < current.size(); i++)
	{
		if (!current[i].spawned)
		{
			allSpawned = false;

			current[i].spawnDelay -= deltaTime;

			if (current[i].spawnDelay <= 0.0f)
			{
				current[i].spawned = true;

				switch (current[i].enemy)
				{
				case KOOPA:
					CreateKoopa(current[i].position, current[i].direction, current[i].speed);
					break;

				case COIN:
					CreateCoin(current[i].position, current[i].direction, current[i].speed);
					break;
				};
			}
		}
	}
	mEnemyWaves[mCurrentWave].SetWave(current);

	if (allSpawned && mEnemiesAndCoins.size() == 0)
		mCurrentWave++;
}

bool GameScreenLevel1::SetUpLevel()
{
	//Load the background texture
	mBackgroundTexture = new Texture2D(mRenderer);
	if (!mBackgroundTexture->LoadFromFile("Images/BackgroundMB.png"))
	{
		std::cout << "Failed to load background texture!" << std::endl;
		return false;
	}
	
	//Set up the player character
	mMarioCharacter = new CharacterPlayable(mRenderer, "Images/Mario.png", Vector2D(64, 330), SDLK_w, SDLK_d, SDLK_a, 
		mLevelMap, MOVEMENTSPEED, &mEnemiesAndCoins, "Mario", MARIO_TEXT_POS, INITIAL_LIVES, PLAYER_FRAME_DELAY, MARIO_IDLE_FRAME_COUNT, MARIO_FRAME_COUNT, MARIO_IDLE_START_FRAME);
	mLuigiCharacter = new CharacterPlayable(mRenderer, "Images/Luigi.png", Vector2D(128, 330), SDLK_UP, SDLK_RIGHT, SDLK_LEFT, 
		mLevelMap, MOVEMENTSPEED, &mEnemiesAndCoins, "Luigi", LUIGI_TEXT_POS, INITIAL_LIVES, PLAYER_FRAME_DELAY, MARIO_IDLE_FRAME_COUNT, MARIO_FRAME_COUNT, MARIO_IDLE_START_FRAME);

	//Set up our POW block
	mPowBlock = new PowBlock(mRenderer, mLevelMap);
	mScreenshake = false;
	mBackgroundYPos = 0.0f;

	return true;
}

void GameScreenLevel1::SetLevelMap()
{
	//Clear up any old map
	if (mLevelMap != NULL)
		delete mLevelMap;

	std::vector<TILE> passableTiles;
	passableTiles.push_back(EMPTY);
	passableTiles.push_back(PIPE_ENTRANCE_LOWER_RIGHT);
	passableTiles.push_back(PIPE_ENTRANCE_UPPER_RIGHT);
	passableTiles.push_back(PIPE_ENTRANCE_LOWER_LEFT);
	passableTiles.push_back(PIPE_ENTRANCE_UPPER_LEFT);
	passableTiles.push_back(PIPE_BODY_LOWER);
	passableTiles.push_back(PIPE_BODY_UPPER);

	//Set the new one
	mLevelMap = new LevelMap(mRenderer, "Images/tileset.png", TILE_WIDTH, TILE_HEIGHT, true, passableTiles, "Levels/Level1.txt");
}

void GameScreenLevel1::SetUpEnemyWaves()
{
	EnemyWave wave1 = EnemyWave();
	EnemyWave wave2 = EnemyWave();
	EnemyWave wave3 = EnemyWave();

	std::vector<WaveComponent> wave;
	wave.push_back(WaveComponent(KOOPA, 0.0f, Vector2D(150, 32), FACING_RIGHT, KOOPA_SPEED));
	wave.push_back(WaveComponent(KOOPA, 0.0f, Vector2D(325, 32), FACING_LEFT, KOOPA_SPEED));
	wave.push_back(WaveComponent(COIN, 10.0f, Vector2D(150, 32), FACING_RIGHT, COIN_SPEED));
	wave1.SetWave(wave);
	mEnemyWaves.push_back(wave1);

	wave.clear();
	wave.push_back(WaveComponent(COIN, 5.0f, Vector2D(150, 32), FACING_RIGHT, COIN_SPEED));
	wave.push_back(WaveComponent(COIN, 6.0f, Vector2D(150, 32), FACING_RIGHT, COIN_SPEED));
	wave.push_back(WaveComponent(COIN, 5.0f, Vector2D(325, 32), FACING_LEFT, COIN_SPEED));
	wave.push_back(WaveComponent(COIN, 6.0f, Vector2D(325, 32), FACING_LEFT, COIN_SPEED));
	wave2.SetWave(wave);
	mEnemyWaves.push_back(wave2);

	wave.clear();
	wave.push_back(WaveComponent(KOOPA, 1.0f, Vector2D(150, 32), FACING_RIGHT, KOOPA_SPEED));
	wave.push_back(WaveComponent(KOOPA, 3.0f, Vector2D(325, 32), FACING_LEFT, KOOPA_SPEED));
	wave.push_back(WaveComponent(KOOPA, 5.0f, Vector2D(150, 32), FACING_RIGHT, KOOPA_SPEED));
	wave.push_back(WaveComponent(KOOPA, 7.0f, Vector2D(325, 32), FACING_LEFT, KOOPA_SPEED));
	wave3.SetWave(wave);
	mEnemyWaves.push_back(wave3);

	mCurrentWave = 0;
}

void GameScreenLevel1::UpdatePOWBlock()
{
	if (Collisions::Instance()->Box(mMarioCharacter->GetCollisionBox(), mPowBlock->GetCollisionBox()) && mPowBlock->IsAvailable() && mMarioCharacter->GetState() != PLAYER_DEATH)
	{
		//Collided whilst jumping
		if (mMarioCharacter->IsJumping())
		{
			DoScreenshake();
			mPowBlock->TakeAHit();
			mMarioCharacter->CancelJump();
		}
	}
	else if (Collisions::Instance()->Box(mLuigiCharacter->GetCollisionBox(), mPowBlock->GetCollisionBox()) && mPowBlock->IsAvailable() && mLuigiCharacter->GetState() != PLAYER_DEATH)
	{
		//Collided whilst jumping
		if (mLuigiCharacter->IsJumping())
		{
			DoScreenshake();
			mPowBlock->TakeAHit();
			mLuigiCharacter->CancelJump();
		}
	}
}

void GameScreenLevel1::DoScreenshake()
{
	mScreenshake = true;
	mScreenshakeTime = SCREENSHAKE_DURATION;
	mWobble = 0.0f;

	for (unsigned int i = 0; i < mEnemiesAndCoins.size(); i++)
	{
		mEnemiesAndCoins[i]->TakeDamage();
	}
}

void GameScreenLevel1::UpdateEnemiesAndCoins(float deltaTime, SDL_Event e)
{
	if (!mEnemiesAndCoins.empty())
	{
		int enemyIndexToDelete = -1;
		for (unsigned int i = 0; i < mEnemiesAndCoins.size(); i++)
		{
			//Check if enemy is on the bottom row of tiles
			if (mEnemiesAndCoins[i]->GetPosition().y > 300.0f)
			{
				if (mEnemiesAndCoins[i]->GetPosition().x < (float)(-mEnemiesAndCoins[i]->GetCollisionBox().width * 0.5f) ||
					mEnemiesAndCoins[i]->GetPosition().x > SCREEN_WIDTH - (float)(mEnemiesAndCoins[i]->GetCollisionBox().width * 0.55f))
				{
					if(mEnemiesAndCoins[i]->GetPosition().x > SCREEN_WIDTH - (float)(mEnemiesAndCoins[i]->GetCollisionBox().width * 0.55f))
						mEnemiesAndCoins[i]->SetPosition(Vector2D(0, 32));
					else
						mEnemiesAndCoins[i]->SetPosition(Vector2D(SCREEN_WIDTH- mEnemiesAndCoins[i]->GetCollisionBox().width * 0.5f, 32));
				}
			}

			//Now do the update
			mEnemiesAndCoins[i]->Update(deltaTime, e);

			//Check to see if the enemy collides with the player if they are not behind the pipes
			if (!((mEnemiesAndCoins[i]->GetPosition().y > 300.0f || mEnemiesAndCoins[i]->GetPosition().y <= 64.0f) &&
				(mEnemiesAndCoins[i]->GetPosition().x < 64.0f || mEnemiesAndCoins[i]->GetPosition().x > SCREEN_WIDTH - 96.0f)))
			{
				if (Collisions::Instance()->Circle(mEnemiesAndCoins[i], mMarioCharacter) && mMarioCharacter->GetState() != PLAYER_DEATH)
				{
					mEnemiesAndCoins[i]->OnPlayerCollision(mMarioCharacter);
				}
				else if (Collisions::Instance()->Circle(mEnemiesAndCoins[i], mLuigiCharacter) && mLuigiCharacter->GetState() != PLAYER_DEATH)
				{
					mEnemiesAndCoins[i]->OnPlayerCollision(mLuigiCharacter);
				}
			}

			//If the enemy is no longer alive, then schedule it for deletion
			if (!mEnemiesAndCoins[i]->GetAlive())
				enemyIndexToDelete = i;
		}

		if (enemyIndexToDelete != -1)
		{
			delete mEnemiesAndCoins[enemyIndexToDelete];
			mEnemiesAndCoins.erase(mEnemiesAndCoins.begin() + enemyIndexToDelete);
		}
	}
}

void GameScreenLevel1::CreateKoopa(Vector2D position, FACING direction, float speed)
{
	CharacterKoopa* koopaCharacter = new CharacterKoopa(mRenderer, "Images/Koopa.png", position, mLevelMap, direction, speed,
		ANIMATION_DELAY, KOOPA_FRAME_COUNT, KOOPA_MOVING_FRAME, KOOPA_MOVING_FRAME_COUNT);
	mEnemiesAndCoins.push_back(koopaCharacter);
}

void GameScreenLevel1::CreateCoin(Vector2D position, FACING direction, float speed)
{
	CharacterCoin* coinCharacter = new CharacterCoin(mRenderer, "Images/Coin.png", position, mLevelMap, direction, speed, ANIMATION_DELAY, COIN_FRAME_COUNT);
	mEnemiesAndCoins.push_back(coinCharacter);
}